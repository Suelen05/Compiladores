// parser.cpp
// Compiladores - Análise Sintática + AST + início de Semântica

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "../lexer/lexer.cpp"

using namespace std;

// ===== AST =====
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

struct ASTNode {
    NodeKind kind;
    Token token; // guarda o token principal (op, id ou literal)
    vector<shared_ptr<ASTNode>> children;
    string value; // lexema ou valor útil
};

// ===== Semântica básica =====
struct SemanticError {
    string message;
    int linha;
    int coluna;
};

class SemanticContext {
public:
    unordered_map<string, string> symbols; // escopo global simples
    vector<SemanticError> errors;

    void declare(const string& name, const string& type, int linha, int coluna) {
        symbols[name] = type;
    }

    bool isDeclared(const string& name) const {
        return symbols.find(name) != symbols.end();
    }

    void report(const string& msg, int linha, int coluna) {
        errors.push_back({msg, linha, coluna});
    }
};

class Parser {
public:
    Parser(const vector<Token>& tokens)
        : tokens(tokens), current(0) {}

    // Parseia e retorna a raiz da AST (program)
    shared_ptr<ASTNode> parse() {
        auto program = parseProgram();
        return program;
    }

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
    bool isAtEnd() const {
        return peek().tipo == TokenType::END_OF_FILE;
    }

    const Token& peek() const {
        return tokens.at(current);
    }

    const Token& previous() const {
        return tokens.at(current - 1);
    }

    const Token& advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool checkType(TokenType t) const {
        if (isAtEnd()) return false;
        return peek().tipo == t;
    }

    bool check(TokenType t, const string& text) const {
        if (isAtEnd()) return false;
        const Token& tk = peek();
        return tk.tipo == t && tk.texto == text;
    }

    bool match(TokenType t, const string& text) {
        if (check(t, text)) {
            advance();
            return true;
        }
        return false;
    }

    void expect(TokenType t, const string& text, const string& msg) {
        if (check(t, text)) {
            advance();
            return;
        }
        error(peek(), msg);
    }

    void expectType(TokenType t, const string& msg) {
        if (checkType(t)) {
            advance();
            return;
        }
        error(peek(), msg);
    }

    [[noreturn]] void error(const Token& token, const string& msg) {
        throw runtime_error(
            "Erro sintático na linha " + to_string(token.linha) +
            ", coluna " + to_string(token.coluna) +
            ": " + msg + " (encontrei '" + token.texto + "')"
        );
    }

    shared_ptr<ASTNode> makeNode(NodeKind kind, const Token& tok, vector<shared_ptr<ASTNode>> children = {}, const string& value = "") {
        auto n = make_shared<ASTNode>();
        n->kind = kind;
        n->token = tok;
        n->children = move(children);
        n->value = value.empty() ? tok.texto : value;
        return n;
    }

    shared_ptr<ASTNode> makeBinary(const Token& op, shared_ptr<ASTNode> lhs, shared_ptr<ASTNode> rhs) {
        return makeNode(NodeKind::Binary, op, {lhs, rhs}, op.texto);
    }

    // ===== regras =====
    // program -> stmt*
   shared_ptr<ASTNode> parseProgram() {
        vector<shared_ptr<ASTNode>> stmts;
        while (!isAtEnd()) {
            // pular comentários
            if (checkType(TokenType::COMMENT)) {
                advance();
                continue;
            }
            stmts.push_back(parseStatement());
        }
        Token fake = isAtEnd() ? previous() : peek();
        return makeNode(NodeKind::Program, fake, move(stmts), "program");
    }

    // stmt -> decl | ifStmt | assign | block
    shared_ptr<ASTNode> parseStatement() {
        while (checkType(TokenType::COMMENT)) advance();

        if (check(TokenType::KEYWORD, "int")) {
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

        error(peek(), "declaração, if, bloco ou atribuição esperado");
    }

    // decl -> "int" IDENTIFIER ";"   (init opcional pode ser adicionado depois)
    shared_ptr<ASTNode> parseDecl() {
        Token kw = advance(); // 'int'
        Token idTok = peek();
        expectType(TokenType::IDENTIFIER, "identificador esperado após 'int'");

        sem.declare(idTok.texto, "int", idTok.linha, idTok.coluna);

        expect(TokenType::PUNCTUATION, ";", "';' esperado ao final da declaração");
        return makeNode(NodeKind::Decl, kw, { makeNode(NodeKind::Identifier, idTok) }, idTok.texto);
    }

    // block -> "{" stmt* "}"
    shared_ptr<ASTNode> parseBlock() {
        Token lbrace = peek();
        expect(TokenType::PUNCTUATION, "{", "esperado '{' para iniciar bloco");
        vector<shared_ptr<ASTNode>> stmts;
        while (!check(TokenType::PUNCTUATION, "}") && !isAtEnd()) {
            stmts.push_back(parseStatement());
        }
        expect(TokenType::PUNCTUATION, "}", "esperado '}' ao final do bloco");
        return makeNode(NodeKind::Block, lbrace, move(stmts), "block");
    }

    // ifStmt -> "if" "(" expr ")" stmt ("else" stmt)?
    shared_ptr<ASTNode> parseIf() {
        Token ifTok = peek();
        expect(TokenType::KEYWORD, "if", "esperado 'if'");
        expect(TokenType::PUNCTUATION, "(", "esperado '(' após if");
        auto cond = parseExpr();
        expect(TokenType::PUNCTUATION, ")", "esperado ')' após condição do if");
        auto thenBranch = parseStatement();
        shared_ptr<ASTNode> elseBranch = nullptr;
        if (match(TokenType::KEYWORD, "else")) {
            elseBranch = parseStatement();
        }
        vector<shared_ptr<ASTNode>> kids = {cond, thenBranch};
        if (elseBranch) kids.push_back(elseBranch);
        return makeNode(NodeKind::If, ifTok, move(kids), "if");
    }

    // assign -> IDENTIFIER "=" expr ";"
    shared_ptr<ASTNode> parseAssign() {
        Token idTok = advance(); // IDENTIFIER
        if (!sem.isDeclared(idTok.texto)) {
            sem.report("variável '" + idTok.texto + "' usada sem declarar", idTok.linha, idTok.coluna);
        }
        expect(TokenType::OPERATOR, "=", "esperado '=' na atribuição");
        auto expr = parseExpr();
        expect(TokenType::PUNCTUATION, ";", "esperado ';' ao final da atribuição");
        auto idNode = makeNode(NodeKind::Identifier, idTok);
        return makeNode(NodeKind::Assign, idTok, {idNode, expr}, "=");
    }

    // ===== EXPRESSÕES =====
    shared_ptr<ASTNode> parseExpr() { return parseOr(); }

    // orExpr -> andExpr ( "||" andExpr )*
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
    shared_ptr<ASTNode> parsePrimary() {
        if (checkType(TokenType::IDENTIFIER)) {
            Token id = advance();
            if (!sem.isDeclared(id.texto)) {
                sem.report("variável '" + id.texto + "' usada sem declarar", id.linha, id.coluna);
            }
            return makeNode(NodeKind::Identifier, id);
        }

        if (checkType(TokenType::NUM_INT) || checkType(TokenType::NUM_REAL) || checkType(TokenType::STRING)) {
            Token lit = advance();
            return makeNode(NodeKind::Literal, lit);
        }

        if (match(TokenType::PUNCTUATION, "(")) {
            auto expr = parseExpr();
            expect(TokenType::PUNCTUATION, ")", "esperado ')' após expressão");
            return expr;
       }

        error(peek(), "expressão, identificador ou literal esperado");
    }
};
