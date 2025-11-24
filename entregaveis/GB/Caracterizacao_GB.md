# Compiladores

## Atividade Prática, Micro-compilador

Objetivo, implementar um compilador educacional mínimo, fases léxica, sintática, semântica e execução. Ênfase, pipeline completo, mensagens claras, testes reproduzíveis.

> **Escopo Mínimo**, palavra-chave, `inicio`, `fim`, `inteiro`, `real`, `imprima`. Tokens, IDENT, NUM_INT, NUM_REAL, OP + − * /, ATRIB :=, DELIM ; ( ), . Precedência, * e / acima de + e −, parênteses alteram ordem. Saída, execução da AST ou código intermediário simples.

## Entregas obrigatórias

1. **Código-fonte** com README de execução sem IDE. Incluir comandos para gerar lexer e parser.
2. **Especificação técnica** curta em PDF, tokens, gramática EBNF, precedência, regras semânticas, política de tipos.
3. **Pasta de testes** com 6 casos no mínimo, entradas .txt e saídas esperadas .out.
4. **Relatório em formato de apresentação**, 8 a 12 slides, roteiro, com demonstração ao vivo.

## Organização do projeto

- **lexer/**, JFlex, tokens com linha e coluna.
- **parser/**, CUP ou ANTLR, construção da AST.
- **semantic/**, tabela de símbolos, verificação de tipos.
- **exec/**, interpretador da AST, execução.
- **main/, CLI**, modos, `--tokens`, `--ast`, `--run`.
- **tests/**, arquivos de teste e saídas esperadas.

### Exemplos de CLI

``` bash
java -jar microc.jar --tokens exemplos/soma.txt
java -jar microc.jar --ast exemplos/soma.txt
java -jar microc.jar --run exemplos/soma.txt
```

## Roteiro de slides do relatório

1. Título e equipe, visão geral da linguagem.
2. Léxico, tabela de tokens, exemplo com linha e coluna.
3. Sintaxe, recorte da gramática, precedência e associatividade.
4. AST, nós principais e um exemplo desenhado.
5. Semântica, regras, mensagens de erro com lexema.
6. Execução, percurso da AST ou código intermediário.
7. Arquitetura do projeto, pastas e classes.
8. Testes, tabela com entradas e saídas esperadas.
9. Demonstração, um caso válido e um caso de erro.
10. Limitações e próximos passos, lições aprendidas.

## Formato da entrega no Moodle

- Arquivo único .zip, padrão de nome, **microc_nome-sobrenome.zip** ou **microc_dupla-nome1-nome2.zip**.
- Conteúdo do ZIP, pasta do projeto, PDF da especificação, pasta testes, slides em PDF

## Apresentação em sala

Tempo, 10 minutos, apresentação 7 minutos, perguntas 3 minutos. Demonstração obrigatória, executar 1 caso que passa e 1 caso de erro com mensagem legível.

## Rubrica resumida, 10,0 pontos

Critério | Descrição | Peso
---------|-----------| -----
Léxico | Tokens corretos, linha e coluna, erro léxico tratado. | 2,0
Sintaxe e AST | Gramática funcional, precedência correta, AST clara. | 2,5
Semântica | Declaração e uso, tipos, coerções, mensagens objetivas. | 2,5
Execução e Testes | Interpretação da AST ou código intermediário, 6 testes. | 2,0
Organização e Relato | Estrutura, README, slides, clareza | 1,0

## Checklist rápido

- [ ] Compila e roda sem IDE, instruções no README.
- [x] `--tokens`, `--ast`, `--run` funcionando.
- [x] Mensagens de erro com linha, coluna e lexema.
- [x] Seis testes mínimos, incluindo um caso de erro léxico, sintático e semântico.
- [ ] Slides em PDF.
  