#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <random>
#include <chrono>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Includes para FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Estruturas
#ifndef POINT_STRUCT_DEFINED
#define POINT_STRUCT_DEFINED
struct Point {
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
};
#endif

#ifndef COLOR_STRUCT_DEFINED
#define COLOR_STRUCT_DEFINED
struct Color {
    float r, g, b, a;
    Color(float r = 1, float g = 1, float b = 1, float a = 1) : r(r), g(g), b(b), a(a) {}
};
#endif

// Estrutura para armazenar informações do caractere FreeType
struct Character {
    unsigned int TextureID; // ID da textura do glifo
    Point Size;      // Tamanho do glifo
    Point Bearing;   // Deslocamento da linha de base para esquerda/topo do glifo
    unsigned int Advance;   // Deslocamento horizontal para o próximo glifo
};

std::map<char, Character> Characters;
FT_Library ft;
FT_Face face;
unsigned int textShaderProgram;
unsigned int textVAO, textVBO;

// Constante para o caminho da fonte e tamanho
const char* FONT_PATH = "fonts/arial.ttf"; // !!! IMPORTANTE: Coloque um arquivo .ttf aqui (ex: arial.ttf)
const unsigned int FONT_PIXEL_HEIGHT_LOAD = 24; // Tamanho base para carregar os glifos

// Função para converter HSL para RGB
Color hslToRgb(float h, float s, float l) {
    h = fmod(h, 360.0f) / 360.0f;
    
    auto hue2rgb = [](float p, float q, float t) {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1.0f/6.0f) return p + (q - p) * 6 * t;
        if (t < 1.0f/2.0f) return q;
        if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6;
        return p;
    };
    
    float r, g, b;
    
    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        float q = l < 0.5f ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        r = hue2rgb(p, q, h + 1.0f/3.0f);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0f/3.0f);
    }
      return Color(r, g, b);
}

struct AFDState {
    std::string name;
    bool isFinal;
    bool isInitial;
    Color color;
    float towerCost;
};

struct AFDTransition {
    std::string fromState;
    char symbol;
    std::string toState;
};

// Vertex Shader Source (para formas)
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

// Fragment Shader Source (para formas)
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
}
)";

// Vertex Shader para Texto
const char* textVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

// Fragment Shader para Texto
const char* textFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D textSampler;
uniform vec3 textColor;
void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textSampler, TexCoords).r);
    FragColor = vec4(textColor, 1.0) * sampled;
}
)";

// Shader Program ID (para formas)
unsigned int shaderProgram;
unsigned int VBO, VAO;

// Constantes do jogo
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 720;
const float TOWER_COST = 50.0f;
const float TOWER_RANGE = 120.0f;
const float TOWER_DAMAGE = 20.0f;
const int TOWER_FIRE_RATE = 75;
const float ENEMY_HEALTH_BASE = 60.0f;
const float ENEMY_SPEED_BASE = 0.0003f; // Reduzido de 0.8f para 0.3f
const float ENEMY_REWARD_BASE = 10.0f;
const int ENEMIES_PER_WAVE_BASE = 5;
const int ENEMY_SPAWN_DELAY = 400; // Frames entre spawns de inimigos

// Variáveis globais do jogo
float money = 120.0f;
int lives = 10;
int currentWave = 0;
std::string placingTowerType = "";
bool waveInProgress = false;
bool gameOver = false;
int frameCount = 0;
int lastEnemySpawnTime = 0;
int enemiesLeftToSpawn = 0;
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

// Caminho dos inimigos
std::vector<Point> path = {
    {0, WINDOW_HEIGHT / 2.0f},
    {150, WINDOW_HEIGHT / 2.0f},
    {150, 100},
    {400, 100},
    {400, 400},
    {650, 400},
    {650, WINDOW_HEIGHT / 2.0f},
    {WINDOW_WIDTH, WINDOW_HEIGHT / 2.0f}
};

