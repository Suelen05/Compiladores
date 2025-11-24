// parser.cpp
// Compiladores - Análise Sintática + AST + início de Semântica

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "../lexer/lexer.cpp"

using namespace std;

// Definição dos tipos de nós da AST
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

// Estrutura dos nós da AST 
struct ASTNode {
    NodeKind kind;
    Token token;            // guarda o token principal (op, id ou literal)
    vector<shared_ptr<ASTNode>> children;
    string value;           // lexema ou valor útil
};

// ===== Semântica básica =====
struct SemanticError {
    string message;
    int linha;
    int coluna;
};

class SemanticContext {
public:
    unordered_map<string, string> symbols;      // escopo global simples
    vector<SemanticError> errors;

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
    Parser(const vector<Token>& tokens): tokens(tokens), current(0) {}

    // Parseia e retorna a raiz da AST (program)
    shared_ptr<ASTNode> parse() {
        auto program = parseProgram();
        return program;
    }

    // Imprime os erros semânticos encontrados
    void printSemanticErrors() const {
        for (auto &e : sem.errors) {
            cerr << "[Erro semântico] " << e.message
                 << " (" << e.linha << "," << e.coluna << ")\n";
        }
    }

private:
    const vector<Token>& tokens;
    size_t current;
    SemanticContext sem;

    // ===== utilidades =====
    // Verifica se chegou ao fim dos tokens
    bool isAtEnd() const {
        return peek().tipo == TokenType::END_OF_FILE;
    }

    // Retorna o token atual sem consumir
    const Token& peek() const {
        return tokens.at(current);
    }

    // Retorna o token anterior sem consumir
    const Token& previous() const {
        return tokens.at(current - 1);
    }

    // Avança para o próximo token e retorna o atual
    const Token& advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    // Verifica o tipo do token atual
    bool checkType(TokenType t) const {
        if (isAtEnd()) return false;
        return peek().tipo == t;
    }

    // Verifica o tipo e texto do token atual
    bool check(TokenType t, const string& text) const {
        if (isAtEnd()) return false;
        const Token& tk = peek();
        return tk.tipo == t && tk.texto == text;
    }

    // Consome o token atual se corresponder ao tipo e texto
    bool match(TokenType t, const string& text) {
        if (check(t, text)) {
            advance();
            return true;
        }
        return false;
    }

    // Espera que o token atual corresponda ao tipo e texto, senão lança erro
    void expect(TokenType t, const string& text, const string& msg) {
        if (check(t, text)) {
            advance();
            return;
        }
        error(peek(), msg);
    }

    // Espera que o token atual corresponda ao tipo, senão lança erro
    void expectType(TokenType t, const string& msg) {
        if (checkType(t)) {
            advance();
            return;
        }
        error(peek(), msg);
    }

