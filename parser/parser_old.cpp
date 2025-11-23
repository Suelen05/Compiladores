// parser.cpp v1.2
// Compiladores - Análise Sintática + começo da Semântica
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "../lexer/lexer.cpp"   

using namespace std;

// Estrutura para guardar erros semânticos
struct SemanticError {
    string message;
    int linha;
    int coluna;
};

class SemanticContext {
public:
    // escopo simples (global)
    unordered_map<string, string> symbols; 
    vector<SemanticError> errors;

    // lista de variaveis declaradas 
    void declare(const string& name, const string& type, int linha, int coluna) {
        // se quiser, dá pra checar redeclaração aqui
        symbols[name] = type;
    }

    // Checa se a variavel ja esta declarada
    bool isDeclared(const string& name) const {
        return symbols.find(name) != symbols.end();
    }

    //retorna a mensagem de erro
    void report(const string& msg, int linha, int coluna) {
        errors.push_back({msg, linha, coluna});
    }
};

class Parser {
public:

    // Recebe o vetor de tokens gerados pelo lexer.cpp
    Parser(const vector<Token>& tokens)
        : tokens(tokens), current(0) {}

    void parseProgram() {
        // program -> stmt*
        while (!isAtEnd()) {
            // pular comentários
            if (checkType(TokenType::COMMENT)) {
                advance();
                continue;
            }
            parseStatement();
        }
    }

    // depois de parsear tu pode chamar isso
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

    // ===== utilidades básicas =====
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

    // checa tipo + texto
    bool check(TokenType t, const string& text) const {
        if (isAtEnd()) return false;
        const Token& tk = peek();
        return tk.tipo == t && tk.texto == text;
    }

    // consome se for do tipo/texto
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
        // erro sintático mais legível
        throw runtime_error(
            "Erro sintático na linha " + to_string(token.linha) +
            ", coluna " + to_string(token.coluna) +
            ": " + msg + " (encontrei '" + token.texto + "')"
        );
    }

    // ===== regras =====

    // stmt -> decl | ifStmt | assign | block
    void parseStatement() {
        // pular comentários
        while (checkType(TokenType::COMMENT)) advance();

        // declaração: KEYWORD "int" IDENTIFIER ";"
        if (check(TokenType::KEYWORD, "int")) {
            parseDecl();
            return;
        }

        // if (...)
        if (check(TokenType::KEYWORD, "if")) {
            parseIf();
            return;
        }

        // bloco
        if (check(TokenType::PUNCTUATION, "{")) {
            parseBlock();
            return;
        }

        // atribuição começa com identificador
        if (checkType(TokenType::IDENTIFIER)) {
            parseAssign();
            return;
        }

        // se chegou aqui, não reconheceu
        error(peek(), "declaração, if, bloco ou atribuição esperado");
    }

    // decl -> "int" IDENTIFIER ";"
    void parseDecl() {
        Token kw = advance(); // 'int'
        (void)kw; // só pra não dar warning

        Token id = peek();
        expectType(TokenType::IDENTIFIER, "identificador esperado após 'int'");

        // registrar na semântica
        sem.declare(id.texto, "int", id.linha, id.coluna);

        expect(TokenType::PUNCTUATION, ";", "';' esperado ao final da declaração");
    }

    // block -> "{" stmt* "}"
    void parseBlock() {
        expect(TokenType::PUNCTUATION, "{", "esperado '{' para iniciar bloco");
        while (!check(TokenType::PUNCTUATION, "}") && !isAtEnd()) {
            parseStatement();
        }
        expect(TokenType::PUNCTUATION, "}", "esperado '}' ao final do bloco");
    }

    // ifStmt -> "if" "(" expr ")" stmt ("else" stmt)?
    void parseIf() {
        expect(TokenType::KEYWORD, "if", "esperado 'if'");
        expect(TokenType::PUNCTUATION, "(", "esperado '(' após if");
        parseExpr();
        expect(TokenType::PUNCTUATION, ")", "esperado ')' após condição do if");
        parseStatement();
        if (match(TokenType::KEYWORD, "else")) {
            parseStatement();
        }
    }

    // assign -> IDENTIFIER "=" expr ";"
    void parseAssign() {
        Token id = advance(); // IDENTIFIER
        // checagem semântica: variável declarada?
        if (!sem.isDeclared(id.texto)) {
            sem.report("variável '" + id.texto + "' usada sem declarar", id.linha, id.coluna);
        }

        expect(TokenType::OPERATOR, "=", "esperado '=' na atribuição");
        parseExpr();
        expect(TokenType::PUNCTUATION, ";", "esperado ';' ao final da atribuição");
    }

    // ===== EXPRESSÕES =====
    // expr -> orExpr
    void parseExpr() {
        parseOr();
    }

    // orExpr -> andExpr ( "||" andExpr )*
    void parseOr() {
        parseAnd();
        while (match(TokenType::OPERATOR, "||")) {
            parseAnd();
        }
    }

    // andExpr -> equality ( "&&" equality )*
    void parseAnd() {
        parseEquality();
        while (match(TokenType::OPERATOR, "&&")) {
            parseEquality();
        }
    }

    // equality -> rel (("=="|"!=") rel)*
    void parseEquality() {
        parseRel();
        while (check(TokenType::OPERATOR, "==") || check(TokenType::OPERATOR, "!=")) {
            advance();
            parseRel();
        }
    }

    // rel -> add (("<"|">"|"<="|">=") add)*
    void parseRel() {
        parseAdd();
        while (check(TokenType::OPERATOR, "<") ||
               check(TokenType::OPERATOR, ">") ||
               check(TokenType::OPERATOR, "<=") ||
               check(TokenType::OPERATOR, ">=")) {
            advance();
            parseAdd();
        }
    }

    // add -> mult (("+"|"-") mult)*
    void parseAdd() {
        parseMult();
        while (check(TokenType::OPERATOR, "+") || check(TokenType::OPERATOR, "-")) {
            advance();
            parseMult();
        }
    }

    // mult -> primary (("*"|"/"|"%") primary)*
    void parseMult() {
        parsePrimary();
        while (check(TokenType::OPERATOR, "*") ||
               check(TokenType::OPERATOR, "/") ||
               check(TokenType::OPERATOR, "%")) {
            advance();
            parsePrimary();
        }
    }

    // primary -> IDENTIFIER | NUM_INT | NUM_REAL | STRING | "(" expr ")"
    void parsePrimary() {
        // identificador
        if (checkType(TokenType::IDENTIFIER)) {
            Token id = advance();

            // se quiser fazer semântica em expressão: checar se existe
            if (!sem.isDeclared(id.texto)) {
                sem.report("variável '" + id.texto + "' usada sem declarar", id.linha, id.coluna);
            }
            return;
        }

        // literais numéricos e string
+        if (checkType(TokenType::NUM_INT) || checkType(TokenType::NUM_REAL) || checkType(TokenType::STRING)) {
            advance();
            return;
        }

        // ( expr )
        if (match(TokenType::PUNCTUATION, "(")) {
            parseExpr();
            expect(TokenType::PUNCTUATION, ")", "esperado ')' após expressão");
            return;
        }

        error(peek(), "expressão, identificador ou literal esperado");
    }
};