// Estados e transições do AFD
std::map<std::string, AFDState> afdStates = {
    {"s0", {"s0", false, true, Color(0.39f, 0.70f, 0.93f), TOWER_COST}}, // Azul claro
    {"s1", {"s1", true, false, Color(0.96f, 0.68f, 0.33f), TOWER_COST}}  // Laranja
};

std::vector<AFDTransition> afdTransitions = {
    {"s0", 'a', "s0"},
    {"s0", 'b', "s1"},
    {"s1", 'a', "s0"},
    {"s1", 'b', "s1"}
};

// Variáveis de interface
std::string feedbackMessage = "";
double mouseX = 0, mouseY = 0;

// Adicionar antes das funções que usam showFeedback
std::string currentFeedback;
float feedbackTimer = 0.0f;

void showFeedback(const std::string& message) {
    currentFeedback = message;
    feedbackTimer = 2.0f; // Mensagem ficará visível por 2 segundos
}

// Função para encontrar transição do AFD
AFDTransition* getAfdTransition(const std::string& fromState, char symbol) {
    for (auto& trans : afdTransitions) {
        if (trans.fromState == fromState && trans.symbol == symbol) {
            return &trans;
        }
    }
    return nullptr;
}

// Função para gerar palavra aleatória do AFD
std::string generateRandomAfdWord(int length = 3) {
    std::string alphabet = "ab";
    std::string word = "";
    for (int i = 0; i < length; i++) {
        word += alphabet[rng() % alphabet.length()];
    }
    return word;
}

// Classes do jogo
class Enemy {
public:
    Point position;
    int pathIndex;
    float radius;
    Color color;
    float speed;
    float maxHealth;
    float health;
    float reward;
    std::string afdWord;
    int currentSymbolIndex;
    bool wordProcessed;

    Enemy(int wave) {
        position = path[0];
        pathIndex = 0;
        radius = 15.0f;
        // Cor aleatória usando HSL como no original
        float hue = rng() % 360;
        color = hslToRgb(hue, 0.6f, 0.55f);
        speed = ENEMY_SPEED_BASE + (wave * 0.05f);
        maxHealth = ENEMY_HEALTH_BASE + (wave * 15);
        health = maxHealth;
        reward = ENEMY_REWARD_BASE + (wave * 2);
        afdWord = generateRandomAfdWord((rng() % 3) + 2);
        currentSymbolIndex = 0;
        wordProcessed = false;
    }

    void update() {
        if (pathIndex < path.size() - 1) {
            Point target = path[pathIndex + 1];
            float dx = target.x - position.x;
            float dy = target.y - position.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < speed) {
                position = target;
                pathIndex++;
            } else {
                position.x += (dx / distance) * speed;
                position.y += (dy / distance) * speed;
            }
        } else {
            lives--;
            health = 0;
            if (lives <= 0) {
                gameOver = true;
            }
        }
    }

    void takeDamage(float amount) {
        health -= amount;
        if (health <= 0) {
            money += reward;
        }
    }

    void processSymbol() {
        if (currentSymbolIndex < afdWord.length()) {
            currentSymbolIndex++;
            if (currentSymbolIndex >= afdWord.length()) {
                wordProcessed = true;
            }
        }
    }
};

class Projectile {
public:
    Point position;
    Enemy* target;
    float radius;
    Color color;
    float speed;
    float damage;
    char processedSymbol;
    bool hasHit;

    Projectile(Point start, Enemy* target, float damage, const std::string& towerState, char symbol) {
        position = start;
        this->target = target;
        radius = 5.0f;
        color = afdStates[towerState].color;
        speed = 6.0f;
        this->damage = damage;
        processedSymbol = symbol;
        hasHit = false;
    }    void update() {
        if (hasHit || !target || target->health <= 0) {
            damage = 0;
            return;
        }

        float dx = target->position.x - position.x;
        float dy = target->position.y - position.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < speed) {
            // Simplificado: sempre causa dano ao atingir o alvo
            target->takeDamage(damage);
            
            // Verificar se deve processar símbolo (sistema AFD opcional)
            if (!target->wordProcessed && target->currentSymbolIndex < target->afdWord.length() &&
                target->afdWord[target->currentSymbolIndex] == processedSymbol) {
                target->processSymbol();
            }
            
            hasHit = true;
            damage = 0;
        } else {
            position.x += (dx / distance) * speed;
            position.y += (dy / distance) * speed;
        }
    }
};

