// main.cpp - CLI para lexer e parser (AST)
#include <iostream>
#include <string>
#include <vector>
#include "../parser/parser.cpp"

using namespace std;

static void printUsage(const string& prog) {
    cerr << "Uso:\n";
    cerr << "  " << prog << " --tokens <arquivo>\n";
    cerr << "  " << prog << " --ast    <arquivo>\n";
}

// Impressão simples da AST
static void printAst(const shared_ptr<ASTNode>& node, int indent = 0) {
    if (!node) return;
    for (int i = 0; i < indent; ++i) cout << "  ";

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

    cout << kindToStr(node->kind) << " : \"" << node->value << "\""
         << " [" << node->token.linha << "," << node->token.coluna << "]\n";
    for (const auto& child : node->children) {
        printAst(child, indent + 1);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    string mode = argv[1];
    string filename = argv[2];

    try {
        if (mode == "--tokens") {
            auto tokens = tokenizeFile(filename);
            for (const auto& t : tokens) {
                cout << t.toString() << "\n";
            }
            return 0;
        }

        if (mode == "--ast") {
            auto tokens = tokenizeFile(filename);
            Parser parser(tokens);
            auto ast = parser.parse();
            printAst(ast);
            parser.printSemanticErrors();
            return 0;
        }

        printUsage(argv[0]);
        return 1;
    } catch (const exception& e) {
        cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
}
