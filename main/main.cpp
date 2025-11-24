// main.cpp - CLI para lexer, parser (AST), semântica e execução
#include <iostream>
#include <string>
#include <vector>
#include "../semantic/semantic.cpp"
#include "../exec/exec.cpp"

using namespace std;

// Imprime uso do programa
static void printUsage(const string& prog) {
    cerr << "Uso:\n";
    cerr << "  " << prog << " --tokens <arquivo>\n";
    cerr << "  " << prog << " --ast    <arquivo>\n";
    cerr << "  " << prog << " --run    <arquivo>\n";
}

// Impressão simples da AST
static void printAst(const shared_ptr<ASTNode>& node, int indent = 0) {
    if (!node) return;                              // nó nulo
    for (int i = 0; i < indent; ++i) cout << "  ";  // indentação

    // converte NodeKind para string
    auto kindToStr = [](NodeKind k) {
        switch (k) {
            case NodeKind::Program:    return "Program";
            case NodeKind::Block:      return "Block";
            case NodeKind::Decl:       return "Decl";
            case NodeKind::Assign:     return "Assign";
            case NodeKind::If:         return "If";
            case NodeKind::Binary:     return "Binary";
            case NodeKind::Literal:    return "Literal";
            case NodeKind::Identifier: return "Identifier";
        }
        return "Node";
    };

    // imprime nó
    cout << kindToStr(node->kind) << " : \"" << node->value << "\""
         << " [" << node->token.linha << "," << node->token.coluna << "]\n";
    for (const auto& child : node->children) {  // filhos
        printAst(child, indent + 1);
    }
}

// Função principal
int main(int argc, char** argv) {
    if (argc != 3) {                // espera 2 argumentos  
        printUsage(argv[0]);    
        return 1;
    }

    string mode = argv[1];          // modo de operação
    string filename = argv[2];      // arquivo de entrada

    try {
        // modo de tokens 
        if (mode == "--tokens") {
            auto tokens = tokenizeFile(filename);   
            for (const auto& t : tokens) {
                cout << t.toString() << "\n";
            }
            return 0;
        }

        // modo de AST
        if (mode == "--ast") {
            auto tokens = tokenizeFile(filename);
            Parser parser(tokens);
            auto ast = parser.parse();
            printAst(ast);
            auto sem = checkProgram(ast);
            for (const auto& e : sem.errors) {
                std::cerr << "[Erro semantico] " << e.message
                          << " (" << e.linha << "," << e.coluna << ")\n";
            }
            return 0;
        }

        // modo de execução
        if (mode == "--run") {
            auto tokens = tokenizeFile(filename);
            Parser parser(tokens);
            auto ast = parser.parse();
            auto sem = checkProgram(ast);
            if (!sem.errors.empty()) {
                for (const auto& e : sem.errors) {
                    std::cerr << "[Erro semantico] " << e.message
                              << " (" << e.linha << "," << e.coluna << ")\n";
                }
                return 1;
            }
            std::unordered_map<std::string, RuntimeValue> values;
            runProgram(ast, sem.symbols, values);
            // Opcional: imprimir estado final
            for (const auto& [name, val] : values) {
                std::cout << name << " = ";
                switch (val.type) {
                    case TypeKind::INT: std::cout << val.i; break;
                    case TypeKind::REAL: std::cout << val.d; break;
                    case TypeKind::STRING: std::cout << val.s; break;
                    case TypeKind::BOOL: std::cout << (val.b ? "true" : "false"); break;
                    default: std::cout << "<unknown>";
                }
                std::cout << "\n";
            }
            return 0;
        }

        printUsage(argv[0]);
        return 1;
    } catch (const exception& e) {
        cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
}