class Tower {
public:
    Point position;
    std::string afdStateName;
    AFDState stateDetails;
    float radius;
    Color color;
    float range;
    float damage;
    int fireRate;
    int lastShotTime;
    Enemy* target;

    Tower(float x, float y, const std::string& stateName) {
        position = Point(x, y);
        afdStateName = stateName;
        stateDetails = afdStates[stateName];
        radius = 20.0f;
        color = stateDetails.color;
        range = TOWER_RANGE;
        damage = TOWER_DAMAGE;
        fireRate = TOWER_FIRE_RATE;
        lastShotTime = 0;
        target = nullptr;
    }

    bool canAttack(Enemy* enemy) {
        if (enemy->wordProcessed || enemy->currentSymbolIndex >= enemy->afdWord.length()) {
            return false;
        }
        char currentSymbol = enemy->afdWord[enemy->currentSymbolIndex];
        return getAfdTransition(afdStateName, currentSymbol) != nullptr;
    }

    void findTarget(std::vector<Enemy>& enemies) {
        target = nullptr;
        float closestDistance = INFINITY;
        
        for (auto& enemy : enemies) {
            if (enemy.health <= 0) continue;

            float dx = enemy.position.x - position.x;
            float dy = enemy.position.y - position.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < range && canAttack(&enemy)) {
                if (distance < closestDistance) {
                    closestDistance = distance;
                    target = &enemy;
                }
            }
        }
    }

    void shoot(std::vector<Projectile>& projectiles, int frame) {
        if (target && target->health > 0 && canAttack(target) && 
            (frame - lastShotTime >= fireRate)) {
            char symbol = target->afdWord[target->currentSymbolIndex];
            projectiles.emplace_back(position, target, damage, afdStateName, symbol);
            lastShotTime = frame;
        }
    }

    void update(std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles, int frame) {
        findTarget(enemies);
        shoot(projectiles, frame);
    }
};

// Variáveis para as entidades do jogo
std::vector<Enemy> enemies;
std::vector<Tower> towers;
std::vector<Projectile> projectiles;

// Declarar protótipos de funções
void setProjectionMatrix(unsigned int currentShaderProgram);
void drawCircle(float x, float y, float radius, Color color);
void drawRectangle(float x, float y, float width, float height, Color color);
void drawLine(Point start, Point end, Color color, float width);
void RenderText(const std::string& text, float x, float y_baseline, float scale, Color color);
void initTextRendering();
void initOpenGL();
void drawPath();
void drawEnemy(const Enemy& enemy);
void drawTower(const Tower& tower);
void drawProjectile(const Projectile& projectile);
void drawUI();
void initGame();
void spawnWave();
void startWave();
bool canPlaceTower(float x, float y);
void placeTower(float x, float y);
void selectTowerType(const std::string& type);
void update();
void render();

// Função para compilar shader
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Erro de compilação do shader: " << infoLog << std::endl;
    }
    
    return shader;
}

