// exec.cpp - interpretador simples da AST
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <iostream>

// Usa ASTNode/NodeKind/TypeKind definidos pelos includes anteriores

// estrutura para valores em tempo de execução
struct RuntimeValue {
    TypeKind type{TypeKind::UNKNOWN};
    int i{0};
    double d{0.0};
    std::string s;
    bool b{false};
};

// cria valor padrão para um tipo
static RuntimeValue makeDefault(TypeKind t) {
    RuntimeValue v;
    v.type = t;
    switch (t) {
        case TypeKind::INT:  v.i = 0; break;
        case TypeKind::REAL: v.d = 0.0; break;
        case TypeKind::STRING: v.s = ""; break;
        case TypeKind::BOOL: v.b = false; break;
        default: break;
    }
    return v;
}

// obtém valor de literal a partir do token
static RuntimeValue literalValue(const Token& tok) {
    RuntimeValue v;
    if (tok.tipo == TokenType::NUM_INT) {
        v.type = TypeKind::INT;
        v.i = std::stoi(tok.texto);
    } else if (tok.tipo == TokenType::NUM_REAL) {
        v.type = TypeKind::REAL;
        v.d = std::stod(tok.texto);
    } else if (tok.tipo == TokenType::STRING) {
        v.type = TypeKind::STRING;
        v.s = tok.texto;
    } else if (tok.tipo == TokenType::KEYWORD && (tok.texto == "true" || tok.texto == "false")) {
        v.type = TypeKind::BOOL;
        v.b = (tok.texto == "true");
    } else {
        v.type = TypeKind::UNKNOWN;
    }
    return v;
}

// promove valor int para real
static RuntimeValue promoteToReal(const RuntimeValue& v) {
    if (v.type == TypeKind::REAL) return v;
    RuntimeValue r;
    r.type = TypeKind::REAL;
    r.d = static_cast<double>(v.i);
    return r;
}

// avalia expressão e retorna valor em tempo de execução
static RuntimeValue evalExpr(const std::shared_ptr<ASTNode>& node,
                             const std::unordered_map<std::string, TypeKind>& symbols,
                             std::unordered_map<std::string, RuntimeValue>& values) {
    if (!node) return {};

    switch (node->kind) {
        case NodeKind::Literal:
            return literalValue(node->token);

        case NodeKind::Identifier: {
            auto it = values.find(node->value);
            if (it == values.end()) {
                throw std::runtime_error("Variavel '" + node->value + "' sem valor em tempo de execucao");
            }
            return it->second;
        }

        case NodeKind::Binary: {
            RuntimeValue left = evalExpr(node->children[0], symbols, values);
            RuntimeValue right = evalExpr(node->children[1], symbols, values);
            const std::string& op = node->value;

            auto requireNumeric = [&](const RuntimeValue& v, const std::string& side) {
                if (v.type != TypeKind::INT && v.type != TypeKind::REAL) {
                    throw std::runtime_error("Operando nao numerico em '" + op + "': " + side);
                }
            };

            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
                requireNumeric(left, "esquerda");
                requireNumeric(right, "direita");

                if (op == "%") {
                    if (left.type != TypeKind::INT || right.type != TypeKind::INT) {
                        throw std::runtime_error("Operador '%' exige int");
                    }
                    RuntimeValue r; r.type = TypeKind::INT; r.i = left.i % right.i; return r;
                }

                // promoção para real se necessário
                bool realResult = (left.type == TypeKind::REAL) || (right.type == TypeKind::REAL);
                if (realResult) {
                    RuntimeValue l = (left.type == TypeKind::REAL) ? left : promoteToReal(left);
                    RuntimeValue r = (right.type == TypeKind::REAL) ? right : promoteToReal(right);
                    RuntimeValue res; res.type = TypeKind::REAL;
                    if (op == "+") res.d = l.d + r.d;
                    else if (op == "-") res.d = l.d - r.d;
                    else if (op == "*") res.d = l.d + r.d * (op == "*" ? 1 : 0); // placeholder to avoid warning
                    else if (op == "/") res.d = l.d / r.d;
                    // corrigir multiplicação
                    if (op == "*") res.d = l.d * r.d;
                    return res;
                } else {
                    RuntimeValue res; res.type = TypeKind::INT;
                    if (op == "+") res.i = left.i + right.i;
                    else if (op == "-") res.i = left.i - right.i;
                    else if (op == "*") res.i = left.i * right.i;
                    else if (op == "/") res.i = left.i / right.i;
                    return res;
                }
            }

            if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
                requireNumeric(left, "esquerda");
                requireNumeric(right, "direita");
                RuntimeValue res; res.type = TypeKind::BOOL;
                double l = (left.type == TypeKind::REAL) ? left.d : left.i;
                double r = (right.type == TypeKind::REAL) ? right.d : right.i;
                if (op == "==") res.b = (l == r);
                else if (op == "!=") res.b = (l != r);
                else if (op == "<") res.b = (l < r);
                else if (op == ">") res.b = (l > r);
                else if (op == "<=") res.b = (l <= r);
                else if (op == ">=") res.b = (l >= r);
                return res;
            }

            if (op == "&&" || op == "||") {
                if (left.type != TypeKind::BOOL || right.type != TypeKind::BOOL) {
                    throw std::runtime_error("Operador logico '" + op + "' exige bool");
                }
                RuntimeValue res; res.type = TypeKind::BOOL;
                if (op == "&&") res.b = left.b && right.b;
                else res.b = left.b || right.b;
                return res;
            }

            throw std::runtime_error("Operador nao suportado: " + op);
        }

        default:
            return {};
    }
}

