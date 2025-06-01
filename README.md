# 🏰 TOWER DEFENSE AFD - Projeto OpenGL 2D

Um jogo Tower Defense desenvolvido em OpenGL que demonstra **modelagem geométrica**, **transformações geométricas** e **teoria de autômatos finitos determinísticos** através de uma experiência interativa.

# Tarefas do Projeto:
- [x] Modelagem geométrica de objetos 2D (círculos, retângulos, linhas)
- [x] Implementação de transformações geométricas (translação, projeção ortográfica)
- [x] Implementação de um sistema de autômatos finitos determinísticos (AFD) como mecânica de jogo
- [x] Tamanho da janela 1920x720 pixels
- [ ] Informações de Introdução e controle do jogo (vida, dinheiro, ondas, etc) na 
interface
- [ ] Escalabilidade da velocidade das ondas de inimigos ao longo do jogo


## 📋 Descrição do Projeto

**Tower Defense AFD** é um projeto que implementa um jogo de defesa de torres em OpenGL, onde as torres funcionam como **Autômatos Finitos Determinísticos (AFD)** que processam palavras carregadas pelos inimigos. O objetivo principal é demonstrar a modelagem de objetos geométricos 2D, transformações geométricas e conceitos de teoria da computação.

### 🔍 Principais Componentes:
- **Modelagem Geométrica:** Círculos (torres/inimigos/projéteis), retângulos (interface), linhas (caminho/trajetórias)
- **Transformações Geométricas:** Translação para movimento, projeção ortográfica
- **Sistema de Coordenadas:** Coordenadas cartesianas 2D com projeção ortográfica
- **Pipeline OpenGL:** Shaders modernos (OpenGL 3.3 Core Profile)
- **Teoria de Autômatos:** AFD implementado para mecânica de jogo

## 🔷 Modelagem Geométrica Implementada

### 1. **Torres AFD (Círculos)**
```cpp
// Geometria: Círculo com centro e raio
void drawCircle(float x, float y, float radius, Color color) {
    const int segments = 32;
    // Centro + pontos do círculo
    vertices.push_back(x); vertices.push_back(y);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(x + cos(angle) * radius);
        vertices.push_back(y + sin(angle) * radius);
    }
}
```
- **Geometria:** 34 vértices (1 centro + 33 perímetro com fechamento)
- **Topologia:** Triangle Fan (leque de triângulos)
- **Estados AFD:** s0 (azul claro), s1 (laranja)
- **Funcionalidade:** Processam símbolos 'a' e 'b' segundo transições AFD

### 2. **Inimigos (Círculos Coloridos)**
```cpp
// Inimigos com cores HSL aleatórias
Enemy(int wave) {
    float hue = rng() % 360;
    color = hslToRgb(hue, 0.6f, 0.55f);
    // Cada inimigo carrega uma palavra do alfabeto {a,b}
    afdWord = generateRandomAfdWord((rng() % 3) + 2);
}
```
- **Geometria:** Círculos com raio 15px
- **Cores:** Geração HSL aleatória para diversidade visual
- **Dados:** Cada inimigo carrega uma palavra (2-4 símbolos)
- **Movimento:** Translação ao longo do caminho predefinido

### 3. **Projéteis (Círculos Pequenos)**
```cpp
// Projéteis que carregam símbolos AFD
Projectile(Point start, Enemy* target, float damage, 
           const std::string& towerState, char symbol) {
    radius = 5.0f;
    color = afdStates[towerState].color; // Cor da torre origem
    processedSymbol = symbol; // Símbolo a ser processado
}
```
- **Geometria:** Círculos pequenos (raio 5px)
- **Cor:** Herdam a cor da torre que os disparou
- **Funcionalidade:** Transportam símbolos do AFD para processamento

### 4. **Interface Gráfica (Retângulos)**
```cpp
// Elementos de UI e feedback visual
void drawRectangle(float x, float y, float width, float height, Color color) {
    float vertices[] = {x, y, x + width, y, x + width, y + height, x, y + height};
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
```
- **Barras de Vida:** Retângulos vermelhos/verdes proporcionais à saúde
- **Interface Principal:** Barra inferior com informações do jogo
- **Sistema de Texto:** Fonte bitmap 8x8 pixels renderizada como micro-retângulos