// Função para criar programa de shader
unsigned int createAndLinkShaderProgram(const char* vsSource, const char* fsSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Erro de linkagem do programa: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// Modificar a função initTextRendering para criar o shader program e VAO/VBO
void initTextRendering() {
    // Inicializar FreeType
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERRO::FREETYPE: Não foi possível inicializar a biblioteca FreeType" << std::endl;
        return;
    }

    // Tentar carregar a fonte do caminho específico primeiro
    if (FT_New_Face(ft, FONT_PATH, 0, &face)) {
        // Se falhar, tentar uma fonte do Windows como fallback
        std::cout << "AVISO: Não foi possível carregar " << FONT_PATH << ", tentando fonte do sistema..." << std::endl;
        if (FT_New_Face(ft, "C:/Windows/Fonts/arial.ttf", 0, &face)) {
            std::cout << "ERRO::FREETYPE: Falha ao carregar qualquer fonte" << std::endl;
            return;
        }
    }

    // Definir tamanho da fonte
    FT_Set_Pixel_Sizes(face, 0, FONT_PIXEL_HEIGHT_LOAD);

    // Configurar OpenGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Gerar texturas para os primeiros 128 caracteres ASCII
    for (unsigned char c = 0; c < 128; c++) {
        // Carregar caractere
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERRO::FREETYPE: Falha ao carregar glifo " << c << std::endl;
            continue;
        }

        // Gerar textura
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Definir opções de textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Armazenar caractere para uso posterior
        Character character = {
            texture,
            Point(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            Point(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    // Criar o shader program para texto
    textShaderProgram = createAndLinkShaderProgram(textVertexShaderSource, textFragmentShaderSource);
    
    // Configurar o VAO e VBO para o texto
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Substituir a função RenderText com a implementação correta
void RenderText(const std::string& text, float x, float y_baseline, float scale, Color color) {
    // Ativar o shader correspondente
    glUseProgram(textShaderProgram);
    setProjectionMatrix(textShaderProgram);
    
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.r, color.g, color.b);
    glActiveTexture(GL_TEXTURE0);
    // Corrigir: Definir a uniform textSampler para usar a textura ativa
    glUniform1i(glGetUniformLocation(textShaderProgram, "textSampler"), 0);
    glBindVertexArray(textVAO);

    // Iterar por todos os caracteres
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y_baseline - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
        // Atualizar VBO para cada caractere
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        
        // Renderizar a textura do glifo sobre o quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        
        // Atualizar conteúdo do VBO
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Renderizar quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Avançar para o próximo glifo (note que advance é número de 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift por 6 para obter valor em pixels (2^6 = 64)
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Funções de renderização OpenGL modernas
void setProjectionMatrix(unsigned int currentShaderProgram) { // Aceita o shader program
    glUseProgram(currentShaderProgram);
    
    // Matriz de projeção ortográfica
    float left = 0.0f, right = (float)WINDOW_WIDTH;
    float bottom = 0.0f, top = (float)WINDOW_HEIGHT;
    float near = -1.0f, far = 1.0f;
    
    float projection[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f / (far - near), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
    };
    
    int projLoc = glGetUniformLocation(currentShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
}

void drawCircle(float x, float y, float radius, Color color) {
    const int segments = 32;
    std::vector<float> vertices;
    
    // Centro
    vertices.push_back(x);
    vertices.push_back(y);
    
    // Pontos do círculo
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * (float)M_PI * i / segments;
        vertices.push_back(x + cos(angle) * radius);
        vertices.push_back(y + sin(angle) * radius);
    }
    
    glUseProgram(shaderProgram); // Usar shader de formas
    setProjectionMatrix(shaderProgram); // Passar o shader program
    
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawRectangle(float x, float y, float width, float height, Color color) {
    float vertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y + height
    };
    
    glUseProgram(shaderProgram); // Usar shader de formas
    setProjectionMatrix(shaderProgram); // Passar o shader program
    
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawLine(Point start, Point end, Color color, float width = 1.0f) {
    float vertices[] = {
        start.x, start.y,
        end.x, end.y
    };
    
    glUseProgram(shaderProgram); // Usar shader de formas
    setProjectionMatrix(shaderProgram); // Passar o shader program
    
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
    
    glLineWidth(width);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_LINES, 0, 2);
    
    glLineWidth(1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Funções do jogo
void initGame() {
    money = 120.0f;
    lives = 10;
    currentWave = 0;
    enemies.clear();
    towers.clear();
    projectiles.clear();
    placingTowerType = "";
    waveInProgress = false;
    gameOver = false;
    frameCount = 0;
}

void spawnWave() {
    enemiesLeftToSpawn = ENEMIES_PER_WAVE_BASE + (currentWave * 2);
    lastEnemySpawnTime = frameCount;
    // Agora, em vez de criar todos os inimigos de uma vez,
    // vamos criar apenas o primeiro e configurar o contador
    if (enemiesLeftToSpawn > 0) {
        enemies.emplace_back(currentWave);
        enemiesLeftToSpawn--;
    }
}

void startWave() {
    if (gameOver || waveInProgress) return;
    waveInProgress = true;
    currentWave++;
    spawnWave();
}

bool canPlaceTower(float x, float y) {
    const float TOWER_PLACEMENT_MIN_DIST_PATH = 30.0f;
    const float TOWER_PLACEMENT_MIN_DIST_TOWER = 40.0f;

    // Verificar distância de outras torres
    for (const auto& tower : towers) {
        float dx = tower.position.x - x;
        float dy = tower.position.y - y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance < TOWER_PLACEMENT_MIN_DIST_TOWER) {
            showFeedback("Muito perto de outra torre!");
            return false;
        }
    }

    // Verificar distância do caminho (simplificado)
    for (int i = 0; i < path.size() - 1; i++) {
        Point p1 = path[i];
        Point p2 = path[i + 1];
        
        float distToP1 = sqrt((x - p1.x) * (x - p1.x) + (y - p1.y) * (y - p1.y));
        float distToP2 = sqrt((x - p2.x) * (x - p2.x) + (y - p2.y) * (y - p2.y));
        
        if (distToP1 < TOWER_PLACEMENT_MIN_DIST_PATH || distToP2 < TOWER_PLACEMENT_MIN_DIST_PATH) {
            showFeedback("Muito perto do caminho!");
            return false;
        }
    }

    return true;
}

void placeTower(float x, float y) {
    if (placingTowerType.empty() || gameOver) return;
    
    if (money >= afdStates[placingTowerType].towerCost && canPlaceTower(x, y)) {
        towers.emplace_back(x, y, placingTowerType);
        money -= afdStates[placingTowerType].towerCost;
        placingTowerType = "";
        showFeedback("Torre colocada com sucesso!");
    }
}

void selectTowerType(const std::string& type) {
    if (gameOver) return;
    
    if (money >= afdStates[type].towerCost) {
        placingTowerType = type;
        showFeedback("Torre " + type + " selecionada. Clique para colocar.");
    } else {
        showFeedback("Dinheiro insuficiente para Torre " + type + ".");
    }
}

void update() {
    if (gameOver) return;

    // Verificar se é hora de gerar mais inimigos
    if (waveInProgress && enemiesLeftToSpawn > 0 && 
        (frameCount - lastEnemySpawnTime >= ENEMY_SPAWN_DELAY)) {
        enemies.emplace_back(currentWave);
        enemiesLeftToSpawn--;
        lastEnemySpawnTime = frameCount;
    }

    // Atualizar torres
    for (auto& tower : towers) {
        tower.update(enemies, projectiles, frameCount);
    }

    // Atualizar projéteis
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](Projectile& p) {
                p.update();
                return p.damage <= 0 || p.hasHit;
            }),
        projectiles.end()
    );

    // Atualizar inimigos
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](Enemy& e) {
                e.update();
                return e.health <= 0;
            }),
        enemies.end()
    );

    // Verificar fim da onda
    if (waveInProgress && enemies.empty() && enemiesLeftToSpawn == 0) {
        waveInProgress = false;
        if (currentWave >= 10) {
            gameOver = true;
        }
    }

    frameCount++;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND); // Habilitar blend para texto e formas transparentes
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Desenhar caminho
    drawPath();
    
    // Desenhar torres
    for (const auto& tower : towers) {
        drawTower(tower);
    }
    
    // Desenhar projéteis
    for (const auto& projectile : projectiles) {
        drawProjectile(projectile);
    }
      // Desenhar inimigos
    for (const auto& enemy : enemies) {
        drawEnemy(enemy);
    }
    
    // Preview da torre sendo colocada
    if (!placingTowerType.empty()) {
        double mouseX_local, mouseY_local;
        glfwGetCursorPos(glfwGetCurrentContext(), &mouseX_local, &mouseY_local);
        
        // Inverter Y para coordenadas OpenGL
        mouseY_local = WINDOW_HEIGHT - mouseY_local;
        
        Color previewColor = afdStates[placingTowerType].color;
        previewColor.a = 0.5f;
        drawCircle((float)mouseX_local, (float)mouseY_local, 20.0f, previewColor);
        
        // Desenhar alcance (círculo)
        const int segments = 32;
        std::vector<float> vertices_range;
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * (float)M_PI * i / segments;
            vertices_range.push_back((float)mouseX_local + cos(angle) * TOWER_RANGE);
            vertices_range.push_back((float)mouseY_local + sin(angle) * TOWER_RANGE);
        }
        
        glUseProgram(shaderProgram);
        setProjectionMatrix(shaderProgram);
        
        int colorLoc = glGetUniformLocation(shaderProgram, "color");
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 0.3f);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices_range.size() * sizeof(float), vertices_range.data(), GL_DYNAMIC_DRAW);
        
        glDrawArrays(GL_LINE_LOOP, 0, vertices_range.size() / 2);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    // Desenhar interface
    drawUI(); // drawUI agora usa RenderText que configura seu próprio shader e blend
}

