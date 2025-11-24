// parser.cpp
// Compiladores - Análise Sintática com AST e checagem básica de declaração/uso
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "../lexer/lexer.cpp"

using namespace std;

// Tipos de nós da AST
enum class NodeKind {
    Program,
    Block,
    Decl,
    Assign,
    If,
    Binary,
    Literal,
    Identifier
};

// Nó da AST
struct ASTNode {
    NodeKind kind;
    Token token;                       // token principal (op, id ou literal)
    vector<shared_ptr<ASTNode>> children;
    string value;                      // lexema ou valor útil
};

// Erros semânticos simples coletados pelo parser (declaração/uso)
struct ParserSemanticError {
    string message;
    int linha;
    int coluna;
};

// Contexto semântico simples do parser
class ParserSemanticContext {
public:
    unordered_map<string, string> symbols; // escopo global simples: nome -> tipo
    vector<ParserSemanticError> errors;

    // Declara uma variável
    void declare(const string& name, const string& type, int linha, int coluna) {
        symbols[name] = type;
    }

    // Verifica se uma variável foi declarada
    bool isDeclared(const string& name) const {
        return symbols.find(name) != symbols.end();
    }

    // Registra um erro semântico
    void report(const string& msg, int linha, int coluna) {
        errors.push_back({msg, linha, coluna});
    }
};

// Analisador Sintático
class Parser {
public:
    Parser(const vector<Token>& tokens) : tokens(tokens), current(0) {}

    // Parseia e retorna a raiz da AST (program)
    shared_ptr<ASTNode> parse() {
        return parseProgram();
    }

    // Imprime erros semânticos coletados
    void printSemanticErrors() const {
        for (auto &e : sem.errors) {
            cerr << "[Erro semantico] " << e.message
                 << " (" << e.linha << "," << e.coluna << ")\n";
        }
    }

private:
    const vector<Token>& tokens;
    size_t current;
    ParserSemanticContext sem;

    // utilidades
    // Verifica se chegou ao fim dos tokens
    bool isAtEnd() const { return peek().tipo == TokenType::END_OF_FILE; }

    // Retorna o token atual sem consumir
    const Token& peek() const { return tokens.at(current); }

    // Retorna o token anterior sem consumir
    const Token& previous() const { return tokens.at(current - 1); }

    // Avança para o próximo token e retorna o atual
    const Token& advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    // Verifica o tipo do token atual sem consumir
    bool checkType(TokenType t) const {
        if (isAtEnd()) return false;
        return peek().tipo == t;
    }

    // Verifica o tipo e texto do token atual sem consumir
    bool check(TokenType t, const string& text) const {
        if (isAtEnd()) return false;
        const Token& tk = peek();
        return tk.tipo == t && tk.texto == text;
    }

    // Se o token atual bate com tipo e texto, avança e retorna true
    bool match(TokenType t, const string& text) {
        if (check(t, text)) { advance(); return true; }
        return false;
    }

    // Espera que o token atual bata com tipo e texto, avança; senão, erro
    void expect(TokenType t, const string& text, const string& msg) {
        if (check(t, text)) { advance(); return; }
        error(peek(), msg);
    }

    // Espera que o token atual bata com tipo, avança; senão, erro
    void expectType(TokenType t, const string& msg) {
        if (checkType(t)) { advance(); return; }
        error(peek(), msg);
    }

    // Lança um erro sintático
    [[noreturn]] void error(const Token& token, const string& msg) {
        throw runtime_error(
            "Erro sintatico na linha " + to_string(token.linha) +
            ", coluna " + to_string(token.coluna) +
            ": " + msg + " (encontrei '" + token.texto + "')"
        );
    }

    // Cria um nó da AST
    shared_ptr<ASTNode> makeNode(NodeKind kind, const Token& tok, vector<shared_ptr<ASTNode>> children = {}, const string& value = "") {
        auto n = make_shared<ASTNode>();
        n->kind = kind;
        n->token = tok;
        n->children = move(children);
        n->value = value.empty() ? tok.texto : value;
        return n;
    }

    // Cria um nó binário da AST
    shared_ptr<ASTNode> makeBinary(const Token& op, shared_ptr<ASTNode> lhs, shared_ptr<ASTNode> rhs) {
        return makeNode(NodeKind::Binary, op, {lhs, rhs}, op.texto);
    }

    // ===== regras =====
    // program -> stmt*
    // Retorna o nó raiz da AST
    shared_ptr<ASTNode> parseProgram() {
        vector<shared_ptr<ASTNode>> stmts;
        while (!isAtEnd()) {
            if (checkType(TokenType::COMMENT)) { advance(); continue; }
            stmts.push_back(parseStatement());
        }
        Token fake = isAtEnd() ? previous() : peek();
        return makeNode(NodeKind::Program, fake, move(stmts), "program");
    }

