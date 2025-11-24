// semantic.cpp - checagem semântica básica usando a AST
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include "../parser/parser.cpp"

enum class TypeKind {
    INT,
    REAL,
    STRING,
    BOOL,
    UNKNOWN
};

struct SemanticError {
    std::string message;
    int linha;
    int coluna;
};

struct SemanticResult {
    std::unordered_map<std::string, TypeKind> symbols;
    std::vector<SemanticError> errors;
};

static std::string typeToString(TypeKind t) {
    switch (t) {
        case TypeKind::INT:    return "int";
        case TypeKind::REAL:   return "real";
        case TypeKind::STRING: return "string";
        case TypeKind::BOOL:   return "bool";
        default:               return "unknown";
    }
}

static TypeKind literalType(const Token& tok) {
    switch (tok.tipo) {
        case TokenType::NUM_INT:  return TypeKind::INT;
        case TokenType::NUM_REAL: return TypeKind::REAL;
        case TokenType::STRING:   return TypeKind::STRING;
        default:                  return TypeKind::UNKNOWN;
    }
}

static void report(std::vector<SemanticError>& errs, const std::string& msg, const Token& tok) {
    errs.push_back({msg, tok.linha, tok.coluna});
}

// Retorna o tipo resultante e acumula erros
static TypeKind evalExpr(const std::shared_ptr<ASTNode>& node,
                         SemanticResult& ctx) {
    if (!node) return TypeKind::UNKNOWN;

    switch (node->kind) {
        case NodeKind::Literal:
            return literalType(node->token);

        case NodeKind::Identifier: {
            auto it = ctx.symbols.find(node->value);
            if (it == ctx.symbols.end()) {
                report(ctx.errors, "variavel '" + node->value + "' usada sem declarar", node->token);
                return TypeKind::UNKNOWN;
            }
            return it->second;
        }

        case NodeKind::Binary: {
            TypeKind lt = evalExpr(node->children[0], ctx);
            TypeKind rt = evalExpr(node->children[1], ctx);
            const std::string& op = node->value;

            auto isNumeric = [](TypeKind t) {
                return t == TypeKind::INT || t == TypeKind::REAL;
            };

            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
                if (!isNumeric(lt) || !isNumeric(rt)) {
                    report(ctx.errors, "operador '" + op + "' exige operandos numéricos", node->token);
                    return TypeKind::UNKNOWN;
                }
                if (op == "%" && (lt != TypeKind::INT || rt != TypeKind::INT)) {
                    report(ctx.errors, "operador '%' exige operandos int", node->token);
                }
                return (lt == TypeKind::REAL || rt == TypeKind::REAL) ? TypeKind::REAL : TypeKind::INT;
            }

            if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
                if (!isNumeric(lt) || !isNumeric(rt)) {
                    report(ctx.errors, "comparação '" + op + "' exige operandos numéricos", node->token);
                }
                return TypeKind::BOOL;
            }

            if (op == "&&" || op == "||") {
                if (lt != TypeKind::BOOL || rt != TypeKind::BOOL) {
                    report(ctx.errors, "operador lógico '" + op + "' exige operandos bool", node->token);
                }
                return TypeKind::BOOL;
            }

            return TypeKind::UNKNOWN;
        }

        default:
            return TypeKind::UNKNOWN;
    }
}

static void checkNode(const std::shared_ptr<ASTNode>& node, SemanticResult& ctx) {
    if (!node) return;

    switch (node->kind) {
        case NodeKind::Program:
        case NodeKind::Block:
            for (auto& c : node->children) checkNode(c, ctx);
            break;

        case NodeKind::Decl: {
            // Assumindo tipo int por enquanto
            TypeKind declType = TypeKind::INT;
            const std::string& name = node->value;
            if (ctx.symbols.count(name)) {
                report(ctx.errors, "variavel '" + name + "' redeclarada", node->token);
            }
            ctx.symbols[name] = declType;
            break;
        }

        case NodeKind::Assign: {
            const auto& idNode = node->children[0];
            const std::string& name = idNode->value;
            auto it = ctx.symbols.find(name);
            TypeKind target = TypeKind::UNKNOWN;
            if (it == ctx.symbols.end()) {
                report(ctx.errors, "variavel '" + name + "' usada sem declarar", idNode->token);
            } else {
                target = it->second;
            }
            TypeKind exprType = evalExpr(node->children[1], ctx);
            if (target != TypeKind::UNKNOWN && exprType != TypeKind::UNKNOWN && target != exprType) {
                bool numericCompat = (target == TypeKind::REAL && exprType == TypeKind::INT);
                if (!numericCompat) {
                    report(ctx.errors, "tipos incompatíveis na atribuição: esperado " +
                        typeToString(target) + ", obtido " + typeToString(exprType), node->token);
                }
            }
            break;
        }

        case NodeKind::If: {
            if (!node->children.empty()) {
                TypeKind condType = evalExpr(node->children[0], ctx);
                if (condType != TypeKind::BOOL && condType != TypeKind::UNKNOWN) {
                    report(ctx.errors, "condição do if deve ser bool", node->children[0]->token);
                }
            }
            for (size_t i = 1; i < node->children.size(); ++i) {
                checkNode(node->children[i], ctx);
            }
            break;
        }

        default:
            // outros nós já são cobertos em evalExpr quando usados
            break;
    }
}

inline SemanticResult checkProgram(const std::shared_ptr<ASTNode>& root) {
    SemanticResult res;
    checkNode(root, res);
    return res;
}
