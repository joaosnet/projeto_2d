# 🚀 COSMIC DRIFT - Projeto OpenGL 2D

Um ambiente 2D desenvolvido em OpenGL que demonstra **modelagem geométrica** e **transformações geométricas** através de uma experiência interativa.

## 📋 Descrição do Projeto

**Cosmic Drift** é um projeto que implementa um ambiente espacial interativo em OpenGL. O objetivo principal é demonstrar a modelagem de objetos geométricos 2D e a aplicação de transformações geométricas (translação, rotação e escala) em um contexto prático e visual.

### 🔍 Principais Componentes:
- **Modelagem Geométrica:** Implementação de diferentes primitivas 2D
- **Transformações Geométricas:** Aplicação de translação, rotação e escala
- **Sistema de Coordenadas:** Utilização de coordenadas homogêneas
- **Pipeline OpenGL:** Uso de shaders personalizados

## 🔷 Modelagem Geométrica Implementada

### 1. **Nave Espacial (Triângulo Estilizado)**
```cpp
// Geometria: Triângulo estilizado representando uma nave
GameObject createPlayerShip() {
    ship.vertices = {
         0.0f,  0.08f, 0.0f,  // ponta da nave
        -0.05f, -0.05f, 0.0f,  // asa esquerda
         0.05f, -0.05f, 0.0f,  // asa direita
         0.0f, -0.02f, 0.0f   // motor central
    };
    ship.indices = {0, 1, 3, 0, 3, 2}; // 2 triângulos
}
```
- **Geometria:** 4 vértices em coordenadas cartesianas normalizadas
- **Topologia:** 2 triângulos conectados usando índices
- **Representação:** Coordenadas (x, y, z) definindo pontos no espaço 2D

### 2. **Asteroides (Hexágonos Irregulares)**
```cpp
// Geração procedural de asteroides irregulares
GameObject createAsteroid() {
    const float baseRadius = 0.08f + randomFloat(gen) * 0.05f;
    const int sides = 6;
    
    // Centro do asteroide
    asteroid.vertices.push_back(0.0f);
    asteroid.vertices.push_back(0.0f);
    asteroid.vertices.push_back(0.0f);
    
    // Vértices irregulares do asteroide
    for(int i = 0; i < sides; i++) {
        float angle = 2.0f * M_PI * i / sides;
        float radiusVariation = baseRadius * (0.7f + randomFloat(gen) * 0.6f);
        asteroid.vertices.push_back(radiusVariation * cos(angle));
        asteroid.vertices.push_back(radiusVariation * sin(angle));
        asteroid.vertices.push_back(0.0f);
    }
}
```
- **Geometria:** 7 vértices (1 centro + 6 perímetro)
- **Topologia:** 6 triângulos radiais formando uma figura única
- **Variação:** Forma gerada proceduralmente com perturbação aleatória
- **Transformações:** Rotação contínua, translação linear
- **Variação:** Cada asteroide tem forma única gerada aleatoriamente

### 3. **Inimigos (Quadrados)**
```cpp
// Quadrados representando naves inimigas
GameObject createEnemy() {
    enemy.vertices = {
        -0.06f, -0.06f, 0.0f,  // inferior esquerdo
         0.06f, -0.06f, 0.0f,  // inferior direito
         0.06f,  0.06f, 0.0f,  // superior direito
        -0.06f,  0.06f, 0.0f   // superior esquerdo
    };
    enemy.indices = {0, 1, 2, 0, 2, 3}; // 2 triângulos
}
```
- **Geometria:** 4 vértices formando um quadrado regular
- **Topologia:** 2 triângulos usando strip de índices
- **Cor:** Vermelho ameaçador
- **Transformações:** IA com perseguição usando translação direcionada
- **Comportamento:** Movimento adaptativo em direção ao jogador

### 4. **Power-ups (Losangos)**
```cpp
// Losangos para itens especiais
GameObject createPowerUp() {
    powerup.vertices = {
         0.0f,  0.05f, 0.0f,  // topo
        -0.03f,  0.0f, 0.0f,  // esquerda
         0.0f, -0.05f, 0.0f,  // baixo
         0.03f,  0.0f, 0.0f   // direita
    };
    powerup.indices = {0, 1, 3, 1, 2, 3}; // 2 triângulos
}
```
- **Geometria:** 4 vértices em forma de diamante
- **Topologia:** 2 triângulos conectados
- **Cores:** Amarelo (Tiro Rápido) / Ciano (Escudo)
- **Transformações:** Escala pulsante para destaque visual
- **Efeitos:** Animação de pulsação contínua

