# Microcompilador em C++

Projeto da disciplina de Compiladores em duas etapas:

- **GA**: analisador léxico standalone.
- **GB**: pipeline completo com léxico, parser (AST), checagem semântica e executor.

## Estrutura

```bash
lexer/       # scanner (tokenizeSource/tokenizeFile)
parser/      # parser recursivo que gera AST
semantic/    # checker de tipos/declaração
exec/        # interpretador da AST e binários gerados
main/        # CLI (--tokens/--ast/--run)
tests/       # entradas .txt + expected em tokens_out/ast_out/run_out + scripts
entregaveis/ # materiais GA/GB
```

## Build (Windows MSVC)

Use o x64 Native Tools Command Prompt ou rode `vcvars64.bat` antes.

```bash
nmake
```
Saída: `exec\microcompilador.exe` (objetos também em `exec\`).

## CLI

- `--tokens <arquivo>`: imprime tokens com linha/coluna.
- `--ast <arquivo>`: gera AST e checa semântica (declaração/uso/tipo).
- `--run <arquivo>`: executa a AST se não houver erros e imprime valores finais das variáveis.

Exemplos:

```bash
exec\microcompilador.exe --tokens tests\err_lexico.txt
exec\microcompilador.exe --ast tests\ok_basico.txt
exec\microcompilador.exe --run tests\ok_basico.txt
```

## Linguagem suportada

- Tipos: `int`, `float`, `string`, `bool`; literais `true/false`.
- Declaração opcionalmente inicializada: `int a;`, `float b = 1.5;`.
- Atribuição: `id = expr;`
- Controle: `if (expr) stmt (else stmt)?`
- Blocos: `{ stmt* }`
- Expressões com precedência: `||`, `&&`, `== !=`, `< <= > >=`, `+ -`, `* / %`, parênteses.

## Semântica

- Variável deve ser declarada antes de usar; redeclaração acusa erro.
- Compatibilidade em atribuição/inicialização (promoção `int → float` permitida; demais incompatibilidades geram erro).
- Condição do `if` deve ser `bool`; operadores aritméticos exigem numéricos.
- Erros reportados com linha/coluna/lexema.

## Executor

Interpreta a AST: mantém ambiente de variáveis, avalia expressões, atribuições e `if/else`. Imprime estado final no modo `--run`.

## Testes

- Entradas: `tests/*.txt` (ok e casos de erro léxico/sintático/semântico).
- Expected:
  - `tests\tokens_out\*.tokens.out` (modo `--tokens`)
  - `tests\ast_out\*.ast.out` (modo `--ast`)
  - `tests\run_out\*.run.out` (modo `--run`)
- Automação: `powershell -ExecutionPolicy Bypass -File tests\run_tests.ps1`
  - Roda todos os modos (`--tokens`, `--ast`, `--run`) em todos os arquivos `.txt`.
  - Compara a saída com os expected; se houver diferença, marca FAIL, grava um `.actual` ao lado do esperado e mostra um diff no resumo.
- Comandos individuais em `tests/COMANDOS.md`.

## Notas

- Escopo simples (variáveis globais); sem laços ou funções.
- Binário e objetos ficam em `exec\`; ajuste o `Makefile` se usar outro toolchain.