    // stmt -> decl | ifStmt | assign | block
    // Declaração, if, atribuição ou bloco
    shared_ptr<ASTNode> parseStatement() {
        while (checkType(TokenType::COMMENT)) advance();    // pular comentários

        if (checkType(TokenType::KEYWORD) && isTypeKeyword(peek().texto)) {
            return parseDecl();
        }
        if (check(TokenType::KEYWORD, "if")) {
            return parseIf();
        }
        if (check(TokenType::PUNCTUATION, "{")) {
            return parseBlock();
        }
        if (checkType(TokenType::IDENTIFIER)) {
            return parseAssign();
        }

        error(peek(), "declaracao, if, bloco ou atribuicao esperado");
    }

    // decl -> (int|float|string|bool) IDENTIFIER ( "=" expr )? ";"
    // Declaração de variável com inicialização opcional
    shared_ptr<ASTNode> parseDecl() {
        Token typeTok = advance();              // tipo
        string typeText = typeTok.texto;

        Token idTok = peek();
        expectType(TokenType::IDENTIFIER, "identificador esperado apos '" + typeText + "'");

        // registrar tipo na tabela do parser
        sem.declare(idTok.texto, typeText, idTok.linha, idTok.coluna);

        // filhos do nó de declaração
        vector<shared_ptr<ASTNode>> kids;
        kids.push_back(makeNode(NodeKind::Identifier, idTok));

        // inicialização opcional
        if (match(TokenType::OPERATOR, "=")) {
            auto initExpr = parseExpr();
            kids.push_back(initExpr);
        }

        expect(TokenType::PUNCTUATION, ";", "';' esperado ao final da declaracao");
        // valor do nó = nome da variável; token do nó = token do tipo (para semântica saber o tipo)
        auto declNode = makeNode(NodeKind::Decl, typeTok, move(kids), idTok.texto);
        return declNode;
    }

    // block -> "{" stmt* "}"
    // Bloco de código
    shared_ptr<ASTNode> parseBlock() {  
        Token lbrace = peek();                                          // salvar token '{' para o nó
        expect(TokenType::PUNCTUATION, "{", "esperado '{' para iniciar bloco");
        vector<shared_ptr<ASTNode>> stmts;                              // filhos do nó bloco
        while (!check(TokenType::PUNCTUATION, "}") && !isAtEnd()) {     // enquanto não achar '}' ou EOF
            stmts.push_back(parseStatement());                          // parsear statements dentro do bloco
        }
        expect(TokenType::PUNCTUATION, "}", "esperado '}' ao final do bloco");
        return makeNode(NodeKind::Block, lbrace, move(stmts), "block"); // criar nó bloco
    }

    // ifStmt -> "if" "(" expr ")" stmt ("else" stmt)?
    // Instrução condicional if-else
    shared_ptr<ASTNode> parseIf() {                                     // inicia um if
        Token ifTok = peek();
        expect(TokenType::KEYWORD, "if", "esperado 'if'");              // condição do if     
        expect(TokenType::PUNCTUATION, "(", "esperado '(' apos if");    // abrir parêntese
        auto cond = parseExpr();                                        // expressão condicional
        expect(TokenType::PUNCTUATION, ")", "esperado ')' apos condicao do if");    // fechar parêntese
        auto thenBranch = parseStatement();                             // ramo "then"
        shared_ptr<ASTNode> elseBranch = nullptr;                       // ramo "else" opcional
        if (match(TokenType::KEYWORD, "else")) {                        // se houver "else"
            elseBranch = parseStatement();                              // parsear ramo "else"
        }
        vector<shared_ptr<ASTNode>> kids = {cond, thenBranch};          // filhos do nó if
        if (elseBranch) kids.push_back(elseBranch);                     // adicionar ramo else se existir
        return makeNode(NodeKind::If, ifTok, move(kids), "if");         // criar nó if
    }

    // assign -> IDENTIFIER "=" expr ";"
    // Atribuição de valor a variável
    shared_ptr<ASTNode> parseAssign() {                                 // inicia uma atribuição
        Token idTok = advance();       
        if (!sem.isDeclared(idTok.texto)) {                             // verificar se foi declarado
            sem.report("variavel '" + idTok.texto + "' usada sem declarar", idTok.linha, idTok.coluna);
        }
        expect(TokenType::OPERATOR, "=", "esperado '=' na atribuicao");
        auto expr = parseExpr();                                        // expressão do lado direito
        expect(TokenType::PUNCTUATION, ";", "esperado ';' ao final da atribuicao");
        auto idNode = makeNode(NodeKind::Identifier, idTok);            // nó do identificador
        return makeNode(NodeKind::Assign, idTok, {idNode, expr}, "=");  // nó de atribuição
    }

    // ===== EXPRESSÕES =====
    
    shared_ptr<ASTNode> parseExpr() { return parseOr(); }   // expressão lógica OR