### 5. **Projéteis (Quadrados Pequenos)**
- **Geometria:** Pequenos quadrados para precisão
- **Cor:** Branco brilhante
- **Transformações:** Translação de alta velocidade
- **Física:** Herdam velocidade parcial da nave

### 6. **Partículas de Explosão**
- **Geometria:** Micro-quadrados
- **Cores:** Correspondem ao objeto destruído
- **Transformações:** Dispersão radial com escala decrescente
- **Efeitos:** Fade-out com transparência

### 7. **Estrelas de Fundo**
- **Geometria:** Pontos variados
- **Cores:** Branco com intensidade variável
- **Transformações:** Pulsação sutil para simular cintilação
- **Ambiente:** 100 estrelas distribuídas aleatoriamente

## 🎮 Controles do Jogo

### Comandos de Movimento:
- **W / ↑:** Propulsão da nave (aplicação de força)
- **A / ← e D / →:** Rotação da nave (esquerda/direita)
- **SPACE:** Disparar projéteis

### Comandos do Sistema:
- **P:** Pausar/Despausar jogo
- **R:** Reiniciar jogo (apenas quando Game Over)
- **ESC:** Sair do programa

### Mecânicas de Jogo:
- **Física Realista:** A nave possui inércia e momentum
- **Sistema de Vidas:** 3 vidas iniciais
- **Pontuação:** 10 pts (asteroides), 25 pts (inimigos), 50 pts (power-ups)
- **Níveis:** Dificuldade aumenta automaticamente a cada 30 segundos
- **Power-ups Temporários:** 5 segundos de duração cada

### Power-ups Disponíveis:
- **🟡 Amarelo:** Tiro Rápido - Reduz cooldown dos disparos
- **🔵 Ciano:** Escudo - Proteção temporária contra colisões

## 🚀 Como Executar

### Compilação e Execução Rápida (VS Code):
1. Abra o projeto no VS Code
2. Pressione `Ctrl + Shift + P`
3. Digite "Tasks: Run Task"
4. Selecione "**build and run**"

### Via Terminal (PowerShell):
```powershell
# Compilar
g++ main.cpp dependencies\lib\glad.c -o cosmic_drift.exe -Idependencies\include -Ldependencies\lib -lglfw3 -lopengl32 -lgdi32 -std=c++17

# Executar
.\cosmic_drift.exe
```

## 🌟 Transformações Geométricas Demonstradas

### 1. **Translação (T)**
- **Aplicação:** Movimento de todos os objetos
- **Matriz:** T(tx, ty) para deslocamento no plano
- **Exemplos:**
  - Nave: Movimento baseado em força e inércia
  - Asteroides: Movimento linear constante
  - Projéteis: Trajetória balística
  - Partículas: Dispersão radial de explosões

### 2. **Rotação (R)**
- **Aplicação:** Orientação e direcionamento
- **Matriz:** R(θ) para rotação ao redor da origem
- **Exemplos:**
  - Nave: Rotação controlada pelo jogador
  - Asteroides: Rotação contínua e aleatória
  - Projéteis: Orientação baseada na trajetória

### 3. **Escala (S)**
- **Aplicação:** Animações e efeitos visuais
- **Matriz:** S(sx, sy) para redimensionamento
- **Exemplos:**
  - Power-ups: Pulsação rítmica para destaque
  - Partículas: Encolhimento durante fade-out
  - Escudo: Expansão/contração para efeito visual

### 4. **Composição de Transformações**
- **Ordem:** T × R × S (Translação × Rotação × Escala)
- **Coordenadas Homogêneas:** Sistema 4x4 para transformações 2D
- **Pipeline:** Aplicação eficiente via shaders GPU

## 🔧 Implementação das Transformações Geométricas