// Callbacks do GLFW
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseY = WINDOW_HEIGHT - mouseY; // Inverter Y
        placeTower(mouseX, mouseY);
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1:
                selectTowerType("s0");
                break;
            case GLFW_KEY_2:
                selectTowerType("s1");
                break;
            case GLFW_KEY_SPACE:
                startWave();
                break;
            case GLFW_KEY_R:
                if (gameOver) {
                    initGame();
                }
                break;
            case GLFW_KEY_ESCAPE:
                placingTowerType = "";
                showFeedback("Seleção de torre cancelada");
                break;
        }
    }
}

int main() {
    // Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }

    // Criar janela
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Defesa da Torre com Torres AFD", NULL, NULL);
    if (!window) {
        std::cerr << "Falha ao criar janela" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }    // Configurar callbacks
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Configurar OpenGL
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.82f, 0.88f, 0.82f, 1.0f); // Verde claro

    // Inicializar renderização moderna
    initOpenGL();

    // Inicializar jogo
    initGame();

    std::cout << "=== Defesa da Torre com Torres AFD ===" << std::endl;
    std::cout << "Controles:" << std::endl;
    std::cout << "1 - Selecionar Torre S0" << std::endl;
    std::cout << "2 - Selecionar Torre S1" << std::endl;
    std::cout << "Espaço - Iniciar Onda" << std::endl;
    std::cout << "R - Reiniciar (quando game over)" << std::endl;
    std::cout << "ESC - Cancelar seleção de torre" << std::endl;
    std::cout << "Clique para colocar torre selecionada" << std::endl;
    std::cout << std::endl;
    std::cout << "Dinheiro: " << money << " | Vidas: " << lives << " | Onda: " << currentWave << std::endl;

    // Loop principal
    float lastFrameTime = (float)glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = (float)glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        glfwPollEvents();
        
        update();
        render();
        
        glfwSwapBuffers(window);
        
        // Imprimir informações do jogo
        static int lastFrame = 0;
        if (frameCount % 60 == 0 && frameCount != lastFrame) {
            std::cout << "Dinheiro: " << money << " | Vidas: " << lives << " | Onda: " << currentWave;
            std::cout << " | Inimigos: " << enemies.size() << " | Torres: " << towers.size() << std::endl;
            lastFrame = frameCount;
            
            if (gameOver) {
                if (lives <= 0) {
                    std::cout << "GAME OVER! Pressione R para reiniciar." << std::endl;
                } else {
                    std::cout << "VITÓRIA! Você completou todas as ondas! Pressione R para reiniciar." << std::endl;
                }
            }
        }
    }
    
    // Limpar recursos do FreeType
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Limpar VAOs/VBOs
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Limpar recursos do texto
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(textShaderProgram);

    glfwTerminate();
    return 0;
}