    // Lança um erro sintático
    [[noreturn]] void error(const Token& token, const string& msg) {
        throw runtime_error(
            "Erro sintático na linha " + to_string(token.linha) +
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

    // Cria um nó binário
    shared_ptr<ASTNode> makeBinary(const Token& op, shared_ptr<ASTNode> lhs, shared_ptr<ASTNode> rhs) {
        return makeNode(NodeKind::Binary, op, {lhs, rhs}, op.texto);
    }

    // ===== regras =====
    // program -> stmt*
    // Retorna o nó raiz Program
   shared_ptr<ASTNode> parseProgram() {
        vector<shared_ptr<ASTNode>> stmts;
        while (!isAtEnd()) {
            
            if (checkType(TokenType::COMMENT)) {    // pular comentários
                advance();
                continue;
            }
            stmts.push_back(parseStatement());  //coloca cada stmt como filho do programa
        }
        Token fake = isAtEnd() ? previous() : peek();   
        return makeNode(NodeKind::Program, fake, move(stmts), "program");
    }

    // stmt -> decl | ifStmt | assign | block
    // Retorna um nó de declaração, if, atribuição ou bloco
    shared_ptr<ASTNode> parseStatement() {
        while (checkType(TokenType::COMMENT)) advance();    // pular comentários

        if (check(TokenType::KEYWORD, "int")) {             // declaração
            return parseDecl();
        }

        if (check(TokenType::KEYWORD, "if")) {              // if
            return parseIf();
        }

        if (check(TokenType::PUNCTUATION, "{")) {           // bloco
            return parseBlock();
        }

        if (checkType(TokenType::IDENTIFIER)) {             // atribuição
            return parseAssign();
        }

        error(peek(), "declaração, if, bloco ou atribuição esperado");      // erro se não reconhecer
    }

    // decl -> "int" IDENTIFIER ";"   (init opcional pode ser adicionado depois)
    // Retorna um nó de declaração
    shared_ptr<ASTNode> parseDecl() {       // declara uma variável int
        Token kw = advance();                // 'int'
        Token idTok = peek();
        expectType(TokenType::IDENTIFIER, "identificador esperado após 'int'");

        sem.declare(idTok.texto, "int", idTok.linha, idTok.coluna);     // registra na tabela de símbolos

        expect(TokenType::PUNCTUATION, ";", "';' esperado ao final da declaração");
        return makeNode(NodeKind::Decl, kw, { makeNode(NodeKind::Identifier, idTok) }, idTok.texto);    // nó Decl com filho Identifier

    // block -> "{" stmt* "}"
    // Retorna um nó de bloco
    shared_ptr<ASTNode> parseBlock() {                              // inicia um bloco
        Token lbrace = peek();
        expect(TokenType::PUNCTUATION, "{", "esperado '{' para iniciar bloco"); 
        vector<shared_ptr<ASTNode>> stmts;                          // nó filho para cada stmt
        while (!check(TokenType::PUNCTUATION, "}") && !isAtEnd()) { // enquanto não encontrar '}' ou EOF
            stmts.push_back(parseStatement());
        }
        expect(TokenType::PUNCTUATION, "}", "esperado '}' ao final do bloco");  
        return makeNode(NodeKind::Block, lbrace, move(stmts), "block"); // nó Block com filhos stmts
    }

    // ifStmt -> "if" "(" expr ")" stmt ("else" stmt)?
    // Retorna um nó if
    shared_ptr<ASTNode> parseIf() {                                     // inicia um if
        Token ifTok = peek();
        expect(TokenType::KEYWORD, "if", "esperado 'if'");
        expect(TokenType::PUNCTUATION, "(", "esperado '(' após if");    // condição entre parênteses
        auto cond = parseExpr();
        expect(TokenType::PUNCTUATION, ")", "esperado ')' após condição do if");// fecha parênteses
        auto thenBranch = parseStatement();
        shared_ptr<ASTNode> elseBranch = nullptr;                       // inicializa else como nulo
        if (match(TokenType::KEYWORD, "else")) {                        // verifica se há else
            elseBranch = parseStatement();
        }
        vector<shared_ptr<ASTNode>> kids = {cond, thenBranch};          // filhos: condição e ramo then
        if (elseBranch) kids.push_back(elseBranch);                     // adiciona ramo else se existir
        return makeNode(NodeKind::If, ifTok, move(kids), "if");
    }

    // assign -> IDENTIFIER "=" expr ";"
    // Retorna um nó de atribuição
    shared_ptr<ASTNode> parseAssign() {
        Token idTok = advance();                // IDENTIFIER
        if (!sem.isDeclared(idTok.texto)) {     // verifica se foi declarado
            sem.report("variável '" + idTok.texto + "' usada sem declarar", idTok.linha, idTok.coluna);
        }
        expect(TokenType::OPERATOR, "=", "esperado '=' na atribuição"); 
        auto expr = parseExpr();                // expressão do lado direito
        expect(TokenType::PUNCTUATION, ";", "esperado ';' ao final da atribuição");
        auto idNode = makeNode(NodeKind::Identifier, idTok);    // nó filho Identifier
        return makeNode(NodeKind::Assign, idTok, {idNode, expr}, "=");// nó Assign com filhos Identifier e expressão
    }

    // ===== EXPRESSÕES =====
    // expr -> orExpr
    // Retorna um nó de expressão
    shared_ptr<ASTNode> parseExpr() { return parseOr(); }   

    // orExpr -> andExpr ( "||" andExpr )*
    // Retorna um nó de expressão lógica OR
    shared_ptr<ASTNode> parseOr() {
        auto left = parseAnd();
        while (match(TokenType::OPERATOR, "||")) {
            Token op = previous();
            auto right = parseAnd();
            left = makeBinary(op, left, right);
        }
        return left;
    }

    // andExpr -> equality ( "&&" equality )*
    // Retorna um nó de expressão lógica AND
    shared_ptr<ASTNode> parseAnd() { 
        auto left = parseEquality();
        while (match(TokenType::OPERATOR, "&&")) {
            Token op = previous();
            auto right = parseEquality();
            left = makeBinary(op, left, right);
        }
        return left;
    }

    // equality -> rel (("=="|"!=") rel)*
    // Retorna um nó de expressão de igualdade
    shared_ptr<ASTNode> parseEquality() {   
        auto left = parseRel();
        while (check(TokenType::OPERATOR, "==") || check(TokenType::OPERATOR, "!=")) {
            Token op = advance();
            auto right = parseRel();
            left = makeBinary(op, left, right);
        }
        return left;
    }

    // rel -> add (("<"|">"|"<="|">=") add)*
    // Retorna um nó de expressão relacional
    shared_ptr<ASTNode> parseRel() {
        auto left = parseAdd();
        while (check(TokenType::OPERATOR, "<") ||
               check(TokenType::OPERATOR, ">") ||
               check(TokenType::OPERATOR, "<=") ||
               check(TokenType::OPERATOR, ">=")) {
            Token op = advance();
            auto right = parseAdd();
            left = makeBinary(op, left, right);
        }
        return left;
    }

    // add -> mult (("+"|"-") mult)*
    // Retorna um nó de expressão de adição/subtração
    shared_ptr<ASTNode> parseAdd() {
        auto left = parseMult();
        while (check(TokenType::OPERATOR, "+") || check(TokenType::OPERATOR, "-")) {
            Token op = advance();
            auto right = parseMult();
            left = makeBinary(op, left, right);
        }
        return left;
    }

    // mult -> primary (("*"|"/"|"%") primary)*
    // Retorna um nó de expressão de multiplicação/divisão/módulo
    shared_ptr<ASTNode> parseMult() {
        auto left = parsePrimary();
        while (check(TokenType::OPERATOR, "*") ||
               check(TokenType::OPERATOR, "/") ||
               check(TokenType::OPERATOR, "%")) {
            Token op = advance();
            auto right = parsePrimary();
            left = makeBinary(op, left, right);
        }
        return left;
    }

    // primary -> IDENTIFIER | NUM_INT | NUM_REAL | STRING | "(" expr ")"
    // Retorna um nó primário (identificador, literal ou expressão entre parênteses)
    shared_ptr<ASTNode> parsePrimary() {
        if (checkType(TokenType::IDENTIFIER)) {     // identificador
            Token id = advance();
            if (!sem.isDeclared(id.texto)) {        // verifica se foi declarado
                sem.report("variável '" + id.texto + "' usada sem declarar", id.linha, id.coluna);
            }
            return makeNode(NodeKind::Identifier, id);
        }

        if (checkType(TokenType::NUM_INT) || checkType(TokenType::NUM_REAL) || checkType(TokenType::STRING)) {
            Token lit = advance();                      // literal
            return makeNode(NodeKind::Literal, lit); 
        }

        if (match(TokenType::PUNCTUATION, "(")) {   // expressão entre parênteses
            auto expr = parseExpr();
            expect(TokenType::PUNCTUATION, ")", "esperado ')' após expressão"); // fecha parênteses
            return expr;
       }

        error(peek(), "expressão, identificador ou literal esperado");  // erro se não reconhecer
    }
};
