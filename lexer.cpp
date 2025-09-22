
// lexer.cpp
// Compiladores - Analisador Léxico 
#include <iostream>  
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype> 
using namespace std;

// Definição dos tipos de tokens
enum class TokenType {
    IDENTIFIER, NUMBER, STRING, KEYWORD,
    OPERATOR, PUNCTUATION, END_OF_FILE, UNKNOWN, COMMENT
};

// Estrutura dos tokens
struct Token {
    TokenType tipo;
    string texto;
    int linha;
    int coluna;

    // Construtor
    Token(TokenType t, string l, int ln, int col) : tipo(t), texto(move(l)), linha(ln), coluna(col) {}
    
    // Converter o token para string
    string toString() const { 
        
        auto tt = [](TokenType t){  
            switch(t){
                case TokenType::IDENTIFIER: return "IDENTIFICADOR"; 
                case TokenType::NUMBER:     return "NUMERO";
                case TokenType::STRING:     return "STRING";
                case TokenType::KEYWORD:    return "KEYWORD";
                case TokenType::OPERATOR:   return "OPERADOR";
                case TokenType::PUNCTUATION:return "PONTUACAO";
                case TokenType::END_OF_FILE:return "FIM DE ARQUIVO";
                case TokenType::UNKNOWN:    return "UNKNOWN";
                case TokenType::COMMENT:    return "COMMENTARIO";
            }
            return "TOKEN"; // caso não ocorra (precaução)
        };

        // Retorna a representação em string do token
        // exemplo: IDENTIFIER -> "nomeVariavel" [linha,coluna]
        return string(tt(tipo)) + " -> \"" + texto + "\" [" + to_string(linha) + "," + to_string(coluna) + "]";
    }
};

// Analisador Léxico
class Lexer {
    string src;         // código fonte
    size_t i = 0;       // indice
    int line = 1;
    int col = 1;

// Conjunto de palavras-chave reconhecidas
unordered_set<string> keywords = {
    "if",
    "else",
    "while",
    "for",
    "switch",
    "case",
    "return",
    "int",
    "float",
    "string",
    "boolean",
    "void",
    "break",
    "continue",
    "true",
    "false",
    "null",
    "do",
    "enum",
    "struct",
    "typedef",
    "const",
    "static",
    "public",
    "private",
    "protected",
    "class",
    "new",
    "this",
    "super",
    "import",
    "package",
    "include"
};

    // Retorna o próximo caractere avançar no indice
    char peek(size_t k=0) const { 
        if (i + k >= src.size()) return '\0';   // se passou do fim, retorna NULL
        return src.at(i + k);                   // retorna o caractere atual + k

    }

    // Consome o próximo caractere e avança o indice
    char get() {
        if (i >= src.size()) return '\0';       // se passou do fim, retorna NULL
        char c = src[i++];                      // usa o caractere atual e avança o índice
        if (c == '\n') {                        // verifica se é o fim da linha e atualiza linha e coluna
            line++; col = 1; 
        } else col++;                           // senão apenas avança a coluna

        return c;                               // retorna o caractere consumido
    }

    // Retorna o indice
    void retreat() {
        if (i==0) return;                       // não retrocede se ja ta no inicio
        i--;
        if (src[i] == '\n') {                   // se retrocedeu uma linha
            line = max(1, line-1);              // não deixa linha ser menor que 1
            col = 1;                            // reseta coluna para 1

        } else {                                // se retrocedeu um caractere normal
            col = max(1, col-1);                // não deixa coluna ser menor que 1
        }
    }

    // identificar tipos de caracteres
    bool isIdentifierStart(char c) { 
        return isalpha((unsigned char)c);       // letra
    }    
    bool isIdentifierPart(char c) { 
        return isalnum((unsigned char)c);       // letra, numero
    }     
    bool isDigit(char c) { 
        return isdigit((unsigned char)c);       // numero
    }                 
    bool isWhitespace(char c) { 
        return c==' '||c=='\t'||c=='\n';        // espaço, tab, nova linha
    }         

public:
    // Construtor, arquivo -> string
    Lexer(string s): src(move(s)) {}

    // Tokeniza todo o código fonte e retorna a lista de tokens
    vector<Token> tokenize() {      

        vector<Token> tokens;                               // lista de tokens

        while (true) {                                      
            Token t = nextToken();                          // pega o próximo token   
            tokens.push_back(t);                            // adiciona na lista
            if (t.tipo == TokenType::END_OF_FILE) break;    // se for final de arquivo, ter0mina
        }
        return tokens;
    }

