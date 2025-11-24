# Comandos de teste

Execute a partir da raiz, com `exec\microcompilador.exe` já compilado.

## Tokens (`--tokens`)

- `exec\microcompilador.exe --tokens tests\ok_basico.txt`
- `exec\microcompilador.exe --tokens tests\err_lexico.txt`
- `exec\microcompilador.exe --tokens tests\err_sintatico.txt`
- `exec\microcompilador.exe --tokens tests\err_semantico_undeclarado.txt`
- `exec\microcompilador.exe --tokens tests\err_semantico_tipo.txt`
- `exec\microcompilador.exe --tokens tests\err_semantico_if.txt`

## AST + Semântica (`--ast`)

- `exec\microcompilador.exe --ast tests\ok_basico.txt`
- `exec\microcompilador.exe --ast tests\err_lexico.txt`
- `exec\microcompilador.exe --ast tests\err_sintatico.txt`
- `exec\microcompilador.exe --ast tests\err_semantico_undeclarado.txt`
- `exec\microcompilador.exe --ast tests\err_semantico_tipo.txt`
- `exec\microcompilador.exe --ast tests\err_semantico_if.txt`

## Execução (`--run`)

- `exec\microcompilador.exe --run tests\ok_basico.txt`
- `exec\microcompilador.exe --run tests\err_lexico.txt`
- `exec\microcompilador.exe --run tests\err_sintatico.txt`
- `exec\microcompilador.exe --run tests\err_semantico_undeclarado.txt`
- `exec\microcompilador.exe --run tests\err_semantico_tipo.txt`
- `exec\microcompilador.exe --run tests\err_semantico_if.txt`

## Rodar tudo de uma vez
- `powershell -ExecutionPolicy Bypass -File tests\run_tests.ps1`