    // orExpr -> andExpr ( "||" andExpr )*
    shared_ptr<ASTNode> parseOr() { 
        auto left = parseAnd();                         // expressão lógica AND
        while (match(TokenType::OPERATOR, "||")) {      // enquanto achar "||"
            Token op = previous();                      // operador ||
            auto right = parseAnd();                    // próxima expressão AND
            left = makeBinary(op, left, right);         // criar nó binário
        }
        return left;                                    // retornar expressão resultante
    }

    // andExpr -> equality ( "&&" equality )*
    // expressão lógica AND
    shared_ptr<ASTNode> parseAnd() {
        auto left = parseEquality();                    // expressão de igualdade
        while (match(TokenType::OPERATOR, "&&")) {      // enquanto achar "&&"
            Token op = previous();                      // operador &&
            auto right = parseEquality();               // próxima expressão de igualdade
            left = makeBinary(op, left, right);         // criar nó binário
        }
        return left;                                    // retornar expressão resultante
    }

    // equality -> rel (("=="|"!=") rel)*
    // expressão de igualdade
    shared_ptr<ASTNode> parseEquality() {
        auto left = parseRel();                         // expressão relacional
        while (check(TokenType::OPERATOR, "==") || check(TokenType::OPERATOR, "!=")) {  // enquanto achar "==" ou "!="
            Token op = advance();
            auto right = parseRel();                    // próxima expressão relacional
            left = makeBinary(op, left, right);         // criar nó binário
        }
        return left;
    }

    // rel -> add (("<"|">"|"<="|">=") add)*
    // expressão relacional
    shared_ptr<ASTNode> parseRel() {                    // expressão relacional
        auto left = parseAdd();                         // expressão de adição
        while (check(TokenType::OPERATOR, "<") ||
               check(TokenType::OPERATOR, ">") ||
               check(TokenType::OPERATOR, "<=") ||
               check(TokenType::OPERATOR, ">=")) {
            Token op = advance();
            auto right = parseAdd();                    // próxima expressão de adição
            left = makeBinary(op, left, right);         // criar nó binário
        }
        return left;
    }

    // add -> mult (("+"|"-") mult)*
    // expressão de adição
    shared_ptr<ASTNode> parseAdd() {                    // expressão de adição
        auto left = parseMult();                        // expressão de multiplicação
        while (check(TokenType::OPERATOR, "+") || check(TokenType::OPERATOR, "-")) {    // enquanto achar "+" ou "-"
            Token op = advance();                       // operador + ou -    
            auto right = parseMult();                   // próxima expressão de multiplicação
            left = makeBinary(op, left, right);         // criar nó binário
        }
        return left;
    }

    // mult -> primary (("*"|"/"|"%") primary)*
    // expressão de multiplicação
    shared_ptr<ASTNode> parseMult() {                   // expressão de multiplicação
        auto left = parsePrimary();                     // expressão primária    
        while (check(TokenType::OPERATOR, "*") ||
               check(TokenType::OPERATOR, "/") ||
               check(TokenType::OPERATOR, "%")) {
            Token op = advance();                       // operador * / %
            auto right = parsePrimary();                // próxima expressão primária
            left = makeBinary(op, left, right);         // criar nó binário
        }
        return left;
    }

    // primary -> IDENTIFIER | NUM_INT | NUM_REAL | STRING | BOOL | "(" expr ")"
    // expressão primária: identificador, literal ou parêntese
    shared_ptr<ASTNode> parsePrimary() {                // expressão primária
        if (checkType(TokenType::IDENTIFIER)) {         // identificador
            Token id = advance();                       // consumir identificador
            if (!sem.isDeclared(id.texto)) {            // verificar se foi declarado
                sem.report("variavel '" + id.texto + "' usada sem declarar", id.linha, id.coluna);
            }
            return makeNode(NodeKind::Identifier, id);  // criar nó identificador
        }

        // se for literal
        if (checkType(TokenType::NUM_INT) || checkType(TokenType::NUM_REAL) || checkType(TokenType::STRING)) {
            Token lit = advance();
            return makeNode(NodeKind::Literal, lit);
        }

        // se for booleano
        if (check(TokenType::KEYWORD, "true") || check(TokenType::KEYWORD, "false")) {
            Token lit = advance();
            return makeNode(NodeKind::Literal, lit);
        }

        // se for parêntese
        if (match(TokenType::PUNCTUATION, "(")) {
            auto expr = parseExpr();
            expect(TokenType::PUNCTUATION, ")", "esperado ')' apos expressao");
            return expr;
        }

        // se nada bater, erro
        error(peek(), "expressao, identificador ou literal esperado");
    }

    // Verifica se uma palavra-chave é um tipo válido
    bool isTypeKeyword(const string& kw) const {
        return kw == "int" || kw == "float" || kw == "string" || kw == "bool" || kw == "boolean";
    }
};