static void execNode(const std::shared_ptr<ASTNode>& node,
                     const std::unordered_map<std::string, TypeKind>& symbols,
                     std::unordered_map<std::string, RuntimeValue>& values) {
    if (!node) return;

    switch (node->kind) {
        case NodeKind::Program:
        case NodeKind::Block:
            for (auto& c : node->children) execNode(c, symbols, values);
            break;

        case NodeKind::Decl: {
            const std::string& name = node->value;
            auto itType = symbols.find(name);
            TypeKind t = (itType != symbols.end()) ? itType->second : TypeKind::UNKNOWN;
            RuntimeValue v = makeDefault(t);
            if (node->children.size() > 1) {
                RuntimeValue init = evalExpr(node->children[1], symbols, values);
                // atribui se compatível
                if (t == TypeKind::REAL && init.type == TypeKind::INT) {
                    v.type = TypeKind::REAL; v.d = static_cast<double>(init.i);
                } else if (t == init.type || t == TypeKind::UNKNOWN) {
                    v = init;
                } else {
                    throw std::runtime_error("Inicializacao incompatível de '" + name + "'");
                }
            }
            values[name] = v;
            break;
        }

        case NodeKind::Assign: {
            const std::string& name = node->children[0]->value;
            RuntimeValue rhs = evalExpr(node->children[1], symbols, values);
            auto itType = symbols.find(name);
            TypeKind target = (itType != symbols.end()) ? itType->second : TypeKind::UNKNOWN;
            if (target == TypeKind::REAL && rhs.type == TypeKind::INT) {
                rhs = promoteToReal(rhs);
            } else if (target != TypeKind::UNKNOWN && rhs.type != target) {
                throw std::runtime_error("Atribuicao incompatível para '" + name + "'");
            }
            values[name] = rhs;
            break;
        }

        case NodeKind::If: {
            RuntimeValue cond = evalExpr(node->children[0], symbols, values);
            if (cond.type != TypeKind::BOOL) {
                throw std::runtime_error("Condicao do if nao booleana");
            }
            if (cond.b) {
                execNode(node->children[1], symbols, values);
            } else if (node->children.size() > 2) {
                execNode(node->children[2], symbols, values);
            }
            break;
        }

        default:
            // Literais/Identificadores apenas em expressões
            break;
    }
}

inline void runProgram(const std::shared_ptr<ASTNode>& root,
                       const std::unordered_map<std::string, TypeKind>& symbols,
                       std::unordered_map<std::string, RuntimeValue>& outValues) {
    execNode(root, symbols, outValues);
}