// Função para inicializar OpenGL
void initOpenGL() {
    // Criar shader program para formas
    shaderProgram = createAndLinkShaderProgram(vertexShaderSource, fragmentShaderSource);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    initTextRendering(); // Inicializar renderização de texto com FreeType
}

// Implementação das funções que estavam faltando
void drawPath() {
    for (int i = 0; i < path.size() - 1; i++) {
        drawLine(path[i], path[i + 1], Color(0.63f, 0.67f, 0.75f), 20.0f);
    }
}

void drawEnemy(const Enemy& enemy) {
    // Corpo do inimigo
    drawCircle(enemy.position.x, enemy.position.y, enemy.radius, enemy.color);
    
    // Contorno preto
    const int segments = 32;
    std::vector<float> vertices;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * (float)M_PI * i / segments;
        vertices.push_back(enemy.position.x + cos(angle) * enemy.radius);
        vertices.push_back(enemy.position.y + sin(angle) * enemy.radius);
    }
    
    glUseProgram(shaderProgram);
    setProjectionMatrix(shaderProgram);
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, 0, 0, 0, 1);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Barra de vida
    float healthBarWidth = enemy.radius * 1.5f;
    float healthBarHeight = 5.0f;
    float healthPercentage = enemy.health / enemy.maxHealth;
    
    float barX = enemy.position.x - healthBarWidth / 2;
    float barY = enemy.position.y - enemy.radius - 12;
    
    drawRectangle(barX, barY, healthBarWidth, healthBarHeight, Color(1, 0, 0));
    drawRectangle(barX, barY, healthBarWidth * healthPercentage, healthBarHeight, Color(0, 1, 0));
    
    // Palavra AFD com símbolo atual destacado
    std::string wordDisplay = "";
    for (int i = 0; i < enemy.afdWord.length(); i++) {
        if (i == enemy.currentSymbolIndex && !enemy.wordProcessed) {
            wordDisplay += "[";
            wordDisplay += enemy.afdWord[i];
            wordDisplay += "]";
        } else {
            wordDisplay += enemy.afdWord[i];
        }
    }
    
    if (enemy.wordProcessed) {
        wordDisplay = enemy.afdWord + " (+)";
    }
    
    float enemy_text_scale = 0.5f;
    float ascender_offset = (FONT_PIXEL_HEIGHT_LOAD * 0.75f) * enemy_text_scale;
    float textX = enemy.position.x - (wordDisplay.length() * 5.0f * enemy_text_scale);
    float textY_baseline = enemy.position.y + enemy.radius + 10 + ascender_offset;
    
    RenderText(wordDisplay, textX, textY_baseline, enemy_text_scale, Color(0, 0, 0));
}

