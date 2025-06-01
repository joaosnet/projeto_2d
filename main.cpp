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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Sistema de fontes bitmap simples (8x8 pixels por caractere)
const int CHAR_WIDTH = 8;
const int CHAR_HEIGHT = 8;

// Bitmap font para caracteres básicos (simplificado)
std::map<char, std::vector<std::vector<bool>>> fontBitmap = {
    {'0', {{0,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'1', {{0,0,1,0,0,0,0,0}, {0,1,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'2', {{0,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {0,0,0,0,1,0,0,0}, {0,0,1,1,0,0,0,0}, {0,1,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0}, {1,1,1,1,1,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'3', {{1,1,1,1,1,0,0,0}, {0,0,0,1,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,1,0,0,0,0}, {0,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'4', {{0,0,0,1,0,0,0,0}, {0,0,1,1,0,0,0,0}, {0,1,0,1,0,0,0,0}, {1,0,0,1,0,0,0,0}, {1,1,1,1,1,0,0,0}, {0,0,0,1,0,0,0,0}, {0,0,0,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'5', {{1,1,1,1,1,0,0,0}, {1,0,0,0,0,0,0,0}, {1,1,1,1,0,0,0,0}, {0,0,0,0,1,0,0,0}, {0,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'6', {{0,0,1,1,0,0,0,0}, {0,1,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0}, {1,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'7', {{1,1,1,1,1,0,0,0}, {0,0,0,0,1,0,0,0}, {0,0,0,1,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'8', {{0,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'9', {{0,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,1,0,0,0}, {0,0,0,0,1,0,0,0}, {0,0,0,1,0,0,0,0}, {0,1,1,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'S', {{0,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,0,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'a', {{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,1,1,1,0,0,0,0}, {0,0,0,0,1,0,0,0}, {0,1,1,1,1,0,0,0}, {1,0,0,0,1,0,0,0}, {0,1,1,1,1,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'b', {{1,0,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0}, {1,1,1,1,0,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,0}, {1,1,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {' ', {{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {':', {{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'[', {{0,1,1,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,1,1,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {']', {{0,1,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,1,1,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {'(', {{0,0,1,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
    {')', {{1,0,0,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,1,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},    {'+', {{0,0,0,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {1,1,1,1,1,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}},
};

// Estruturas
struct Point {
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Color {
    float r, g, b, a;
    Color(float r = 1, float g = 1, float b = 1, float a = 1) : r(r), g(g), b(b), a(a) {}
};

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

// Vertex Shader Source
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

// Fragment Shader Source
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
}
)";

// Shader Program ID
unsigned int shaderProgram;
unsigned int VBO, VAO;

// Constantes do jogo
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float TOWER_COST = 50.0f;
const float TOWER_RANGE = 120.0f;
const float TOWER_DAMAGE = 20.0f;
const int TOWER_FIRE_RATE = 75;
const float ENEMY_HEALTH_BASE = 60.0f;
const float ENEMY_SPEED_BASE = 0.8f;
const float ENEMY_REWARD_BASE = 10.0f;
const int ENEMIES_PER_WAVE_BASE = 5;

// Estruturas
struct Point {
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Color {
    float r, g, b, a;
    Color(float r = 1, float g = 1, float b = 1, float a = 1) : r(r), g(g), b(b), a(a) {}
};

// Variáveis globais do jogo
float money = 120.0f;
int lives = 10;
int currentWave = 0;
std::string placingTowerType = "";
bool waveInProgress = false;
bool gameOver = false;
int frameCount = 0;
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
int feedbackTimer = 0;
double mouseX = 0, mouseY = 0;

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
    }

    void update() {
        if (hasHit || !target || target->health <= 0) {
            damage = 0;
            return;
        }

        float dx = target->position.x - position.x;
        float dy = target->position.y - position.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < speed) {
            if (!target->wordProcessed && target->currentSymbolIndex < target->afdWord.length() &&
                target->afdWord[target->currentSymbolIndex] == processedSymbol) {
                target->takeDamage(damage);
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
void createShaderProgram() {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Erro de linkagem do programa: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// Função para inicializar OpenGL
void initOpenGL() {
    createShaderProgram();
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Funções de renderização OpenGL modernas
void setProjectionMatrix() {
    glUseProgram(shaderProgram);
    
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
    
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
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
    
    glUseProgram(shaderProgram);
    setProjectionMatrix();
    
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
    
    glUseProgram(shaderProgram);
    setProjectionMatrix();
    
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
    
    glUseProgram(shaderProgram);
    setProjectionMatrix();
    
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

// Função para renderizar texto bitmap
void drawText(const std::string& text, float x, float y, Color color, float scale = 1.0f) {
    float currentX = x;
    
    for (char c : text) {
        if (fontBitmap.find(c) != fontBitmap.end()) {
            auto& bitmap = fontBitmap[c];
            
            for (int row = 0; row < CHAR_HEIGHT; row++) {
                for (int col = 0; col < CHAR_WIDTH; col++) {
                    if (bitmap[row][col]) {
                        float pixelX = currentX + col * scale;
                        float pixelY = y + row * scale;
                        drawRectangle(pixelX, pixelY, scale, scale, color);
                    }
                }
            }
        }
        currentX += CHAR_WIDTH * scale;
    }
}

// Função para desenhar interface do usuário
void drawUI() {
    // Fundo da barra de informações
    drawRectangle(0, WINDOW_HEIGHT - 60, WINDOW_WIDTH, 60, Color(1.0f, 1.0f, 1.0f, 0.9f));
    
    // Bordas da barra
    drawLine(Point(0, WINDOW_HEIGHT - 60), Point(WINDOW_WIDTH, WINDOW_HEIGHT - 60), Color(0.29f, 0.33f, 0.41f), 2.0f);
    
    // Informações do jogo
    std::stringstream ss;
    
    // Dinheiro
    ss.str("");
    ss << "Dinheiro: " << (int)money;
    drawText(ss.str(), 20, WINDOW_HEIGHT - 50, Color(0.18f, 0.22f, 0.28f), 2.0f);
    
    // Vidas
    ss.str("");
    ss << "Vidas: " << lives;
    drawText(ss.str(), 200, WINDOW_HEIGHT - 50, Color(0.18f, 0.22f, 0.28f), 2.0f);
    
    // Onda
    ss.str("");
    ss << "Onda: " << currentWave;
    drawText(ss.str(), 350, WINDOW_HEIGHT - 50, Color(0.18f, 0.22f, 0.28f), 2.0f);
    
    // Status da torre sendo colocada
    if (!placingTowerType.empty()) {
        ss.str("");
        ss << "Colocando Torre " << placingTowerType << " (ESC para cancelar)";
        drawText(ss.str(), 500, WINDOW_HEIGHT - 50, Color(0.18f, 0.22f, 0.28f), 1.5f);
    }
    
    // Controles na parte inferior
    drawText("1: Torre S0  2: Torre S1  ESPACO: Iniciar Onda", 20, WINDOW_HEIGHT - 25, Color(0.18f, 0.22f, 0.28f), 1.0f);
    
    // Feedback do AFD
    if (feedbackTimer > 0) {
        float feedbackY = 50;
        float textWidth = feedbackMessage.length() * CHAR_WIDTH * 1.5f;
        float feedbackX = (WINDOW_WIDTH - textWidth) / 2;
        
        // Fundo semi-transparente
        drawRectangle(feedbackX - 10, feedbackY - 5, textWidth + 20, 20, Color(0.18f, 0.22f, 0.28f, 0.85f));
        drawText(feedbackMessage, feedbackX, feedbackY, Color(0.88f, 0.92f, 0.94f), 1.5f);
        
        feedbackTimer--;
    }
    
    // Game Over ou Vitória
    if (gameOver) {
        std::string message;
        if (lives <= 0) {
            message = "GAME OVER! Pressione R para reiniciar";
        } else {
            message = "VITORIA! Pressione R para reiniciar";
        }
        
        float textWidth = message.length() * CHAR_WIDTH * 3.0f;
        float messageX = (WINDOW_WIDTH - textWidth) / 2;
        float messageY = WINDOW_HEIGHT / 2;
        
        // Fundo escuro
        drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color(0, 0, 0, 0.7f));
        
        // Texto da mensagem
        drawText(message, messageX, messageY, Color(1, 1, 1), 3.0f);
    }
}

// Função para mostrar feedback
void showFeedback(const std::string& message) {
    feedbackMessage = message;
    feedbackTimer = 150; // ~2.5 segundos a 60fps
}

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
    setProjectionMatrix();
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
    
    float textWidth = wordDisplay.length() * CHAR_WIDTH * 1.0f;
    float textX = enemy.position.x - textWidth / 2;
    float textY = enemy.position.y + enemy.radius + 10;
    
    drawText(wordDisplay, textX, textY, Color(0, 0, 0), 1.0f);
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
    setProjectionMatrix();
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
    
    float textWidth = stateName.length() * CHAR_WIDTH * 1.0f;
    float textX = tower.position.x - textWidth / 2;
    float textY = tower.position.y - 4;
    
    drawText(stateName, textX, textY, Color(0, 0, 0), 1.0f);
    
    // Linha para o alvo
    if (tower.target && tower.target->health > 0 && !tower.target->wordProcessed) {
        drawLine(tower.position, tower.target->position, Color(0.29f, 0.33f, 0.41f), 2.0f);
    }
}

void drawProjectile(const Projectile& projectile) {
    drawCircle(projectile.position.x, projectile.position.y, projectile.radius, projectile.color);
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
    int numEnemies = ENEMIES_PER_WAVE_BASE + (currentWave * 2);
    for (int i = 0; i < numEnemies; i++) {
        enemies.emplace_back(currentWave);
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
    if (waveInProgress && enemies.empty()) {
        waveInProgress = false;
        if (currentWave >= 10) {
            gameOver = true;
        }
    }

    frameCount++;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
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
        double mouseX, mouseY;
        glfwGetCursorPos(glfwGetCurrentContext(), &mouseX, &mouseY);
        
        // Inverter Y para coordenadas OpenGL
        mouseY = WINDOW_HEIGHT - mouseY;
        
        Color previewColor = afdStates[placingTowerType].color;
        previewColor.a = 0.5f;
        drawCircle((float)mouseX, (float)mouseY, 20.0f, previewColor);
        
        // Desenhar alcance (círculo)
        const int segments = 32;
        std::vector<float> vertices;
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * (float)M_PI * i / segments;
            vertices.push_back((float)mouseX + cos(angle) * TOWER_RANGE);
            vertices.push_back((float)mouseY + sin(angle) * TOWER_RANGE);
        }
        
        glUseProgram(shaderProgram);
        setProjectionMatrix();
        
        int colorLoc = glGetUniformLocation(shaderProgram, "color");
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 0.3f);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        
        glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    // Desenhar interface
    drawUI();
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
    while (!glfwWindowShouldClose(window)) {
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

    glfwTerminate();
    return 0;
}