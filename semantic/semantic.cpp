// semantic.cpp - checagem semântica básica usando a AST
// Compiladores - Analisador Semântico
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include "../parser/parser.cpp"

// Definição dos tipos de dados
enum class TypeKind {
    INT,
    REAL,
    STRING,
    BOOL,
    UNKNOWN
};

// Estrutura de erro semântico
struct SemanticError {
    std::string message;
    int linha;
    int coluna;
};

// Resultado da análise semântica
struct SemanticResult {
    std::unordered_map<std::string, TypeKind> symbols;
    std::vector<SemanticError> errors;
};

// Converte TypeKind para string
static std::string typeToString(TypeKind t) {
    switch (t) {
        case TypeKind::INT:    return "int";
        case TypeKind::REAL:   return "real";
        case TypeKind::STRING: return "string";
        case TypeKind::BOOL:   return "bool";
        default:               return "unknown";
    }
}

// Retorna o tipo de um literal baseado no token
static TypeKind literalType(const Token& tok) {
    switch (tok.tipo) {
        case TokenType::NUM_INT:  return TypeKind::INT;
        case TokenType::NUM_REAL: return TypeKind::REAL;
        case TokenType::STRING:   return TypeKind::STRING;
        default:                  return TypeKind::UNKNOWN;
    }
}

// Registra um erro semântico
static void report(std::vector<SemanticError>& errs, const std::string& msg, const Token& tok) {
    errs.push_back({msg, tok.linha, tok.coluna});
}

// Retorna o tipo resultante e verifica tipos em expressões 
static TypeKind evalExpr(const std::shared_ptr<ASTNode>& node, SemanticResult& ctx) {

    if (!node) return TypeKind::UNKNOWN;                        // nó nulo

    switch (node->kind) {                                       // tipo do nó

        case NodeKind::Literal:                                 // literal
            return literalType(node->token);

        case NodeKind::Identifier: {                            // identificador
            auto it = ctx.symbols.find(node->value);            // procura na tabela de símbolos
            if (it == ctx.symbols.end()) {                      // não declarado
                report(ctx.errors, "variavel '" + node->value + "' usada sem declarar", node->token);
                return TypeKind::UNKNOWN;
            }
            return it->second;                                  // retorna o tipo declarado
        }

        case NodeKind::Binary: {                                // expressão binária
            TypeKind lt = evalExpr(node->children[0], ctx);     // tipo do operando esquerdo
            TypeKind rt = evalExpr(node->children[1], ctx);     // tipo do operando direito
            const std::string& op = node->value;                // operador

            auto isNumeric = [](TypeKind t) {                   // verifica se é numérico
                return t == TypeKind::INT || t == TypeKind::REAL;
            };

            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {    // operadores aritméticos
                if (!isNumeric(lt) || !isNumeric(rt)) {                             // tipos inválidos
                    report(ctx.errors, "operador '" + op + "' exige operandos numéricos", node->token);
                    return TypeKind::UNKNOWN;
                }
                if (op == "%" && (lt != TypeKind::INT || rt != TypeKind::INT)) {    // módulo só para int
                    report(ctx.errors, "operador '%' exige operandos int", node->token);
                }
                return (lt == TypeKind::REAL || rt == TypeKind::REAL) ? TypeKind::REAL : TypeKind::INT;// resultado é real se algum operando for real
            }

            if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {   // operadores relacionais
                if (!isNumeric(lt) || !isNumeric(rt)) {                             // tipos inválidos
                    report(ctx.errors, "comparação '" + op + "' exige operandos numéricos", node->token);
                }
                return TypeKind::BOOL;
            }

            if (op == "&&" || op == "||") {                                         // operadores lógicos
                if (lt != TypeKind::BOOL || rt != TypeKind::BOOL) {                 // tipos inválidos
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

// Função recursiva para checar a AST
static void checkNode(const std::shared_ptr<ASTNode>& node, SemanticResult& ctx) {
    if (!node) return;

    // Verifica o nó baseado no tipo
    switch (node->kind) {
        case NodeKind::Program:
        case NodeKind::Block:
            for (auto& c : node->children) checkNode(c, ctx);
            break;
        
        // Declaração de variável
        case NodeKind::Decl: {  
            // Assumindo tipo int por enquanto
            TypeKind declType = TypeKind::INT;      // tipo fixo para simplificação
            const std::string& name = node->value;  // nome da variável
            if (ctx.symbols.count(name)) {          // verifica redeclaração
                report(ctx.errors, "variavel '" + name + "' redeclarada", node->token); 
            }
            ctx.symbols[name] = declType;           // registra na tabela de símbolos
            break;
        }

        // Atribuição
        case NodeKind::Assign: {                 
            const auto& idNode = node->children[0];         // nó Identificador
            const std::string& name = idNode->value;        // nome da variável
            auto it = ctx.symbols.find(name);               // procura na tabela de símbolos
            TypeKind target = TypeKind::UNKNOWN;            // tipo alvo
            if (it == ctx.symbols.end()) {                  // sem declaração
                report(ctx.errors, "variavel '" + name + "' usada sem declarar", idNode->token);
            } else {                                         // com declaração
                target = it->second;
            }
            TypeKind exprType = evalExpr(node->children[1], ctx);   // tipo da expressão atribuída
            if (target != TypeKind::UNKNOWN && exprType != TypeKind::UNKNOWN && target != exprType) {// tipos incompatíveis
                bool numericCompat = (target == TypeKind::REAL && exprType == TypeKind::INT);
                if (!numericCompat) {                               // permite int para real
                    report(ctx.errors, "tipos incompatíveis na atribuição: esperado " +
                        typeToString(target) + ", obtido " + typeToString(exprType), node->token);
                }
            }
            break;
        }

        // If statement
        case NodeKind::If: {    
            if (!node->children.empty()) {                                              // verifica condição
                TypeKind condType = evalExpr(node->children[0], ctx);
                if (condType != TypeKind::BOOL && condType != TypeKind::UNKNOWN) {      // se a condição não é bool
                    report(ctx.errors, "condição do if deve ser bool", node->children[0]->token);
                }
            }
            for (size_t i = 1; i < node->children.size(); ++i) {                        // verifica ramos then/else
                checkNode(node->children[i], ctx);
            }
            break;
        }

        default:
            // outros nós já são cobertos em evalExpr quando usados
            break;
    }
}

// Função principal para checagem semântica do programa
inline SemanticResult checkProgram(const std::shared_ptr<ASTNode>& root) {      
    SemanticResult res;
    checkNode(root, res);
    return res;
}
