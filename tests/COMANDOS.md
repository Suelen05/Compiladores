# Comandos de teste

Execute a partir da raiz do repositório, com `exec\microcompilador.exe` já compilado.

## Tokens
- `exec\microcompilador.exe --tokens tests\err_lexico.txt`
- `exec\microcompilador.exe --tokens tests\ok_basico.txt`

## AST + semântica
- `exec\microcompilador.exe --ast tests\ok_basico.txt`
- `exec\microcompilador.exe --ast tests\err_sintatico.txt`
- `exec\microcompilador.exe --ast tests\err_semantico_undeclarado.txt`
- `exec\microcompilador.exe --ast tests\err_semantico_tipo.txt`
- `exec\microcompilador.exe --ast tests\err_semantico_if.txt`

## Execução
- `exec\microcompilador.exe --run tests\ok_basico.txt`
- `exec\microcompilador.exe --run tests\err_semantico_undeclarado.txt`
- `exec\microcompilador.exe --run tests\err_semantico_tipo.txt`
- `exec\microcompilador.exe --run tests\err_semantico_if.txt`