### 5. **Caminho dos Inimigos (Linhas)**
```cpp
// Caminho predefinido por pontos conectados
std::vector<Point> path = {
    {0, WINDOW_HEIGHT / 2.0f}, {150, WINDOW_HEIGHT / 2.0f},
    {150, 100}, {400, 100}, {400, 400}, {650, 400},
    {650, WINDOW_HEIGHT / 2.0f}, {WINDOW_WIDTH, WINDOW_HEIGHT / 2.0f}
};
```
- **Geometria:** Sequência de linhas conectadas
- **Renderização:** `GL_LINES` com espessura configurável
- **Funcionalidade:** Define trajetória dos inimigos

## 🤖 Sistema AFD Implementado

### Estados do Autômato:
```cpp
std::map<std::string, AFDState> afdStates = {
    {"s0", {"s0", false, true, Color(0.39f, 0.70f, 0.93f), TOWER_COST}}, // Estado inicial
    {"s1", {"s1", true, false, Color(0.96f, 0.68f, 0.33f), TOWER_COST}}  // Estado final
};
```

### Transições:
```cpp
std::vector<AFDTransition> afdTransitions = {
    {"s0", 'a', "s0"}, // s0 --a--> s0
    {"s0", 'b', "s1"}, // s0 --b--> s1
    {"s1", 'a', "s0"}, // s1 --a--> s0
    {"s1", 'b', "s1"}  // s1 --b--> s1
};
```

### Mecânica do Jogo:
- **Torres:** Representam estados do AFD (s0, s1)
- **Inimigos:** Carregam palavras do alfabeto {a, b}
- **Combate:** Torres só podem atacar inimigos cujo próximo símbolo é aceito pela transição
- **Processamento:** Projéteis "consomem" símbolos das palavras dos inimigos
- **Vitória:** Inimigo é destruído quando toda sua palavra é processada

## 🎮 Controles do Jogo

### Comandos Principais:
- **1:** Selecionar Torre S0 (estado inicial - azul)
- **2:** Selecionar Torre S1 (estado final - laranja)
- **ESPAÇO:** Iniciar próxima onda de inimigos
- **ESC:** Cancelar seleção de torre
- **R:** Reiniciar jogo (quando Game Over)

### Mecânicas de Jogo:
- **Economia:** Comece com $120, ganhe dinheiro eliminando inimigos
- **Torres:** Custam $50 cada, têm alcance e taxa de tiro específicos
- **Vida:** 10 vidas iniciais, perde 1 vida por inimigo que escape
- **Ondas:** 10 ondas progressivamente mais difíceis
- **Posicionamento:** Torres não podem ser muito próximas do caminho ou outras torres

## 🚀 Como Executar

### Compilação e Execução Rápida (VS Code):
1. Abra o projeto no VS Code
2. Pressione `Ctrl + Shift + P`
3. Digite "Tasks: Run Task"
4. Selecione "**build and run**"

### Via Terminal (PowerShell):
```powershell
# Compilar
g++ main.cpp dependencies\lib\glad.c -o tower_defense.exe -Idependencies\include -Ldependencies\lib -lglfw3 -lopengl32 -lgdi32 -std=c++17

# Executar
.\tower_defense.exe
```

## 🌟 Transformações Geométricas Demonstradas

### 1. **Translação (T)**
```cpp
// Movimento dos inimigos ao longo do caminho
void Enemy::update() {
    Point target = path[pathIndex + 1];
    float dx = target.x - position.x;
    float dy = target.y - position.y;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < speed) {
        position = target; // Teleporte para próximo ponto
        pathIndex++;
    } else {
        // Translação suave
        position.x += (dx / distance) * speed;
        position.y += (dy / distance) * speed;
    }
}
```
- **Aplicação:** Movimento suave de inimigos e projéteis
- **Implementação:** Interpolação linear entre pontos do caminho
- **Matemática:** Normalização de vetores direcionais

### 2. **Projeção Ortográfica**
```cpp
// Matriz de projeção ortográfica para renderização 2D
void setProjectionMatrix() {
    float left = 0.0f, right = (float)WINDOW_WIDTH;
    float bottom = 0.0f, top = (float)WINDOW_HEIGHT;
    float near = -1.0f, far = 1.0f;
    
    float projection[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f / (far - near), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), 
        -(far + near) / (far - near), 1.0f
    };
}
```
- **Funcionalidade:** Converte coordenadas do mundo para coordenadas da tela
- **Implementação:** Matriz 4x4 aplicada via shaders
- **Resultado:** Renderização 2D precisa em pixels