### 1. **Sistema de Coordenadas Homogêneas**
```cpp
// Matrizes 4x4 para transformações 2D
float transform[16] = {0};
float scaleMatrix[16] = {0};
float rotMatrix[16] = {0};
float transMatrix[16] = {0};
```
- Utilização de matrizes 4x4 para transformações 2D
- Representação em coordenadas homogêneas (x, y, z, w)
- Facilita composição sequencial de múltiplas transformações

### 2. **Translação (T)**
```cpp
// Matriz de Translação T(tx, ty)
transMatrix[0] = 1.0f; transMatrix[5] = 1.0f; 
transMatrix[10] = 1.0f; transMatrix[15] = 1.0f;
transMatrix[12] = obj.posX; transMatrix[13] = obj.posY;
```
- **Matriz T(tx, ty):**
  ```
  [1  0  0  tx]
  [0  1  0  ty]
  [0  0  1  0 ]
  [0  0  0  1 ]
  ```
- **Aplicações Implementadas:**
  - Movimento de objetos no plano 2D
  - Posicionamento dinâmico baseado em velocidade
  - Wraparound nas bordas da tela

### 3. **Rotação (R)**
```cpp
// Matriz de Rotação R(θ)
float cosR = cos(obj.rotation);
float sinR = sin(obj.rotation);
rotMatrix[0] = cosR; rotMatrix[1] = sinR;
rotMatrix[4] = -sinR; rotMatrix[5] = cosR;
rotMatrix[10] = 1.0f; rotMatrix[15] = 1.0f;
```
- **Matriz R(θ):**
  ```
  [cos(θ)  sin(θ)  0  0]
  [-sin(θ) cos(θ)  0  0]
  [0       0       1  0]
  [0       0       0  1]
  ```
- **Aplicações Implementadas:**
  - Rotação da nave controlada pelo usuário
  - Rotação contínua de asteroides
  - Orientação de objetos durante movimento

### 4. **Escala (S)**
```cpp
// Matriz de Escala S(sx, sy)
scaleMatrix[0] = obj.scaleX; scaleMatrix[5] = obj.scaleY; 
scaleMatrix[10] = 1.0f; scaleMatrix[15] = 1.0f;

// Animação de pulsação para power-ups
obj.scaleX = 1.0f + 0.2f * sin(gameState.timeElapsed * obj.pulseSpeed);
obj.scaleY = obj.scaleX;
```
- **Matriz S(sx, sy):**
  ```
  [sx  0   0  0]
  [0   sy  0  0]
  [0   0   1  0]
  [0   0   0  1]
  ```
- **Aplicações Implementadas:**
  - Efeitos de pulsação para power-ups
  - Variação de tamanho de objetos
  - Animação de partículas (expansão/contração)

### 5. **Composição de Transformações**
```cpp
// Ordem: T × R × S (Translação × Rotação × Escala)
// 1. R × S
for(int row = 0; row < 4; row++) {
    for(int col = 0; col < 4; col++) {
        for(int k = 0; k < 4; k++) {
            tempMatrix[row * 4 + col] += rotMatrix[row * 4 + k] * scaleMatrix[k * 4 + col];
        }
    }
}

// 2. T × (R × S)
for(int row = 0; row < 4; row++) {
    for(int col = 0; col < 4; col++) {
        transform[row * 4 + col] = 0;
        for(int k = 0; k < 4; k++) {
            transform[row * 4 + col] += transMatrix[row * 4 + k] * tempMatrix[k * 4 + col];
        }
    }
}
```
- Implementação manual da multiplicação de matrizes
- Sequência de aplicação: primeiro escala, depois rotação, por fim translação
- Armazenamento column-major para compatibilidade com OpenGL

## 🎯 Algoritmos e Sistemas Implementados

### Sistema de Física:
- **Integração Temporal:** Método de Euler para movimento
- **Detecção de Colisão:** Círculos boundários otimizados
- **Wrap-around:** Objetos atravessam bordas da tela
- **Atrito Diferenciado:** Cada tipo de objeto possui fricção específica

### Sistema de Spawning:
- **Asteroides:** Geração procedural nas bordas da tela
- **Inimigos:** Spawning adaptativo longe do jogador
- **Power-ups:** Aparição aleatória em posições seguras