    // Retorna o próximo token
    Token nextToken() {                 
        
        while (true) {
            // olha o próximo caractere sem consumir
            char c = peek();

            // se for fim de arquivo, retorna token EOF
            if (c == '\0') return Token(TokenType::END_OF_FILE, "<EOF>", line, col);
            
            // pula espaços em branco, tab e nova linha
            if (isWhitespace(c)) {  
                get();
                continue;
            }
            break;
        }

        int tokLine = line;                 // guarda a linha do início do token
        int tokCol = col;                   // guarda a coluna do início do token
        char c = get();                     // consome o próximo caractere

        // Identificadores e keywords
        if (isIdentifierStart(c)) {
            string lex;
            lex.push_back(c);               // adiciona o caractere inicial
            
            while (isIdentifierPart(peek())) lex.push_back(get());          // consome o resto do identificador
                                         
            if (keywords.count(lex)) {                                      // se for palavra-chave
                return Token(TokenType::KEYWORD, lex, tokLine, tokCol);     // retorna token keyword
            }
            return Token(TokenType::IDENTIFIER, lex, tokLine, tokCol);      // retorna token identificador                          
        }

        // Numeros (inteiros e decimais)
        if (isDigit(c)) {
            string lex;
            lex.push_back(c);           // adiciona o caractere inicial
            bool hasDot = false;        // flag para ponto decimal
            while (true) {              // consome o resto do número
                char p = peek();        // olha o próximo caractere

                if (p == '.' && !hasDot && isDigit(peek(1))) {  // ponto decimal
                    hasDot = true;                              // marca que já tem ponto
                    lex.push_back(get());                       // consume o ponto
                } else if (isDigit(p)) {                        // se for dígito
                    lex.push_back(get());                   
                } else break;                                   // senão termina
            }
            return Token(TokenType::NUMBER, lex, tokLine, tokCol);  // retorna token número
        }

        // Strings 
        if (c == '"') {
            string lex;                 
            lex.push_back(c);               // adiciona a aspa inicial
            bool closed = false;            // flag para fechar string

            while (true) {                  // consome o resto da string
                char p = get();             // consome o próximo caractere

                if (p == '\0') break;       // se for fim de arquivo, termina

                lex.push_back(p);           // se não adiciona o caractere na string

                if (p == '\\') {            // escape sequence
                    
                    char nxt = get();   
                    if (nxt == '\0') break; // se for fim de arquivo, termina
                                            
                    lex.push_back(nxt);     // adiciona o caractere escapado
                    continue;               
                }

                if (p == '"') { closed = true; break; } // se encontrar a aspa final, fecha a string
            }

            if (!closed) {                              // se não fechou a string, erro
                return Token(TokenType::UNKNOWN, lex + "(String nunca foi fechada)", tokLine, tokCol);  // retorna token desconhecido
            }
            return Token(TokenType::STRING, lex, tokLine, tokCol);  // retorna token string
        }

        // Comentários de linha: // comentário
        if (c == '/' && peek() == '/') {                                        // se encontrar //(comentário)
            string lex;                                                         
            lex.push_back(c);                                                   // adiciona o primeiro '/' 
            lex.push_back(get());                                               // adiciona o segundo '/'
            while (peek() != '\n' && peek() != '\0') lex.push_back(get());      // consome o resto da linha
            return Token(TokenType::COMMENT, lex, tokLine, tokCol);             // retorna token comentário
        }

        // Operadores boleanos(duplos): ==, !=, <=, >=
        if ((c == '=' && peek() == '=') || (c == '!' && peek() == '=') ||
            (c == '<' && peek() == '=') || (c == '>' && peek() == '=')
            (c == '&' && peek() == '&') || (c == '|' && peek() == '|')) {

            string lex;
            lex.push_back(c);                                               // adiciona o primeiro caractere
            lex.push_back(get());                                           // adiciona o segundo caractere
            return Token(TokenType::OPERATOR, lex, tokLine, tokCol);        // retorna token operador
        }

        // Operadores simples: +, -, *, /, =, <, >, %
        string singleOps = "+-*/=<>%";                                      // define os operadores simples

        if (singleOps.find(c) != string::npos) {                            // se o caractere for um operador simples    
            string lex(1,c);                                                // cria a string do operador  
            return Token(TokenType::OPERATOR, lex, tokLine, tokCol);        // retorna token operador
        }

        // Pontuação: ();,{}[]
        string punct = "();,{}[]";                                          // define os caracteres de pontuação

        if (punct.find(c) != string::npos) {                                // se o caractere for de pontuação
            string lex(1,c);                                                // cria a string da pontuação
            return Token(TokenType::PUNCTUATION, lex, tokLine, tokCol);     // retorna token pontuação
        }

        // Caractere desconhecido
        string lex(1, c);                                                   // cria a string do caractere
        return Token(TokenType::UNKNOWN, lex, tokLine, tokCol);             // retorna token desconhecido
    }
};

// Função para salvar tokens em um arquivo
void saveTokensToFile(const vector<Token>& tokens, const string& filename) { 

    ofstream outFile(filename);                                             
    if (!outFile) {                                                        
        cerr << "Não foi possível abrir o arquivo para escrita: " << filename << "\n";
        return;
    }
    for (const auto& token : tokens) {           
        outFile << token.toString() << "\n";      // escreve cada token em uma nova linha                           
    }
    outFile.close();
}

// Programa principal para testar o analisador lexico
int main(int argc, char** argv) {               

    if (argc < 2) {                                             // verifica se o arquivo foi passado como argumento
        cerr << "Uso: " << argv[0] << " <arquivo_fonte>\n";     // se não, exibe mensagem de uso
        return 1;
    }

    ifstream file(argv[1]);                                     // abre o arquivo fonte
    if (!file) {                                                // verifica se o arquivo foi aberto com sucesso
        cerr << "Não foi possível abrir: " << argv[1] << "\n";  // se não foi possivel abrir, exibe mensagem de erro
        return 1;
    }

    stringstream ss;                                            // cria um stringstream
    ss << file.rdbuf();                                         // lê todo o conteúdo do arquivo para o stringstream
    
    Lexer lexer(ss.str());                                      // cria o analisador léxico com o conteúdo do arquivo  
    auto tokens = lexer.tokenize();                             // tokeniza o conteúdo
    for (const auto &t : tokens) {                              // para cada token encontrado
        cout << t.toString() << "\n";                           // exibe o token
    }

    saveTokensToFile(tokens, "tokens.txt");                     // salva os tokens em um arquivo
    return 0;
}
