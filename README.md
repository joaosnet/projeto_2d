# 🏰 TOWER DEFENSE - Projeto OpenGL 2D

Um jogo Tower Defense desenvolvido em OpenGL moderno (3.3 Core Profile) que demonstra **modelagem geométrica**, **transformações geométricas** e **renderização de texto** com FreeType através de uma experiência interativa responsiva.

# Tarefas do Projeto:
- [x] Modelagem geométrica de objetos 2D (círculos, retângulos, linhas)
- [x] Implementação de transformações geométricas (translação, projeção ortográfica)
- [x] Sistema de torres com diferentes tipos (Básica e Avançada)
- [x] Janela responsiva maximizada automaticamente
- [x] Sistema de renderização de texto com FreeType
- [x] Interface completa do jogo (vida, dinheiro, ondas, etc)
- [x] Sistema de ondas progressivas de inimigos
- [x] Cores procedurais HSL para inimigos


## 📋 Descrição do Projeto

**Tower Defense** é um projeto que implementa um jogo de defesa de torres em OpenGL moderno, onde o jogador posiciona torres estrategicamente para defender contra ondas de inimigos. O objetivo principal é demonstrar a modelagem de objetos geométricos 2D, transformações geométricas e renderização de texto utilizando FreeType.

### 🔍 Principais Componentes:
- **Modelagem Geométrica:** Círculos (torres/inimigos/projéteis), retângulos (interface), linhas (caminho/trajetórias)
- **Transformações Geométricas:** Translação para movimento, projeção ortográfica responsiva
- **Sistema de Coordenadas:** Coordenadas cartesianas 2D com projeção ortográfica adaptativa
- **Pipeline OpenGL:** Shaders modernos (OpenGL 3.3 Core Profile)
- **Renderização de Texto:** FreeType integrado para interface rica
- **Sistema Responsivo:** Janela maximizada automaticamente com redimensionamento dinâmico

## 🔷 Modelagem Geométrica Implementada

### 1. **Torres (Círculos)**
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
- **Tipos:** Torre Básica (azul) e Torre Avançada (laranja)
- **Funcionalidade:** Sistema de targeting automático com alcance específico

### 2. **Inimigos (Círculos Coloridos)**
```cpp
// Inimigos com cores HSL procedurais
Enemy(int wave) {
    float hue = rng() % 360;
    color = hslToRgb(hue, 0.7f, 0.6f); // Cores vibrantes
    speed = ENEMY_SPEED_BASE + (wave * 0.0005f * ENEMY_SPEED_BASE);
    maxHealth = ENEMY_HEALTH_BASE + (wave * 15);
    health = maxHealth;
    reward = ENEMY_REWARD_BASE + (wave * 2);
}
```
- **Geometria:** Círculos com raio 15px
- **Cores:** Geração HSL aleatória para máxima diversidade visual
- **Progressão:** Saúde, velocidade e recompensa aumentam por onda
- **Movimento:** Translação suave ao longo do caminho predefinido

### 3. **Projéteis (Círculos Pequenos)**
```cpp
// Projéteis com targeting inteligente
Projectile(Point start, Enemy* target, float damage, Color color) {
    position = start;
    this->target = target;
    radius = 5.0f;
    this->color = color; // Cor da torre origem
    speed = 6.0f;
    this->damage = damage;
    hasHit = false;
}
```
- **Geometria:** Círculos pequenos (raio 5px)
- **Cor:** Herdam a cor da torre que os disparou
- **Funcionalidade:** Sistema de targeting com colisão por distância
- **Inteligência:** Seguem inimigos em movimento até o impacto

### 4. **Interface Gráfica (Retângulos e Texto)**
```cpp
// Sistema de renderização de texto com FreeType
void RenderText(const std::string& text, float x, float y_baseline, 
                float scale, Color color) {
    // Renderização avançada de texto com suporte a caracteres especiais
    // Inclui acentos portugueses: áàâãéèêíìîóòôõúùûç
}

// Elementos de UI e feedback visual
void drawRectangle(float x, float y, float width, float height, Color color) {
    float vertices[] = {x, y, x + width, y, x + width, y + height, x, y + height};
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
```
- **Barras de Vida:** Retângulos vermelhos/verdes proporcionais à saúde
- **Interface Principal:** Informações em tempo real (dinheiro, vidas, onda)
- **Sistema de Texto:** FreeType para renderização profissional de fontes
- **Feedback Visual:** Mensagens temporárias para ações do usuário
- **Caracteres Especiais:** Suporte completo a acentos portugueses