### Inteligência Artificial:
- **Inimigos:** Perseguição simples com vetor direcionado
- **Variação:** Comportamento randomizado para imprevisibilidade

### Sistema de Partículas:
- **Explosões:** 8-12 partículas por evento
- **Dispersão:** Vetores radiais aleatórios
- **Lifecycle:** Fade-out progressivo com transparência

## 🔧 Características Técnicas

### Renderização OpenGL:
- **Vertex Shaders:** Aplicação de transformações matriciais
- **Fragment Shaders:** Colorização e efeitos de transparência
- **Blending:** Alpha blending para partículas e efeitos
- **VAO/VBO/EBO:** Geometria otimizada em GPU

### Performance:
- **60 FPS Target:** Loop de jogo otimizado
- **Delta Time:** Movimento independente de framerate
- **Cleanup Automático:** Gerenciamento de memória eficiente
- **Culling:** Objetos inativos são removidos automaticamente

## 🎨 Inovações Visuais

### Efeitos de Partículas Avançados:
- **💫 Propulsão:** Trail de partículas azuis quando a nave acelera
- **💥 Explosões:** Dispersão radial com cores correspondentes aos objetos
- **🌀 Wraparound:** Efeitos visuais quando objetos atravessam bordas
- **⚡ Impacto:** Feedback visual instantâneo em colisões

### Sistema de Transparência:
- **Alpha Blending:** Partículas com fade-out suave
- **Escudo Pulsante:** Efeito visual ciano semi-transparente
- **Estrelas Cintilantes:** Variação de brilho atmosférica

### Interface Dinâmica:
- **Console Atualizado:** Informações em tempo real
- **Feedback Visual:** Cores indicam estados (power-ups, escudo)
- **Animações Fluidas:** 60 FPS com interpolação suave

## 🏆 Requisitos Técnicos Atendidos

### ✅ Modelagem Geométrica:
- **Geometria 2D:** 7 tipos diferentes de objetos implementados
- **Topologia Variada:** Triângulos, quadrados, hexágonos irregulares
- **Representação Eficiente:** VAO/VBO/EBO com índices otimizados

### ✅ Transformações Geométricas:
- **Translação:** Movimento com física realista e inércia
- **Rotação:** Controle direcional e rotação automática
- **Escala:** Animações de pulsação e efeitos visuais
- **Composição:** Matrizes 4x4 aplicadas via shaders

### ✅ Ambiente OpenGL:
- **Pipeline Moderno:** OpenGL 3.3 Core Profile
- **Shaders Personalizados:** Vertex e Fragment shaders
- **Gestão de Recursos:** Buffers automáticos e cleanup
- **Performance Otimizada:** Renderização eficiente em GPU

## 🎮 Experiência de Jogo

### Progressão e Desafio:
- **Dificuldade Adaptativa:** Aumenta automaticamente
- **Sistema de Pontuação:** Múltiplos objetivos de scoring
- **Power-ups Estratégicos:** Timing e posicionamento críticos
- **Física Realista:** Momentum e inércia para precisão

### Jogabilidade Viciante:
- **Controles Responsivos:** Input imediato e preciso
- **Feedback Constante:** Visual e sonoro (partículas)
- **Desafio Crescente:** Curva de aprendizado equilibrada
- **Rejogabilidade:** Cada partida é única e dinâmica

## 📚 Aplicação Educacional

Este projeto demonstra conceitos fundamentais de:

### Computação Gráfica:
- **Pipeline de Renderização:** Vertex → Primitive Assembly → Fragment
- **Transformações Afins:** Aplicação prática em tempo real
- **Sistemas de Coordenadas:** Homogêneas e cartesianas
- **Geometria Computacional:** Detecção de colisão e física

### Programação de Jogos:
- **Game Loop:** Estrutura clássica Input → Update → Render
- **Estado de Jogo:** Gerenciamento centralizado
- **Sistemas de Partículas:** Efeitos visuais procedurais
- **IA Básica:** Comportamento de perseguição

### Matemática Aplicada:
- **Álgebra Linear:** Multiplicação de matrizes 4x4
- **Trigonometria:** Movimento circular e direcionamento
- **Física:** Integração numérica e cinemática
- **Probabilidade:** Geração procedural e randomização

---

