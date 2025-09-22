# Compiladores

## Objetivo Geral

O objetivo desta atividade é compreender, implementar e testar um analisador léxico utilizando qualquer linguagem ou ferramenta escolhida pelo aluno (por exemplo: C, C++, Java, Python, JFlex, Flex, entre outras). A ideia é que os estudantes consigam construir um programa capaz de ler um código-fonte simples e reconhecer seus tokens, preparando o caminho para as fases seguintes da compilação: análise sintática e análise semântica.

A atividade não é apenas técnica: deve-se também explicar o processo de desenvolvimento. Isso significa apresentar em sala de aula, por meio de slides, trechos de código e um vídeo curto de execução, como resolveu cada etapa e quais foram as dificuldades encontradas.

## Competências Desenvolvidas

Identificar e classificar símbolos léxicos em programas. ✔️

Utilizar expressões regulares ou regras definidas manualmente para descrever padrões léxicos.

Implementar um analisador léxico funcional com suporte a identificadores ✔️, números ✔️, operadores ✔️ e strings ✔️.

Explicar decisões técnicas e dificuldades encontradas no processo de implementação.

Desenvolver habilidades de comunicação.

Os alunos devem construir um analisador léxico simples para uma mini linguagem de programação. Esse analisador deve reconhecer os seguintes elementos básicos:

- **Identificadores:** nomes de variáveis ou funções ✔️.
- **Números:** inteiros ou decimais ✔️.
- **Palavras-chave:** como `if`, `else`, `while` ✔️.
- **Operadores:** como `+`, `-`, `*`, `/`, `=` ✔️.
- **Strings literais:** textos entre aspas, como "Olá mundo" ✔️.

A implementação será dividida em cinco etapas. O objetivo é não apenas programar, mas também explicar o raciocínio utilizado, documentando o processo em slides e vídeo.

Durante a apresentação em aula:

- Os slides devem trazer explicações, exemplos de código e testes.
- O vídeo deve mostrar o programa rodando na máquina e ser comentado pelos alunos.

# Etapas da Atividade

## Etapa 1 – Identificação de Tokens com Expressões Regulares

Criar uma tabela com exemplos de cadeias válidas e inválidas para cada tipo de token.

Escrever expressões regulares que descrevem esses padrões.

Testar manualmente quais cadeias são aceitas ou rejeitadas.

### Exemplo

Expressão regular para identificadores em Java:

``` java

// identificador: letra ou _ seguido de letras/dígitos/_
[a-zA-Z_][a-zA-Z0-9_]*

```

- Cadeias válidas: `var1`,`_contador`, `x`.
- Cadeias inválidas: `1abc`, `@nome`.

## Etapa 2 – Scanner Simples com Retorno de Inteiros

Implementar um scanner inicial que leia um arquivo de entrada e retorne números inteiros representando cada token reconhecido.

Testar com exemplos curtos de código-fonte.

**Exemplo em Python (simples):**

``` python

# Código Python
# Analisador Léxico com tokens: NUM, ID, OP
import re

tokens = [
    ("NUM", r"\d+"),
    ("ID", r"[a-zA-Z_][a-zA-Z0-9_]*"),
    ("OP", r"[+\-*/=]"),
]

codigo = "x = 42 + y"

for tipo, padrao in tokens:
    for m in re.finditer(padrao, codigo):
        print(tipo, "->", m.group())

```

``` bash

# Saída do Programa
ID   -> x
OP   -> =
NUM  -> 42
OP   -> +
ID   -> y

```

## Etapa 3 – Classe Token com Informações Detalhadas

Criar uma classe (ou estrutura de dados) `Token` que armazene **tipo**, **valor**, **linha** e **coluna**.

Adaptar o scanner para retornar objetos `Token`.

Exemplo de classe `Token` em Java:

``` java

// Classe Token em Java
public class Token {
    String tipo;
    String valor;
    int linha;
    int coluna;

    public Token(String tipo, String valor, int linha, int coluna) {
        this.tipo = tipo;
        this.valor = valor;
        this.linha = linha;
        this.coluna = coluna;
    }

    public String toString() {
        return tipo + " (" + valor + ") [" + linha + "," + coluna + "]";
    }
}

```

## Etapa 4 – Uso de Macros e Modularização

Refatorar o código para usar macros ou funções reutilizáveis.

Garantir que expressões regulares não se repitam, facilitando manutenção.

**Exemplo em JFlex (macro):**

``` java

// Expressões Regulares
ID = [a-zA-Z_][a-zA-Z0-9_]*
NUM = [0-9]+

```

## Etapa 5 – Reconhecimento de Strings Literais com Estados

Adicionar suporte a **strings literais** e implementar **estados** para tratar cadeias abertas sem fechamento.

Detectar erros como “Olá mundo (string sem aspas finais).

**Exemplo de regra em JFlex:**

``` java

// Regras do analisador léxico (JFlex)
<STR> \" { yybegin(YYINITIAL); return new Token("STRING", yytext()); }
<STR> [^\"\n]+
<YYINITIAL> \" { yybegin(STR); }

```

# Entregáveis

- **PPT da apresentação** com explicações, trechos de código e resultados.
- **Vídeo explicativo** (2–5 minutos) mostrando o programa rodando e narrando o processo.
- **Código-fonte completo** na linguagem escolhida.
- **Arquivos de entrada de teste**.
- **Relatório simples** com:
  - Tabela de cadeias (Etapa 1).
  - Prints das saídas dos testes.
  - Diferença entre reconhecer if como palavra-chave ou identificador.
  - Comentários sobre modularização e estados.

---

# Task List

[ ] Etapa 1 – Identificação de Tokens com Expressões Regulares

[ ] Etapa 2 – Scanner Simples com Retorno de Inteiros

[ ] Etapa 3 – Classe Token com Informações Detalhadas

[ ] Etapa 4 – Uso de Macros e Modularização

[ ] Etapa 5 – Reconhecimento de Strings Literais com Estados