### 5. **Caminho dos Inimigos (Linhas)**
```cpp
// Caminho responsivo baseado em proporções da janela
void updatePath() {
    path.clear();
    path = {
        {0.0f, static_cast<float>(WINDOW_HEIGHT) / 2.0f},
        {static_cast<float>(WINDOW_WIDTH) * 0.15f, static_cast<float>(WINDOW_HEIGHT) / 2.0f},
        {static_cast<float>(WINDOW_WIDTH) * 0.15f, static_cast<float>(WINDOW_HEIGHT) * 0.14f},
        {static_cast<float>(WINDOW_WIDTH) * 0.4f, static_cast<float>(WINDOW_HEIGHT) * 0.14f},
        {static_cast<float>(WINDOW_WIDTH) * 0.4f, static_cast<float>(WINDOW_HEIGHT) * 0.56f},
        {static_cast<float>(WINDOW_WIDTH) * 0.65f, static_cast<float>(WINDOW_HEIGHT) * 0.56f},
        {static_cast<float>(WINDOW_WIDTH) * 0.65f, static_cast<float>(WINDOW_HEIGHT) / 2.0f},
        {static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT) / 2.0f}
    };
}
```
- **Geometria:** Sequência de linhas conectadas formando um percurso em zigue-zague
- **Renderização:** `GL_LINES` com espessura configurável
- **Responsividade:** Recalculado automaticamente quando a janela é redimensionada
- **Funcionalidade:** Define trajetória dos inimigos do início ao fim da tela

## 🏰 Sistema de Torres Implementado

### Tipos de Torres:
```cpp
std::map<std::string, TowerType> towerTypes = {
    {"basic", {"Básica", Color(0.3f, 0.7f, 1.0f), 50.0f, 1.0f, 120.0f, 10}},
    {"advanced", {"Avançada", Color(1.0f, 0.6f, 0.2f), 70.0f, 2.0f, 150.0f, 50}}
};
```

### Características das Torres:

#### 🔵 Torre Básica (Azul)
- **Custo:** $50
- **Dano:** 1.0 por projétil
- **Alcance:** 120 pixels (normalizado para diferentes resoluções)
- **Taxa de Tiro:** 10 frames entre disparos (~6 tiros/segundo a 60 FPS)
- **Cor:** Azul claro (`Color(0.3f, 0.7f, 1.0f)`)

#### 🟠 Torre Avançada (Laranja)
- **Custo:** $70
- **Dano:** 2.0 por projétil
- **Alcance:** 150 pixels (normalizado para diferentes resoluções)
- **Taxa de Tiro:** 50 frames entre disparos (~1.2 tiros/segundo a 60 FPS)
- **Cor:** Laranja (`Color(1.0f, 0.6f, 0.2f)`)

### Sistema de Targeting:
```cpp
void Tower::findTarget(std::vector<Enemy>& enemies) {
    target = nullptr;
    float closestDistance = INFINITY;
    
    for (auto& enemy : enemies) {
        if (enemy.health <= 0) continue;
        float distance = sqrt(dx*dx + dy*dy);
        if (distance < range && distance < closestDistance) {
            closestDistance = distance;
            target = &enemy;
        }
    }
}
```

### Mecânica do Jogo:
- **Economia:** Comece com $120, ganhe dinheiro eliminando inimigos
- **Posicionamento:** Torres não podem ser muito próximas do caminho ou outras torres
- **Estratégia:** Balanceamento entre custo, dano e taxa de tiro
- **Alcance:** Visualização do alcance durante o posicionamento

## 🎮 Controles do Jogo

### Comandos Principais:
- **1:** Selecionar Torre Básica (azul - custo $50)
- **2:** Selecionar Torre Avançada (laranja - custo $70)
- **ESPAÇO:** Iniciar próxima onda de inimigos
- **ESC:** Cancelar seleção de torre
- **R:** Reiniciar jogo (quando Game Over)
- **Clique do Mouse:** Posicionar torre selecionada

### Mecânicas de Jogo:
- **Economia:** Comece com $120, ganhe dinheiro eliminando inimigos
- **Torres:** Diferentes custos, danos, alcances e taxas de tiro
- **Vida:** 10 vidas iniciais, perde 1 vida por inimigo que escape
- **Ondas:** Sistema progressivo de dificuldade crescente
- **Posicionamento:** Torres não podem sobrepor caminho ou outras torres
- **Interface Responsiva:** Funciona em qualquer resolução de tela

## 🚀 Como Executar

### Compilação e Execução Rápida (VS Code):
1. Abra o projeto no VS Code
2. Pressione `Ctrl + Shift + P`
3. Digite "Tasks: Run Task"
4. Selecione "**build and run**"