## 🚀 Conclusão

**Cosmic Drift** representa uma implementação completa e elegante dos conceitos de transformações geométricas em OpenGL, combinando:

✨ **Técnica Sólida:** Demonstração rigorosa de conceitos fundamentais  
🎮 **Experiência Envolvente:** Jogo viciante e divertido  
🎨 **Qualidade Visual:** Efeitos modernos e interface polida  
📖 **Valor Educacional:** Código bem documentado e estruturado  

O projeto excede os requisitos básicos ao implementar um ambiente completo de jogo que serve tanto como demonstração técnica quanto como experiência interativa memorável.

**Transforme geometria em diversão! 🌟**
```powershell
# Compilar
g++ main.cpp dependencies\lib\glad.c -o app.exe -Idependencies\include -Ldependencies\lib -lglfw3 -lopengl32 -lgdi32 -std=c++17

# Executar  
.\app.exe
```

## 📋 Estrutura do Projeto

```
COSMIC_DRIFT/
│
├── main.cpp                 # Código fonte principal
├── cosmic_drift.exe         # Executável compilado
├── README.md                # Este documento
│
└── dependencies/            # Bibliotecas externas
    ├── include/             # Arquivos de cabeçalho
    │   ├── glad/           
    │   ├── GLFW/           
    │   └── KHR/            
    │
    └── lib/                 # Bibliotecas compiladas
        ├── glad.c
        ├── glfw3.dll
        ├── libglfw3.a
        └── libglfw3dll.a
```

## 📚 Conceitos de Computação Gráfica Demonstrados

### 1. **Sistemas de Coordenadas:**
- Coordenadas cartesianas para representação de vértices
- Coordenadas homogêneas para transformações
- Normalização para manter consistência de tamanho

### 2. **Representação Geométrica:**
- Vértices e faces (índices) para definir objetos
- Triangulação para renderização eficiente
- Topologias variadas para diferentes formas

### 3. **Transformações Afins:**
- Implementação matemática das matrizes fundamentais
- Aplicação prática em objetos em movimento
- Composição de múltiplas transformações

### 4. **Pipeline Gráfico:**
- Vertex processing: transformação de coordenadas
- Rasterização: conversão de primitivas em fragmentos
- Fragment processing: colorização e efeitos visuais

## 🏆 Conclusão

O projeto **Cosmic Drift** implementa com sucesso os conceitos fundamentais de modelagem geométrica e transformações geométricas usando OpenGL. Através da criação de diferentes objetos 2D e da aplicação de translação, rotação e escala, o projeto demonstra de forma prática os fundamentos teóricos da computação gráfica.

O sistema desenvolvido permite visualizar claramente como as transformações afetam os objetos em tempo real, proporcionando uma compreensão intuitiva dos conceitos matemáticos envolvidos. A implementação das matrizes de transformação e sua aplicação via shaders demonstra o fluxo completo do pipeline gráfico OpenGL.

Este projeto serve como uma base sólida para explorar conceitos mais avançados de computação gráfica, como iluminação, texturização e técnicas de animação.

---

## 📋 Estrutura do Projeto

```
COSMIC_DRIFT/
│
├── main.cpp                 # Código fonte principal
├── cosmic_drift.exe         # Executável compilado
├── README.md                # Este documento
│
└── dependencies/            # Bibliotecas externas
    ├── include/             # Arquivos de cabeçalho
    │   ├── glad/           
    │   ├── GLFW/           
    │   └── KHR/            
    │
    └── lib/                 # Bibliotecas compiladas
        ├── glad.c
        ├── glfw3.dll
        ├── libglfw3.a
        └── libglfw3dll.a
```

## 🔧 Conceitos Técnicos Implementados

### Modelagem Geométrica:
- **Primitivas 2D:** Diferentes geometrias criadas programaticamente
- **Representação:** Vértices em coordenadas cartesianas + índices
- **Buffers OpenGL:** VAO, VBO e EBO para cada objeto

### Transformações Afins:
- **Translação:** Matriz T(tx, ty) para movimentação
- **Rotação:** Matriz R(θ) para rotação ao redor da origem  
- **Escala:** Matriz S(sx, sy) para redimensionamento
- **Composição:** T × R × S aplicada via multiplicação de matrizes

