# 🗓️ Cronograma – Parte 2: Análise Sintática e Semântica

**Período:** 14/11 a 24/11  
**Tempo diário:** até 1 hora por dia  

---

## 📅 Semana 1 – Implementação da Análise Sintática

### **14/11 – Revisão da Gramática**

- [x] Relembrar a gramática definida na parte 1.  
- [x] Ajustar ou simplificar regras se necessário (declaração, atribuição, if/else, blocos).  
- [x] Criar arquivo base `parser.cpp` e preparar leitura de tokens vindos do léxico.  

### **15/11 – Estrutura do Parser**

- [x] Implementar a classe `Parser` e funções iniciais:  
  - `parseProgram()`  
  - `parseStatement()`  
  - `parseExpression()`  
- [x] Garantir que o parser percorra a lista de tokens gerada pelo lexer.  

### **16/11 – Declarações e Blocos**

- [ ] Implementar as regras de:  
  - Declaração: `int x;`, `string s = "oi";`  
  - Bloco: `{ stmt* }`  
- [ ] Adicionar tratamento de erros com mensagens claras (ex.: “esperado ;”).  

### **17/11 – Estruturas de Controle**

- [x] Implementar `if` e `else`:  
  - `if (expr) stmt (else stmt)?`  
- [ ] Testar exemplos simples e verificar o aninhamento de blocos.  

### **18/11 – Expressões**

- [x] Implementar parsing de expressões com precedência:  
  - `||`, `&&`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `+`, `-`, `*`, `/`, `%`.  
- [x] Criar funções auxiliares (`parseOrExpr`, `parseAndExpr`, `parseRelExpr`, etc.).  

### **19/11 – Testes Sintáticos**

- [ ] Criar arquivos `.src` de teste com casos válidos e inválidos.  
- [ ] Verificar se o parser reconhece a estrutura correta e acusa erros sintáticos.  

---

## 📘 Semana 2 – Análise Semântica e Validação

### **20/11 – Implementar Tabela de Símbolos**

- [ ] Criar uma tabela (`unordered_map<string, string>`) para armazenar variáveis e tipos.  
- [ ] Ao encontrar uma declaração (`int x;`), registrar a variável e seu tipo.  

### **21/11 – Checagem de Declarações**

- [ ] Validar se variáveis foram **declaradas antes de usar**.  
- [ ] Gerar mensagem de erro: “variável não declarada”.  

### **22/11 – Checagem de Tipos**

- [ ] Validar compatibilidade de tipos em atribuições:
  - `int` ≠ `string`.  
- [ ] Registrar erros semânticos como “tipos incompatíveis em atribuição”.  

### **23/11 – Testes de Semântica**

- [ ] Criar dois arquivos:  
  - `ok.src` → código válido.  
  - `erros.src` → exemplos com erros de sintaxe e semântica.  
- [ ] Verificar se as mensagens de erro são claras.  

---

## 🎤 Semana 3 – Apresentação e Finalização

### **24/11 – Montagem Final e Revisão**

- [ ] Montar slides da **Parte 2** com:  
  1. Recapitulando o léxico.  
  2. Sintaxe – gramática e fluxo de análise.  
  3. Exemplo aceito pelo parser.  
  4. Exemplo com erro sintático.  
  5. Semântica – tabela de símbolos e verificação de tipos.  
  6. Dificuldades e aprendizados.  
- [ ] Revisar o relatório.  
- [ ] Ensaiar ou gravar o vídeo de apresentação.  

---

## 🧾 Observações

- Cada dia equivale a uma etapa curta de até **1 hora**.  
- O cronograma deixa **tempo de sobra** para revisar e testar antes da entrega.  
- O foco é mostrar a **evolução natural do compilador**:
  1. **Léxico** → identifica os tokens.  
  2. **Sintático** → valida a estrutura.  
  3. **Semântico** → checa o sentido e a coerência.  