### 3. **Transformação de Coordenadas**
```cpp
// Conversão mouse -> OpenGL (inversão Y)
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    mouseY = WINDOW_HEIGHT - mouseY; // Inverter eixo Y
    placeTower(mouseX, mouseY);
}
```
- **Necessidade:** GLFW usa origem superior-esquerda, OpenGL usa inferior-esquerda
- **Solução:** Inversão matemática do eixo Y
- **Aplicação:** Posicionamento preciso de torres via mouse

## 🔧 Implementação Técnica OpenGL

### Pipeline Moderno (OpenGL 3.3 Core):
```cpp
// Vertex Shader - Processamento de vértices
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

// Fragment Shader - Colorização
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
}
)";
```

### Gestão de Buffers:
```cpp
// VAO + VBO para geometria dinâmica
unsigned int VBO, VAO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);

// Upload dinâmico de geometria
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
             vertices.data(), GL_DYNAMIC_DRAW);
```

### Características Técnicas:
- **Shaders Programáveis:** Vertex e Fragment shaders customizados
- **Buffers Dinâmicos:** Geometria gerada em tempo real
- **Alpha Blending:** Transparência para efeitos visuais
- **Viewport Transform:** Mapeamento para coordenadas de tela

## 🎯 Algoritmos e Sistemas Implementados

### Sistema de Detecção de Alcance:
```cpp
// Verificação se inimigo está no alcance da torre
bool Tower::canAttack(Enemy* enemy) {
    float dx = enemy->position.x - position.x;
    float dy = enemy->position.y - position.y;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < range && !enemy->wordProcessed) {
        char currentSymbol = enemy->afdWord[enemy->currentSymbolIndex];
        return getAfdTransition(afdStateName, currentSymbol) != nullptr;
    }
    return false;
}
```

### Sistema de Targeting:
```cpp
// Seleção do inimigo mais próximo atacável
void Tower::findTarget(std::vector<Enemy>& enemies) {
    target = nullptr;
    float closestDistance = INFINITY;
    
    for (auto& enemy : enemies) {
        float distance = sqrt(dx*dx + dy*dy);
        if (distance < range && canAttack(&enemy) && distance < closestDistance) {
            closestDistance = distance;
            target = &enemy;
        }
    }
}
```

### Geração Procedural:
```cpp
// Geração aleatória de palavras AFD
std::string generateRandomAfdWord(int length = 3) {
    std::string alphabet = "ab";
    std::string word = "";
    for (int i = 0; i < length; i++) {
        word += alphabet[rng() % alphabet.length()];
    }
    return word;
}
```

### Sistema de Cores HSL:
```cpp
// Conversão HSL para RGB para variedade visual
Color hslToRgb(float h, float s, float l) {
    // Implementação completa de conversão de espaço de cores
    // Permite geração de cores vibrantes e harmônicas
}
```

## 🎨 Características Visuais

### Sistema de Feedback Visual:
- **Alcance de Torres:** Círculo semi-transparente durante posicionamento
- **Barras de Vida:** Indicadores vermelhos/verdes proporcional à saúde
- **Estado da Palavra:** Símbolo atual destacado entre colchetes `[a]`
- **Targeting:** Linhas conectando torres aos seus alvos
- **Preview de Torre:** Visualização antes do posicionamento

### Interface Rica:
- **Fonte Bitmap:** Sistema de renderização de texto customizado 8x8 pixels
- **Informações em Tempo Real:** Dinheiro, vidas, onda atual
- **Feedback de Sistema:** Mensagens temporárias para ações do usuário
- **Console Debug:** Informações detalhadas no terminal

### Esquema de Cores:
- **Torres S0:** Azul claro (`Color(0.39f, 0.70f, 0.93f)`)
- **Torres S1:** Laranja (`Color(0.96f, 0.68f, 0.33f)`)
- **Inimigos:** Cores HSL aleatórias para distinção visual
- **Interface:** Tons de cinza com contraste adequado

## 🏆 Requisitos Técnicos Atendidos

### ✅ Modelagem Geométrica:
- **Primitivas 2D:** Círculos (32 segmentos), retângulos, linhas
- **Representação Eficiente:** Triangle Fan para círculos, Triangle Strip para retângulos
- **Geometria Dinâmica:** Vértices calculados em tempo real

### ✅ Transformações Geométricas:
- **Translação:** Movimento fluido via interpolação linear
- **Projeção:** Matriz ortográfica 4x4 para mapeamento 2D
- **Coordenadas:** Sistema cartesiano com origem configurável