### Via Terminal (PowerShell):
```powershell
# Compilar e Executar
g++ main.cpp dependencies/lib/glad.c -o modern_tower_defense.exe -Idependencies/include -Ldependencies/lib -lglfw3 -lfreetype -lopengl32 -lgdi32 -std=c++17 && .\modern_tower_defense.exe
```

### Comando de Compilação Detalhado:
```powershell
# Apenas Compilar
g++ main.cpp dependencies/lib/glad.c -o modern_tower_defense.exe -Idependencies/include -Ldependencies/lib -lglfw3 -lfreetype -lopengl32 -lgdi32 -std=c++17

# Executar
.\modern_tower_defense.exe
```

### Dependências Necessárias:
- **GLFW3:** Gerenciamento de janelas e input
- **GLAD:** Loader de extensões OpenGL
- **FreeType:** Renderização de texto
- **OpenGL:** API gráfica
- **GDI32:** Sistema gráfico do Windows
- **C++17:** Padrão da linguagem utilizado

## 🌟 Transformações Geométricas Demonstradas

### 1. **Sistema Responsivo e Normalização**
```cpp
// Janela maximizada automaticamente com detecção de resolução
GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
WINDOW_WIDTH = mode->width;
WINDOW_HEIGHT = mode->height;
glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

// Torres usam coordenadas normalizadas para responsividade
class Tower {
    Point normalizedPos;    // Posição normalizada (0.0-1.0)
    float normalizedRadius; // Raio normalizado
    float normalizedRange;  // Alcance normalizado
};
```
- **Aplicação:** Suporte a qualquer resolução de monitor
- **Implementação:** Coordenadas normalizadas + redimensionamento dinâmico
- **Resultado:** Jogo funciona perfeitamente em 4K, ultrawide, ou monitores pequenos

### 2. **Translação (T)**
```cpp
// Movimento dos inimigos ao longo do caminho
void Enemy::update() {
    Point target = path[pathIndex + 1];
    float dx = target.x - position.x;
    float dy = target.y - position.y;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < speed) {
        position = target; // Chegou ao próximo ponto
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
- **Matemática:** Normalização de vetores direcionais para velocidade constante

### 3. **Projeção Ortográfica Responsiva**
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
    
    return (distance < range && enemy->health > 0);
}
```

### Sistema de Targeting:
```cpp
// Seleção do inimigo mais próximo atacável
void Tower::findTarget(std::vector<Enemy>& enemies) {
    target = nullptr;
    float closestDistance = INFINITY;
    
    for (auto& enemy : enemies) {
        if (enemy.health <= 0) continue;
        float dx = enemy.position.x - position.x;
        float dy = enemy.position.y - position.y;
        float distance = sqrt(dx*dx + dy*dy);
        
        if (distance < range && distance < closestDistance) {
            closestDistance = distance;
            target = &enemy;
        }
    }
}
```

