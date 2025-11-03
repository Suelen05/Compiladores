# 🗓️ Cronograma – Parte 2: Análise Sintática e Semântica  
**Período:** 03/11 a 17/11  
**Tempo diário:** até 1 hora por dia  

---

## 📅 Semana 1 – Construção do Parser (Análise Sintática)

### **03/11 – Planejamento da Gramática**
- [x] Definir a **mini linguagem** a ser suportada (declaração, atribuição, if/else, blocos).  
- [x] Escrever a **gramática base (EBNF)**.  
- [ ] Descrever isso no relatório.  

### **04/11 – Estrutura do Parser**
- [x] Escolher o tipo de parser (**recursivo descendente** em C++).  
- [x] Criar o arquivo `parser.cpp` e/ou a classe `Parser`.  
- [x] Definir as funções principais:
  - `parseProgram()`
  - `parseStatement()`
  - `parseExpression()`

### **05/11 – Declarações e Blocos**
- [x] Implementar:
  - `int x;`
  - `string s = "oi";`
  - `{ stmt* }`
- [ ] Criar função `expect(TokenType tipo)` para validar tokens e exibir erros.  

### **06/11 – Estruturas de Controle**
- [x] Implementar:
  - `if (expr) stmt (else stmt)?`
  - `ID = expr;`
- [ ] Garantir o uso correto de `;`, `(`, `)` e `{}`.  

### **07/11 – Expressões**
- [x] Implementar a hierarquia:
  - `||`, `&&`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `+`, `-`, `*`, `/`, `%`.  
- [ ] Criar funções específicas (`parseOrExpr`, `parseAndExpr`, etc.).  

### **08/11 – Testes Sintáticos**
- [ ] Rodar testes em arquivos `.src`.  
- [ ] Verificar se o parser reconhece programas válidos e acusa erros sintáticos corretamente.  

---

## 🧩 Semana 2 – Análise Semântica e Apresentação

### **09/11 – Buffer / Revisão**
- [ ] Reforçar pontos da análise sintática.  
- [ ] Testar casos limite e ajustar mensagens de erro.  

### **10/11 – Tabela de Símbolos**
- [ ] Criar uma estrutura (ex.: `unordered_map<string, string>`) para armazenar variáveis declaradas.  
- [ ] Adicionar escopo global (ou pilha de escopos).  

### **11/11 – Checagem de Declarações**
- [ ] Verificar se variáveis foram **declaradas antes de usar**.  
- [ ] Registrar erros semânticos como: *“variável não declarada”*.  

### **12/11 – Checagem de Tipos**
- [ ] Validar tipos em atribuições:
  - `int` ≠ `string`.  
- [ ] Registrar erros semânticos de incompatibilidade.  

### **13/11 – Casos de Teste**
- [ ] Criar dois arquivos:
  - `ok.src` → código válido.  
  - `erros.src` → código com falhas sintáticas e semânticas.  
- [ ] Salvar prints das saídas para a apresentação.  

---

## 🧠 Semana 3 – Documentação e Apresentação

### **14/11 – Estrutura dos Slides**
- [ ] Criar roteiro dos slides:
  1. Recap da parte léxica.  
  2. Sintaxe: fluxo geral.  
  3. Gramática usada.  
  4. Exemplo aceito.  
  5. Exemplo com erro de sintaxe.  
  6. Semântica: o que é verificado.  
  7. Dificuldades.  

### **15/11 – Relatório**
- [ ] Escrever seção:  
  - *“Análise Sintática e Semântica”*  
  - Explicar a gramática, tipo de parser e checagens semânticas.  

### **16/11 – Montagem do PPT**
- [ ] Inserir trechos de código e prints das saídas.  
- [ ] Adicionar tópicos de conclusões e trabalhos futuros.  

### **17/11 – Revisão Final e Vídeo**
- [ ] Testar o código completo.  
- [ ] Ensaiar e/ou gravar o vídeo da parte 2.  
- [ ] Fazer revisão final do relatório e slides.  

---

## 🧾 Observações
- Cada atividade toma **no máximo 1h por dia**.  
- As tarefas foram organizadas para permitir adiantar etapas se sobrar tempo.  
- O foco da segunda parte é mostrar que o compilador agora:
  1. **Compreende a estrutura (sintaxe)** do código.  
  2. **Reconhece erros e tipos (semântica)** de forma coerente.  