### ✅ Pipeline OpenGL Moderno:
- **OpenGL 3.3 Core:** Sem funcionalidades obsoletas
- **Shaders Programáveis:** Vertex e Fragment customizados
- **Buffers Eficientes:** VAO/VBO com upload dinâmico
- **Alpha Blending:** Transparência e efeitos visuais

### ✅ Teoria da Computação:
- **AFD Funcional:** Estados e transições implementados
- **Processamento de Palavras:** Símbolos consumidos sequencialmente
- **Validação:** Torres só atacam se transição existe

## 🎮 Experiência de Jogo

### Progressão Equilibrada:
- **10 Ondas:** Dificuldade crescente gradual
- **Economia Estratégica:** Gerenciamento de recursos
- **Posicionamento Tático:** Localização das torres é crítica
- **Timing:** Coordenação entre ondas e construção

### Mecânicas Educativas:
- **Visualização AFD:** Estados e transições claramente representados
- **Processamento Visual:** Símbolos destacados durante consumo
- **Feedback Imediato:** Torres só disparam se transição válida
- **Experimentação:** Jogador descobre comportamentos AFD

## 📚 Aplicação Educacional

Este projeto demonstra conceitos fundamentais de:

### Computação Gráfica:
- **Pipeline de Renderização:** Vertex Processing → Rasterization → Fragment Processing
- **Transformações 2D:** Translação e projeção ortográfica
- **Sistemas de Coordenadas:** Conversões entre diferentes espaços
- **Geometria Computacional:** Círculos, distâncias, colisões

### Teoria da Computação:
- **Autômatos Finitos:** Estados, transições, alfabetos
- **Processamento de Linguagens:** Reconhecimento de palavras
- **Aplicação Prática:** AFD como mecânica de jogo
- **Visualização:** Conceitos abstratos tornados tangíveis

### Programação de Jogos:
- **Game Loop:** Input → Update → Render
- **Gerenciamento de Estado:** Múltiplas entidades independentes
- **Sistemas de Targeting:** Algoritmos de seleção de alvos
- **Interface de Usuário:** Feedback visual e interação

### Matemática Aplicada:
- **Álgebra Linear:** Matrizes de transformação
- **Geometria:** Distâncias euclidianas, normalização de vetores
- **Trigonometria:** Geração de círculos via seno/cosseno
- **Probabilidade:** Geração procedural e randomização

## 📋 Estrutura do Projeto

```
TOWER_DEFENSE_AFD/
│
├── main.cpp                 # Código fonte principal (1057 linhas)
├── tower_defense.exe        # Executável compilado
├── README.md                # Este documento
│
└── dependencies/            # Bibliotecas externas
    ├── include/             # Headers OpenGL/GLFW
    │   ├── glad/glad.h
    │   ├── GLFW/glfw3.h
    │   └── KHR/khrplatform.h
    └── lib/                 # Bibliotecas linkadas
        ├── glad.c
        ├── glfw3.dll
        └── libglfw3.a
```

## 🔧 Conceitos Técnicos Implementados

### Modelagem Geométrica:
- **Círculos:** 32 segmentos via Triangle Fan
- **Retângulos:** 4 vértices via Triangle Fan
- **Linhas:** Primitive GL_LINES com espessura configurável
- **Texto:** Fonte bitmap 8x8 pixels como micro-retângulos

### Transformações Geométricas:
- **Translação:** `T(dx, dy)` para movimento de entidades
- **Projeção Ortográfica:** Matriz 4x4 para mapeamento 2D
- **Conversão de Coordenadas:** GLFW ↔ OpenGL

### Pipeline OpenGL:
- **Vertex Shader:** Transformação de vértices via matriz de projeção
- **Fragment Shader:** Colorização uniforme por primitiva
- **Buffer Management:** VAO/VBO dinâmicos
- **Uniform Variables:** Cor e matriz de projeção

## 🚀 Conclusão

**Tower Defense AFD** representa uma implementação exemplar dos conceitos de transformações geométricas em OpenGL, combinando:

✨ **Técnica Sólida:** Demonstração rigorosa de conceitos de computação gráfica  
🤖 **Inovação Educativa:** AFD como mecânica de jogo única  
🎮 **Experiência Envolvente:** Jogo estratégico e desafiador  
📖 **Valor Didático:** Código bem estruturado e documentado  

O projeto excede os requisitos básicos ao implementar um sistema completo que integra teoria da computação com computação gráfica, criando uma experiência educativa única que torna conceitos abstratos em mecânicas de jogo tangíveis e divertidas.

**Aprenda AFD jogando! 🏰**