### Pipeline Gráfico:
- **Vertex Shader:** Aplica transformações nos vértices
- **Fragment Shader:** Define cores dos pixels
- **Uniforms:** Comunicação CPU-GPU para matrizes e cores
- **Coordenadas Homogêneas:** Sistema 4x4 para transformações 2D

## 🛠️ Configuração do Ambiente

### Dependências Necessárias:
- **GLFW 3.3+:** Gerenciamento de janelas e entrada
- **GLAD:** Carregador de funções OpenGL 3.3 Core
- **OpenGL 3.3+:** API gráfica (nativa do Windows)

### Configuração das Bibliotecas:

#### 1. GLFW Setup:
- Baixe a versão MinGW-w64 do [site oficial](https://www.glfw.org/download.html)
- Extraia os headers para `dependencies/include/GLFW/`
- Copie as bibliotecas para `dependencies/lib/`

#### 2. GLAD Setup:
- Acesse [GLAD Web Service](https://glad.dav1d.de/)
- Configure: OpenGL 3.3+ Core Profile
- Baixe e extraia `glad.h` para `dependencies/include/glad/`
- Copie `glad.c` para `dependencies/lib/`

### Verificação da Instalação:
```powershell
# Teste se MinGW está no PATH
g++ --version

# Teste a compilação
g++ main.cpp dependencies\lib\glad.c -o test.exe -Idependencies\include -Ldependencies\lib -lglfw3 -lopengl32 -lgdi32 -std=c++17
```

## ⚡ Características do Programa

### Visual Feedback:
- **Objeto Ativo:** Exibido com brilho total (100% da cor)
- **Objetos Inativos:** Escurecidos a 70% para indicação visual
- **Instruções Dinâmicas:** Mostradas na tela (toggle com H)
- **Janela:** 1000x800 pixels com título "Projeto 2D OpenGL - Transformações Geométricas"

### Sistema de Transformações:
- **Ordem de Aplicação:** Escala → Rotação → Translação (S × R × T)
- **Coordenadas Homogêneas:** Matrizes 4x4 para transformações 2D
- **Tempo Real:** Atualizações suaves a ~60 FPS
- **Reset Individual:** Cada objeto pode ser resetado independentemente

### Objetos Geométricos Detalhados:

#### 🔺 Triângulo (Objeto 0)
- **Cor:** Vermelho (1.0, 0.0, 0.0)
- **Vértices:** 3 pontos formando triângulo equilátero
- **Posição Inicial:** (-0.6, 0.3) - Superior esquerda

#### 🟢 Quadrado (Objeto 1)  
- **Cor:** Verde (0.0, 1.0, 0.0)
- **Vértices:** 4 pontos com 2 triângulos (6 índices)
- **Posição Inicial:** (0.6, 0.3) - Superior direita

#### 🔵 Hexágono (Objeto 2)
- **Cor:** Azul (0.0, 0.0, 1.0)
- **Vértices:** 7 pontos (1 centro + 6 perímetro)
- **Construção:** Trigonométrica com ângulos calculados
- **Posição Inicial:** (-0.6, -0.3) - Inferior esquerda

#### 🟡 Losango (Objeto 3)
- **Cor:** Amarelo (1.0, 1.0, 0.0)
- **Vértices:** 4 pontos em formato de diamante
- **Proporção:** Mais alto que largo (0.7 altura, 0.4 largura)
- **Posição Inicial:** (0.6, -0.3) - Inferior direita

#### 🔷 Asteroide (Objeto 4)
- **Cor:** Cinza (0.5, 0.5, 0.5)
- **Variação:** Forma e tamanho irregulares
- **Posição Inicial:** Posições aleatórias nas bordas da tela

#### 🟥 Inimigo (Objeto 5)
- **Cor:** Vermelho (1.0, 0.0, 0.0)
- **Comportamento:** Persegue a nave do jogador
- **Posição Inicial:** Posição aleatória longe do jogador

#### 🟩 Power-up (Objeto 6)
- **Cor:** Verde (0.0, 1.0, 0.0)
- **Tipo:** Tiro Rápido ou Escudo, dependendo da cor
- **Posição Inicial:** Posições seguras, longe de inimigos

## 🚨 Solução de Problemas

### Erros de Compilação:
```
"GLFW/glfw3.h: No such file or directory"
```
**Solução:** Verifique se os headers estão em `dependencies/include/GLFW/`

```
"undefined reference to glfwInit"
```
**Solução:** Confirme que `libglfw3.a` está em `dependencies/lib/` e que a task de build está usando `-lglfw3`

```
"glad.h: No such file or directory"
```
**Solução:** Baixe GLAD do site oficial e coloque `glad.h` em `dependencies/include/glad/`

### Erros de Execução:
```
"GLFW3.dll not found"
```
**Solução:** Copie `glfw3.dll` para a mesma pasta do `app.exe`

```
"Failed to initialize GLAD"
```
**Solução:** Certifique-se que `glad.c` está sendo compilado junto e que o OpenGL 3.3+ está disponível

### Problemas de Portabilidade:
- **MinGW não encontrado:** Adicione MinGW ao PATH do Windows
- **VS Code tasks não funcionam:** Verifique se a extensão C/C++ está instalada
- **Performance baixa:** Certifique-se que drivers gráficos estão atualizados

## 🎯 Exemplo de Saída

O programa abre uma janela **1000x800** pixels com:
- **Fundo cinza escuro** 
- **4 objetos geométricos** posicionados nos quadrantes
- **Objeto ativo** (inicial: triângulo vermelho) com brilho total
- **Outros objetos** escurecidos a 70%
- **Instruções** exibidas no console

### Console Output Esperado:
```
=== PROJETO 2D OPENGL - TRANSFORMAÇÕES GEOMÉTRICAS ===
Objetos criados:
0. Triângulo (Vermelho)
1. Quadrado (Verde)  
2. Hexágono (Azul)
3. Losango (Amarelo)

CONTROLES:
- W/A/S/D: Mover objeto
- Q/E: Rotacionar
- R/F: Escalar
- TAB: Trocar objeto
- SPACE: Reset
- H: Toggle instruções
- ESC: Sair
```

## 📚 Documentação Adicional

Para detalhes técnicos completos sobre a implementação, consulte:
- **`PROJETO_TRANSFORMACOES.md`** - Documentação técnica detalhada
- **`main.cpp`** - Código fonte comentado
- **`.vscode/tasks.json`** - Configuração de build

## 🏆 Recursos Educacionais

Este projeto demonstra:
- ✅ **Fundamentos de OpenGL:** VAO, VBO, EBO, Shaders
- ✅ **Álgebra Linear:** Matrizes de transformação 4x4
- ✅ **Geometria Computacional:** Criação de primitivas 2D  
- ✅ **Programação Gráfica:** Pipeline de renderização
- ✅ **Interação Humano-Computador:** Sistema de controles intuitivo

Ideal para estudantes de **Computação Gráfica** e **Processamento de Imagens**.

## 🚀 Pipeline de Renderização OpenGL

### 1. **Vertex Shader**
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(aPos, 1.0);
}
```
- Recebe vértices em coordenadas locais
- Aplica a matriz de transformação composta
- Converte para coordenadas de clip space

### 2. **Fragment Shader**
```glsl
#version 330 core
out vec4 FragColor;
uniform vec3 objectColor;
uniform float alpha;
void main() {
    FragColor = vec4(objectColor, alpha);
}
```
- Define a cor final de cada fragmento
- Suporta transparência via canal alpha
- Permite cores diferentes para cada tipo de objeto

### 3. **Aplicação das Transformações**
```cpp
// Envio da matriz de transformação para a GPU
unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
unsigned int alphaLoc = glGetUniformLocation(shaderProgram, "alpha");

// Para cada objeto
float transform[16] = {0}; // calculado anteriormente
float color[3] = {obj.colorR, obj.colorG, obj.colorB};
float alpha = obj.alpha;

glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);
glUniform3fv(colorLoc, 1, color);
glUniform1f(alphaLoc, alpha);

// Desenhar objeto
glBindVertexArray(obj.VAO);
glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
```
- Transferência da matriz de transformação para GPU via uniform
- Aplicação simultânea de translação, rotação e escala 
- Renderização de diferentes topologias com o mesmo shader
