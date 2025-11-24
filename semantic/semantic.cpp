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

// Mapeia token de tipo de declaração para TypeKind
static TypeKind mapDeclType(const Token& tok) {
    const std::string& t = tok.texto;
    if (t == "int") return TypeKind::INT;
    if (t == "float" || t == "double") return TypeKind::REAL;
    if (t == "string") return TypeKind::STRING;
    if (t == "bool" || t == "boolean") return TypeKind::BOOL;
    return TypeKind::UNKNOWN;
}

// Retorna o tipo de um literal baseado no token
static TypeKind literalType(const Token& tok) {
    switch (tok.tipo) {
        case TokenType::NUM_INT:  return TypeKind::INT;
        case TokenType::NUM_REAL: return TypeKind::REAL;
        case TokenType::STRING:   return TypeKind::STRING;
        case TokenType::KEYWORD:
            if (tok.texto == "true" || tok.texto == "false") return TypeKind::BOOL;
            return TypeKind::UNKNOWN;
        default:                  return TypeKind::UNKNOWN;
    }
}

// Registra um erro semântico
static void report(std::vector<SemanticError>& errs, const std::string& msg, const Token& tok) {
    errs.push_back({msg, tok.linha, tok.coluna});
}

// Retorna o tipo resultante e verifica tipos em expressões
static TypeKind evalExpr(const std::shared_ptr<ASTNode>& node, SemanticResult& ctx) {
    if (!node) return TypeKind::UNKNOWN;            // Evitar ponteiro nulo

    switch (node->kind) {                           // Tipo de nó
        //caso literal
        case NodeKind::Literal:
            return literalType(node->token);

        // caso identificador
        case NodeKind::Identifier: {
            auto it = ctx.symbols.find(node->value);
            if (it == ctx.symbols.end()) {
                report(ctx.errors, "variavel '" + node->value + "' usada sem declarar", node->token);
                return TypeKind::UNKNOWN;       // variável não declarada
            }
            return it->second;  // retornar tipo declarado
        }

        // caso binário
        case NodeKind::Binary: {
            TypeKind lt = evalExpr(node->children[0], ctx);     // tipo do operando esquerdo
            TypeKind rt = evalExpr(node->children[1], ctx);     // tipo do operando direito
            const std::string& op = node->value;

            auto isNumeric = [](TypeKind t) {                       // verifica se é tipo numérico
                return t == TypeKind::INT || t == TypeKind::REAL;   //retorna true se for int ou real
            };

            //se for operador aritmético
            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
                //verifica se ambos os operandos são numéricos
                if (!isNumeric(lt) || !isNumeric(rt)) {
                    report(ctx.errors, "operador '" + op + "' exige operandos numericos", node->token);
                    return TypeKind::UNKNOWN;   // tipo desconhecido
                }
                //verifica se o operador % tem operandos int
                if (op == "%" && (lt != TypeKind::INT || rt != TypeKind::INT)) {
                    report(ctx.errors, "operador '%' exige operandos int", node->token);
                }
                //retorna o tipo resultante
                return (lt == TypeKind::REAL || rt == TypeKind::REAL) ? TypeKind::REAL : TypeKind::INT;
            }

            //se for operador de comparação
            if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
                //verifica se ambos os operandos são numéricos
                if (!isNumeric(lt) || !isNumeric(rt)) {
                    report(ctx.errors, "comparacao '" + op + "' exige operandos numericos", node->token);
                }
                return TypeKind::BOOL;  // tipo booleano
            }

            //se for operador lógico
            if (op == "&&" || op == "||") {
                //verifica se ambos os operandos são booleanos
                if (lt != TypeKind::BOOL || rt != TypeKind::BOOL) {
                    report(ctx.errors, "operador logico '" + op + "' exige operandos bool", node->token);
                }
                return TypeKind::BOOL;  // tipo booleano
            }

            return TypeKind::UNKNOWN;   // tipo desconhecido
        }

        default:                        // outros tipos de nó
            return TypeKind::UNKNOWN;
    }
}

// Função recursiva para checar a AST
static void checkNode(const std::shared_ptr<ASTNode>& node, SemanticResult& ctx) {
    if (!node) return;                      // Evitar ponteiro nulo

    switch (node->kind) {                   // Tipo de nó
        //casos compostos
        case NodeKind::Program:
        //caso bloco
        case NodeKind::Block:
            for (auto& c : node->children) checkNode(c, ctx);// verifica cada filho 
            break;

        // caso declaração
        case NodeKind::Decl: {
            TypeKind declType = mapDeclType(node->token);       // tipo declarado
            const std::string& name = node->value;              // nome da variável
            // verifica redeclaração
            if (ctx.symbols.count(name)) {  
                report(ctx.errors, "variavel '" + name + "' redeclarada", node->token);
            }
            ctx.symbols[name] = declType;                       // adiciona ao contexto

            // verifica inicialização
            if (node->children.size() > 1) {
                TypeKind initType = evalExpr(node->children[1], ctx);
                // verifica compatibilidade de tipos
                if (declType != TypeKind::UNKNOWN && initType != TypeKind::UNKNOWN && declType != initType) {
                    bool numericCompat = (declType == TypeKind::REAL && initType == TypeKind::INT);
                    //verifica compatibilidade numérica
                    if (!numericCompat) {
                        report(ctx.errors, "tipos incompativeis na inicializacao: declarado " +
                            typeToString(declType) + ", obtido " + typeToString(initType), node->token);
                    }
                }
            }
            break;
        }

        // caso atribuição
        case NodeKind::Assign: {
            const auto& idNode = node->children[0];             // nó do identificador
            const std::string& name = idNode->value;
            auto it = ctx.symbols.find(name);                   // procura na tabela de símbolos
            TypeKind target = TypeKind::UNKNOWN;                // tipo alvo
            // verifica se a variável foi declarada
            if (it == ctx.symbols.end()) {
                report(ctx.errors, "variavel '" + name + "' usada sem declarar", idNode->token);
            }
            // se declarada, obtém o tipo 
            else {
                target = it->second;
            }
            // avalia o tipo da expressão atribuída
            TypeKind exprType = evalExpr(node->children[1], ctx);
            // verifica compatibilidade de tipos
            if (target != TypeKind::UNKNOWN && exprType != TypeKind::UNKNOWN && target != exprType) {
                bool numericCompat = (target == TypeKind::REAL && exprType == TypeKind::INT);
                //verifica compatibilidade numérica
                if (!numericCompat) {
                    report(ctx.errors, "tipos incompativeis na atribuicao: esperado " +
                        typeToString(target) + ", obtido " + typeToString(exprType), node->token);
                }
            }
            break;
        }

        // caso if
        case NodeKind::If: {
            // verifica tipo da condição
            if (!node->children.empty()) {
                TypeKind condType = evalExpr(node->children[0], ctx);
                // verifica se é booleano
                if (condType != TypeKind::BOOL && condType != TypeKind::UNKNOWN) {
                    report(ctx.errors, "condicao do if deve ser bool", node->children[0]->token);
                }
            }
            // verifica os ramos then e else
            for (size_t i = 1; i < node->children.size(); ++i) {
                checkNode(node->children[i], ctx);
            }
            break;
        }

        default:
            break;
    }
}

// Função principal para checagem semântica do programa
inline SemanticResult checkProgram(const std::shared_ptr<ASTNode>& root) {
    SemanticResult res;
    checkNode(root, res);
    return res;
}
