// lexer.cpp
// Compiladores - Analisador Léxico
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype>
#include <stdexcept>

using namespace std;

// Definição dos tipos de tokens
enum class TokenType {
    IDENTIFIER,
    NUM_INT,
    NUM_REAL,
    STRING,
    KEYWORD,
    OPERATOR,
    PUNCTUATION,
    END_OF_FILE,
    UNKNOWN,
    COMMENT
};

// Estrutura dos tokens
struct Token {
    TokenType tipo;
    string texto;
    int linha;
    int coluna;

    Token(TokenType t, string l, int ln, int col)
        : tipo(t), texto(move(l)), linha(ln), coluna(col) {}

    string toString() const {
        auto tt = [](TokenType t) {
            switch (t) {
                case TokenType::IDENTIFIER:  return "IDENTIFICADOR";
                case TokenType::NUM_INT:     return "NUM_INT";
                case TokenType::NUM_REAL:    return "NUM_REAL";
                case TokenType::STRING:      return "STRING";
                case TokenType::KEYWORD:     return "KEYWORD";
                case TokenType::OPERATOR:    return "OPERADOR";
                case TokenType::PUNCTUATION: return "PONTUACAO";
                case TokenType::END_OF_FILE: return "FIM DE ARQUIVO";
                case TokenType::UNKNOWN:     return "UNKNOWN";
                case TokenType::COMMENT:     return "COMMENTARIO";
            }
            return "TOKEN";
        };
        return string(tt(tipo)) + " -> \"" + texto + "\" [" +
               to_string(linha) + "," + to_string(coluna) + "]";
    }
};

// Analisador Léxico
class Lexer {
    string src;
    size_t i = 0;
    int line = 1;
    int col = 1;

    unordered_set<string> keywords = {
        "if", "else", "while", "for", "switch", "case", "return",
        "int", "float", "string", "boolean", "void", "break",
        "continue", "true", "false", "null", "do", "enum", "struct",
        "typedef", "const", "static", "public", "private", "protected",
        "class", "new", "this", "super", "import", "package", "include"
    };

    char peek(size_t k = 0) const {
        if (i + k >= src.size()) return '\0';
        return src.at(i + k);
    }

    char get() {
        if (i >= src.size()) return '\0';
        char c = src[i++];
        if (c == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        return c;
    }

    bool isIdentifierStart(char c) { return isalpha((unsigned char)c) || c == '_'; }
    bool isIdentifierPart(char c)  { return isalnum((unsigned char)c) || c == '_'; }
    bool isDigit(char c)           { return isdigit((unsigned char)c); }
    bool isWhitespace(char c)      { return c == ' ' || c == '\t' || c == '\n'; }

public:
    explicit Lexer(string s) : src(move(s)) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        while (true) {
            Token t = nextToken();
            tokens.push_back(t);
            if (t.tipo == TokenType::END_OF_FILE) break;
        }
        return tokens;
    }

    Token nextToken() {
        while (true) {
            char c = peek();
            if (c == '\0') return Token(TokenType::END_OF_FILE, "<EOF>", line, col);
            if (isWhitespace(c)) { get(); continue; }
            break;
        }

        int tokLine = line;
        int tokCol  = col;
        char c = get();

        // Identificadores e keywords
        if (isIdentifierStart(c)) {
            string lex(1, c);
            while (isIdentifierPart(peek())) lex.push_back(get());
            if (keywords.count(lex)) return Token(TokenType::KEYWORD, lex, tokLine, tokCol);
            return Token(TokenType::IDENTIFIER, lex, tokLine, tokCol);
        }

        // Números (int e real)
        if (isDigit(c)) {
            string lex(1, c);
            bool hasDot = false;
            while (true) {
                char p = peek();
                if (p == '.' && !hasDot && isDigit(peek(1))) {
                    hasDot = true;
                    lex.push_back(get());
                } else if (isDigit(p)) {
                    lex.push_back(get());
                } else break;
            }
            if (hasDot) return Token(TokenType::NUM_REAL, lex, tokLine, tokCol);
            return Token(TokenType::NUM_INT, lex, tokLine, tokCol);
        }

        // Strings
        if (c == '"') {
            string lex;
            lex.push_back(c);
            bool closed = false;
            while (true) {
                char p = get();
                if (p == '\0') break;
                lex.push_back(p);
                if (p == '\\') {
                    char nxt = get();
                    if (nxt == '\0') break;
                    lex.push_back(nxt);
                    continue;
                }
                if (p == '"') { closed = true; break; }
            }
            if (!closed) {
                return Token(TokenType::UNKNOWN, lex + "(String nunca foi fechada)", tokLine, tokCol);
            }
            return Token(TokenType::STRING, lex, tokLine, tokCol);
        }

        // Comentários de linha
        if (c == '/' && peek() == '/') {
            string lex;
            lex.push_back(c);
            lex.push_back(get());
            while (peek() != '\n' && peek() != '\0') lex.push_back(get());
            return Token(TokenType::COMMENT, lex, tokLine, tokCol);
        }

        // Operadores duplos
        if ((c == '=' && peek() == '=') || (c == '!' && peek() == '=') ||
            (c == '<' && peek() == '=') || (c == '>' && peek() == '=') ||
            (c == '&' && peek() == '&') || (c == '|' && peek() == '|')) {
            string lex;
            lex.push_back(c);
            lex.push_back(get());
            return Token(TokenType::OPERATOR, lex, tokLine, tokCol);
        }

        // Operadores simples
        string singleOps = "+-*/=<>%";
        if (singleOps.find(c) != string::npos) {
            string lex(1, c);
            return Token(TokenType::OPERATOR, lex, tokLine, tokCol);
        }

        // Pontuação
        string punct = "();,{}[]";
        if (punct.find(c) != string::npos) {
            string lex(1, c);
            return Token(TokenType::PUNCTUATION, lex, tokLine, tokCol);
        }

        // Caractere desconhecido
        string lex(1, c);
        return Token(TokenType::UNKNOWN, lex, tokLine, tokCol);
    }
};

// Funções utilitárias para o pipeline
inline vector<Token> tokenizeSource(const string& source) {
    Lexer lexer(source);
    return lexer.tokenize();
}

inline vector<Token> tokenizeFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        throw runtime_error("Nao foi possivel abrir: " + filename);
    }
    stringstream ss;
    ss << file.rdbuf();
    return tokenizeSource(ss.str());
}
