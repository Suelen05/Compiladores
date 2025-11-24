# Microcompilador em C++

Projeto da disciplina de Compiladores dividido em duas etapas:

- **GA**: analisador léxico.
- **GB**: pipeline completo com léxico, parser (AST), checagem semântica básica e executor simples.

## Estrutura do projeto

``` bash
.
├── lexer/         # Lexer (tokenizeSource/tokenizeFile)
├── parser/        # Parser recursivo que gera AST
├── semantic/      # Checker de tipos/declaração sobre a AST
├── exec/          # Executor da AST (interpretação) e binários gerados
├── main/          # CLI (microcompilador.exe) com modos --tokens/--ast/--run
├── tests/         # Casos de teste e .out esperados
└── entregaveis/   # Materiais das entregas GA e GB
```

## Como compilar (Windows, MSVC)

Requer Visual Studio Build Tools e `nmake`. Use o **x64 Native Tools Command Prompt** ou rode `vcvars64.bat` antes.

```bash
nmake
```

Saída: `exec\microcompilador.exe` e objetos em `exec\`.

## Como usar

```bash
# Listar tokens
exec\microcompilador.exe --tokens caminho\arquivo.txt

# Gerar AST e checar semântica (erros de variável/ti pos)
exec\microcompilador.exe --ast caminho\arquivo.txt

# Executar (só roda se semântica passar) e imprime valores finais
exec\microcompilador.exe --run caminho\arquivo.txt
```

### Exemplos rápidos

```bash
exec\microcompilador.exe --tokens tests\err_lexico.txt
exec\microcompilador.exe --ast tests\ok_basico.txt
exec\microcompilador.exe --run tests\ok_basico.txt
```

## Tipos e sintaxe suportados (GB)

- Declarações: `int`, `float`, `string`, `bool` com inicialização opcional: `int a;`, `float b = 1.5;`
- Atribuições: `ident = expr;`
- Controle: `if (expr) stmt (else stmt)?`
- Blocos: `{ stmt* }`
- Expressões: `|| && == != < > <= >= + - * / %` com precedência e parênteses.
- Literais: inteiros, reais, strings, `true`/`false`.

## Checagem semântica

- Variável deve ser declarada antes de usar.
- Redeclaração acusa erro.
- Compatibilidade em atribuição e inicialização (promove `int` → `float`; demais incompatibilidades geram erro).
- Condição do `if` deve ser `bool`.
- Erros emitidos com linha e coluna.

## Executor

Interpreta a AST em memória:

- Mantém um ambiente de variáveis (tipos e valores).
- Suporta aritmética, comparação, lógica, atribuições e `if/else`.
- Imprime o estado final das variáveis no `--run`.

## Testes

Casos em `tests/` com entradas e saídas esperadas (.out):

- `ok_basico`: fluxo válido com `if/else`.
- Erros léxico, sintático e semânticos (`undeclarado`, `tipo`, `if` não bool).
Rode os modos `--tokens`, `--ast` ou `--run` apontando para esses arquivos para validar.

## Entregas (GA e GB)

- **GA**: entrega do analisador léxico (já incorporado em `lexer/`), com exemplos e tokens.
- **GB**: pipeline completo (lexer + parser + semântico + executor), organização em pastas, testes e binário `microcompilador.exe`.