void drawTower(const Tower& tower) {
    // Corpo da torre
    drawCircle(tower.position.x, tower.position.y, tower.radius, tower.color);
    
    // Contorno preto
    const int segments = 32;
    std::vector<float> vertices;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * (float)M_PI * i / segments;
        vertices.push_back(tower.position.x + cos(angle) * tower.radius);
        vertices.push_back(tower.position.y + sin(angle) * tower.radius);
    }
    
    glUseProgram(shaderProgram);
    setProjectionMatrix(shaderProgram);
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, 0, 0, 0, 1);
    glLineWidth(2.0f);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2);
    glLineWidth(1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Nome do estado na torre
    std::string stateName = tower.afdStateName;
    std::transform(stateName.begin(), stateName.end(), stateName.begin(), ::toupper);
    
    float tower_text_scale = 0.6f;
    float ascender_offset = (FONT_PIXEL_HEIGHT_LOAD * 0.75f) * tower_text_scale;
    float textX = tower.position.x - (stateName.length() * 4.0f * tower_text_scale);
    float textY_baseline = tower.position.y - 4 + ascender_offset;
    
    RenderText(stateName, textX, textY_baseline, tower_text_scale, Color(0, 0, 0));
    
    // Linha para o alvo
    if (tower.target && tower.target->health > 0 && !tower.target->wordProcessed) {
        drawLine(tower.position, tower.target->position, Color(0.29f, 0.33f, 0.41f), 2.0f);
    }
}

