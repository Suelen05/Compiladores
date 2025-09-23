# 🔎 Analisador Léxico em C++

## 📌 Descrição

Este projeto é um **analisador léxico** desenvolvido em **C++** para a disciplina de Compiladores.  
O objetivo é reconhecer tokens básicos de uma mini linguagem de programação, tais como:

- **Identificadores**  
- **Palavras-chave** (`if`, `else`, `while`, `int`, `float`, `string`, `return`, etc.)  
- **Números** (inteiros e decimais)  
- **Strings literais** (com suporte a escapes e detecção de erro em strings não finalizadas)  
- **Operadores** (`+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, etc.)  
- **Pontuação** (`;`, `,`, `(`, `)`, `{`, `}`, `[`, `]`)  
- **Comentários de linha** (`// ...`)  
- **Tokens inválidos** (classificados como `UNKNOWN`)  

O analisador percorre o código-fonte de entrada e gera como saída uma lista de tokens no formato:

``` bash
TIPO -> "lexema" [linha,coluna]
```

## ⚙️ Compilação e Execução

### 1. Compilação

O projeto foi compilado utilizando o **compilador `cl` do Visual Studio** e o **nmake**:

```bash
nmake
```

Isso gera o executável `lexer.exe`.

### Execução

Para executar o analisador léxico em um arquivo de entrada (ex.: `exemplo.src`):

```bash
./lexer.exe exemplo.src
```

## 📂 Estrutura do Projeto

``` bash
.
├── entregaveis      # Pasta contendo os arquivos para a avaliação do professor
├── lexer.cpp        # Código-fonte principal do analisador
├── Makefile         # Arquivo de build (para uso com nmake)
├── exemplo.src      # Arquivo de teste com código exemplo
└── README.md        # Documentação do projeto

```

## 🧪 Exemplo de Uso

### Arquivo `exemplo.src`:

``` c
// teste simples

int main() {
    int x = 42;
    int num;
    float y = 3.14;
    string s = "Ola, mundo\n";
    if (x > 10 && num < 20) {
        x = x + 1;
    } else {
        x = x - 1;
    }
    // string sem fechar 
    "oops
    return 0;
}

```

### Saída esperada:

``` bash
COMMENTARIO -> "// teste simples" [1,1]
KEYWORD -> "int" [3,1]
IDENTIFICADOR -> "main" [3,5]
PONTUACAO -> "(" [3,9]
PONTUACAO -> ")" [3,10]
PONTUACAO -> "{" [3,12]
KEYWORD -> "int" [4,5]
IDENTIFICADOR -> "x" [4,9]
OPERADOR -> "=" [4,11]
NUMERO -> "42" [4,13]
PONTUACAO -> ";" [4,15]
...
```

## 🚀 Funcionalidades Extras

- Detecção de strings não finalizadas, retornando token `UNKNOWN`.
- Suporte a operadores compostos (`==`, `!=`, `<=`, `>=`, `&&`, `||`).
- Rastreamento de linha e coluna para cada token, facilitando a depuração.

## 👩‍💻 Autor

Trabalho desenvolvido por Suelen Fraga para a disciplina de Compiladores.
Universidade do Vale do Rio dos Sinos (UNISINOS).