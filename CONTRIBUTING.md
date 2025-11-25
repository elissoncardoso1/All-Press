# Contributing to All Press C++

Obrigado por seu interesse em contribuir para o All Press C++! Este documento fornece diretrizes para contribuições.

## Como Contribuir

### Reportando Bugs

1. Verifique se o bug já foi reportado nas Issues
2. Se não, crie uma nova Issue com:
   - Descrição clara do problema
   - Passos para reproduzir
   - Comportamento esperado vs atual
   - Logs relevantes
   - Sistema operacional e versão

### Sugerindo Melhorias

1. Crie uma Issue descrevendo:
   - O problema que a melhoria resolve
   - A solução proposta
   - Alternativas consideradas
   - Impacto em funcionalidades existentes

### Pull Requests

1. **Fork o repositório**
2. **Crie uma branch** para sua feature:
   ```bash
   git checkout -b feature/minha-feature
   ```

3. **Faça suas alterações** seguindo o estilo de código:
   - Use C++17
   - Siga as convenções de nomenclatura existentes
   - Adicione comentários quando necessário
   - Mantenha funções pequenas e focadas

4. **Escreva testes** para novas funcionalidades

5. **Teste suas alterações**:
   ```bash
   ./scripts/build.sh
   cd build && ctest
   ```

6. **Commit suas alterações**:
   ```bash
   git commit -m "feat: adiciona nova funcionalidade X"
   ```
   
   Use conventional commits:
   - `feat:` nova funcionalidade
   - `fix:` correção de bug
   - `docs:` documentação
   - `refactor:` refatoração
   - `test:` testes
   - `chore:` manutenção

7. **Push para sua branch**:
   ```bash
   git push origin feature/minha-feature
   ```

8. **Abra um Pull Request**

## Estilo de Código

### C++

```cpp
// Nomenclatura
class MyClass {};              // PascalCase para classes
void my_function() {}          // snake_case para funções
int my_variable = 0;           // snake_case para variáveis
const int MAX_SIZE = 100;      // UPPER_CASE para constantes

// Formatação
if (condition) {
    // código
}

// Namespaces
namespace AllPress::Core {
    // código
}

// Includes
#include <system_headers>      // Primeiro headers do sistema
#include "project_headers.h"   // Depois headers do projeto
```

### Comentários

```cpp
/**
 * @brief Breve descrição da função
 * @param param1 Descrição do parâmetro
 * @return Descrição do retorno
 */
int my_function(int param1);

// Comentário inline para explicar código não-óbvio
int result = complex_calculation(); // Por que isso é necessário
```

## Estrutura de Testes

```cpp
#include <gtest/gtest.h>

class MyComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Configuração antes de cada teste
    }

    void TearDown() override {
        // Limpeza após cada teste
    }
};

TEST_F(MyComponentTest, TestDescription) {
    // Arrange
    // Act
    // Assert
    EXPECT_EQ(expected, actual);
}
```

## Checklist para Pull Request

- [ ] Código compila sem warnings
- [ ] Testes passam
- [ ] Novos testes adicionados para novas funcionalidades
- [ ] Documentação atualizada
- [ ] CHANGELOG.md atualizado
- [ ] Código segue o estilo do projeto
- [ ] Commits seguem conventional commits
- [ ] Branch está atualizada com main

## Revisão de Código

Todos os Pull Requests passam por revisão. Esperamos:

- Código limpo e legível
- Testes adequados
- Documentação clara
- Performance considerada
- Segurança verificada

## Processo de Release

1. Versão segue Semantic Versioning (MAJOR.MINOR.PATCH)
2. CHANGELOG.md é atualizado
3. Tag é criada
4. Release notes são publicadas

## Dúvidas?

Abra uma Issue com a label "question" ou entre em contato com os mantenedores.

## Código de Conduta

- Seja respeitoso e inclusivo
- Aceite críticas construtivas
- Foque no que é melhor para a comunidade
- Mostre empatia com outros membros

## Licença

Ao contribuir, você concorda que suas contribuições serão licenciadas sob a mesma licença MIT do projeto.

---

Obrigado por contribuir! 🚀