void drawProjectile(const Projectile& projectile) {
    drawCircle(projectile.position.x, projectile.position.y, projectile.radius, projectile.color);
}

void drawUI() {
    // Fundo da barra de informações
    drawRectangle(0, WINDOW_HEIGHT - 60, WINDOW_WIDTH, 60, Color(1.0f, 1.0f, 1.0f, 0.9f));
    
    // Bordas da barra
    drawLine(Point(0, WINDOW_HEIGHT - 60), Point(WINDOW_WIDTH, WINDOW_HEIGHT - 60), Color(0.29f, 0.33f, 0.41f), 2.0f);
    
    // Informações do jogo
    std::stringstream ss;
    float ui_text_scale = 0.8f;
    float ascender_offset = (FONT_PIXEL_HEIGHT_LOAD * 0.75f) * ui_text_scale;

    // Dinheiro
    ss.str("");
    ss << "Dinheiro: " << (int)money;
    RenderText(ss.str(), 20, (WINDOW_HEIGHT - 50) + ascender_offset, ui_text_scale, Color(0.18f, 0.22f, 0.28f));
    
    // Vidas
    ss.str("");
    ss << "Vidas: " << lives;
    RenderText(ss.str(), 200, (WINDOW_HEIGHT - 50) + ascender_offset, ui_text_scale, Color(0.18f, 0.22f, 0.28f));
    
    // Onda
    ss.str("");
    ss << "Onda: " << currentWave;
    RenderText(ss.str(), 350, (WINDOW_HEIGHT - 50) + ascender_offset, ui_text_scale, Color(0.18f, 0.22f, 0.28f));
    
    // Status da torre sendo colocada
    if (!placingTowerType.empty()) {
        ss.str("");
        ss << "Colocando Torre " << placingTowerType << " (ESC para cancelar)";
        RenderText(ss.str(), 500, (WINDOW_HEIGHT - 50) + ascender_offset, ui_text_scale * 0.75f, Color(0.18f, 0.22f, 0.28f));
    }
    
    // Controles na parte inferior
    float controls_text_scale = 0.6f;
    ascender_offset = (FONT_PIXEL_HEIGHT_LOAD * 0.75f) * controls_text_scale;
    RenderText("1: Torre S0  2: Torre S1  ESPACO: Iniciar Onda", 20, (WINDOW_HEIGHT - 25) + ascender_offset, controls_text_scale, Color(0.18f, 0.22f, 0.28f));
    
    // Feedback do AFD
    if (feedbackTimer > 0) {
        float feedback_text_scale = 0.7f;
        ascender_offset = (FONT_PIXEL_HEIGHT_LOAD * 0.75f) * feedback_text_scale;
        float feedbackY_baseline = 50 + ascender_offset; 
        float feedbackX = 50;

        RenderText(currentFeedback, feedbackX, feedbackY_baseline, feedback_text_scale, Color(0.88f, 0.92f, 0.94f));
        
        feedbackTimer -= 0.016f;
    }
    
    // Game Over ou Vitória
    if (gameOver) {
        std::string message;
        if (lives <= 0) {
            message = "GAME OVER! Pressione R para reiniciar";
        } else {
            message = "VITORIA! Pressione R para reiniciar";
        }
        
        float gameover_text_scale = 1.2f;
        ascender_offset = (FONT_PIXEL_HEIGHT_LOAD * 0.75f) * gameover_text_scale;
        float messageX = 100;
        float messageY_baseline = WINDOW_HEIGHT / 2.0f + ascender_offset;
        
        // Fundo escuro
        drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color(0, 0, 0, 0.7f));
        
        // Texto da mensagem
        RenderText(message, messageX, messageY_baseline, gameover_text_scale, Color(1, 1, 1));
    }
}