### Geração Procedural:
```cpp
// Geração de cores HSL aleatórias para inimigos
Enemy(int wave) {
    float hue = rng() % 360;
    color = hslToRgb(hue, 0.7f, 0.6f); // Cores vibrantes
    speed = ENEMY_SPEED_BASE + (wave * 0.0005f * ENEMY_SPEED_BASE);
    maxHealth = ENEMY_HEALTH_BASE + (wave * 15);
    health = maxHealth;
    reward = ENEMY_REWARD_BASE + (wave * 2);
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
- **Interface Responsiva:** Adaptável a diferentes resoluções de tela
- **Targeting:** Torres destacam seus alvos atuais
- **Preview de Torre:** Visualização antes do posicionamento

### Interface Rica:
- **Renderização FreeType:** Sistema profissional de texto com acentos portugueses
- **Informações em Tempo Real:** Dinheiro, vidas, onda atual
- **Feedback de Sistema:** Mensagens temporárias para ações do usuário
- **Console Debug:** Informações detalhadas no terminal

### Esquema de Cores:
- **Torre Básica:** Azul (`Color(0.3f, 0.7f, 1.0f)`)
- **Torre Avançada:** Laranja (`Color(1.0f, 0.6f, 0.2f)`)
- **Inimigos:** Cores HSL procedurais para máxima distinção visual
- **Interface:** Design moderno com contraste adequado

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

### ✅ Sistema de Renderização de Texto:
- **FreeType Integration:** Renderização profissional de fontes
- **Suporte Unicode:** Caracteres especiais e acentos portugueses
- **Escalabilidade:** Texto adaptável a diferentes resoluções
- **Performance:** Caching de glifos para eficiência

## 🎮 Experiência de Jogo

### Progressão Equilibrada:
- **10 Ondas:** Dificuldade crescente gradual
- **Economia Estratégica:** Gerenciamento de recursos
- **Posicionamento Tático:** Localização das torres é crítica
- **Timing:** Coordenação entre ondas e construção

### Mecânicas Educativas:
- **Geometria Interativa:** Círculos, retângulos e linhas em ação
- **Transformações Visuais:** Translação e projeção em tempo real
- **Sistema Responsivo:** Adaptação dinâmica a diferentes resoluções
- **Feedback Imediato:** Torres atacam automaticamente alvos no alcance
- **Experimentação:** Jogador explora estratégias de posicionamento

## 📚 Aplicação Educacional

Este projeto demonstra conceitos fundamentais de:

### Computação Gráfica:
- **Pipeline de Renderização:** Vertex Processing → Rasterization → Fragment Processing
- **Transformações 2D:** Translação e projeção ortográfica responsiva
- **Sistemas de Coordenadas:** Conversões e normalização para responsividade
- **Geometria Computacional:** Círculos, distâncias, colisões, targeting

### Renderização de Texto:
- **FreeType Integration:** Biblioteca profissional para fontes
- **Unicode Support:** Caracteres especiais e acentos
- **Shader Programming:** Vertex e Fragment shaders para texto
- **Performance Optimization:** Caching de glifos e batch rendering

### Programação de Jogos:
- **Game Loop:** Input → Update → Render
- **Gerenciamento de Estado:** Múltiplas entidades independentes
- **Sistemas de Targeting:** Algoritmos de seleção de alvos
- **Interface Responsiva:** Adaptação automática a diferentes telas

### Matemática Aplicada:
- **Álgebra Linear:** Matrizes de transformação
- **Geometria:** Distâncias euclidianas, normalização de vetores
- **Trigonometria:** Geração de círculos via seno/cosseno
- **Probabilidade:** Geração procedural e randomização

## 📋 Estrutura do Projeto

```
TOWER_DEFENSE/
│
├── main.cpp                 # Código fonte principal (1358 linhas)
├── modern_tower_defense.exe         # Executável compilado
├── README.md                # Este documento
│
└── dependencies/            # Bibliotecas externas
    ├── include/             # Headers das bibliotecas
    │   ├── glad/glad.h      # OpenGL Loader
    │   ├── GLFW/glfw3.h     # Window Management
    │   ├── ft2build.h       # FreeType
    │   └── freetype/        # FreeType Headers
    └── lib/                 # Bibliotecas linkadas
        ├── glad.c           # OpenGL Loader Source
        ├── glfw3.dll        # GLFW Dynamic Library
        ├── libglfw3.a       # GLFW Static Library
        ├── freetype.lib     # FreeType Library
        └── libfreetype.a    # FreeType Static Library
```

## 🔧 Conceitos Técnicos Implementados

### Modelagem Geométrica:
- **Círculos:** 32 segmentos via Triangle Fan para torres, inimigos e projéteis
- **Retângulos:** 4 vértices via Triangle Fan para interface e barras de vida
- **Linhas:** Primitive GL_LINES para caminho dos inimigos
- **Texto:** FreeType para renderização profissional de fontes

### Transformações Geométricas:
- **Translação:** `T(dx, dy)` para movimento suave de entidades
- **Projeção Ortográfica:** Matriz 4x4 responsiva para diferentes resoluções
- **Normalização:** Sistema de coordenadas adaptativo para responsividade
- **Conversão de Coordenadas:** GLFW ↔ OpenGL com suporte a múltiplas resoluções

### Pipeline OpenGL:
- **Vertex Shader:** Transformação de vértices via matriz de projeção uniforme
- **Fragment Shader:** Colorização uniforme e renderização de texto
- **Buffer Management:** VAO/VBO dinâmicos para geometria em tempo real
- **Dual Shader System:** Shaders separados para formas e texto

## 🚀 Conclusão

**Tower Defense** representa uma implementação exemplar dos conceitos de transformações geométricas em OpenGL moderno, combinando:

✨ **Técnica Sólida:** Demonstração rigorosa de conceitos de computação gráfica  
🖼️ **Inovação Visual:** FreeType integrado para texto profissional  
🎮 **Experiência Envolvente:** Jogo estratégico e responsivo  
📖 **Valor Didático:** Código bem estruturado e documentado  
🖥️ **Responsividade:** Suporte completo a diferentes resoluções

O projeto excede os requisitos básicos ao implementar um sistema completo de renderização com suporte a texto avançado, geometria responsiva e pipeline OpenGL moderno, criando uma experiência educativa robusta que demonstra conceitos fundamentais de computação gráfica de forma prática e interativa.

**Explore geometria 2D jogando! 🏰**
