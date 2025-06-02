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

// Altere a definição do mapa Characters para usar int como chave
std::map<int, Character> Characters;
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

// Definição de tipos de torres
struct TowerType {
    std::string name;
    Color color;
    float cost;
    float damage;
    float range;
    int fireRate;
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

// Variáveis para o tamanho da janela (agora são variáveis em vez de constantes)
int WINDOW_WIDTH = 1000;
int WINDOW_HEIGHT = 720;

// Constantes do jogo
const float TOWER_COST_BASIC = 50.0f;
const float TOWER_COST_ADVANCED = 70.0f;
const float TOWER_RANGE_BASIC = 120.0f;
const float TOWER_RANGE_ADVANCED = 150.0f;
const float TOWER_DAMAGE_BASIC = 0.35f;
const float TOWER_DAMAGE_ADVANCED = 0.80f;
const int TOWER_FIRE_RATE_BASIC = 10;
const int TOWER_FIRE_RATE_ADVANCED = 50;
const float ENEMY_HEALTH_BASE = 60.0f;
const float ENEMY_SPEED_BASE = 0.1f;
const float ENEMY_REWARD_BASE = 10.0f;
const int ENEMIES_PER_WAVE_BASE = 5;
const int ENEMY_SPAWN_DELAY = 1000; // Reduzido de 700 para 60 (aproximadamente 1 segundo a 60 FPS)

// NOVAS CORES PARA UI E ELEMENTOS
const Color COLOR_TEXT_UI = Color(0.9f, 0.9f, 0.9f);
const Color COLOR_PATH = Color(0.2f, 0.25f, 0.3f, 0.8f);
const Color COLOR_BUTTON_NORMAL = Color(0.25f, 0.3f, 0.35f, 0.7f);
const Color COLOR_BUTTON_HOVER = Color(0.35f, 0.4f, 0.45f, 0.8f); // Não implementado neste exemplo, mas para referência
const Color COLOR_BUTTON_SELECTED = Color(0.4f, 0.8f, 1.0f, 0.9f);
const Color COLOR_FEEDBACK_BG = Color(0.15f, 0.18f, 0.22f, 0.85f);
const Color COLOR_GAMEOVER_BG = Color(0.1f, 0.1f, 0.1f, 0.9f);

// Tipos de torres disponíveis
std::map<std::string, TowerType> towerTypes = {
    {"basic", {"Básica", Color(0.3f, 0.7f, 1.0f), TOWER_COST_BASIC, TOWER_DAMAGE_BASIC, TOWER_RANGE_BASIC, TOWER_FIRE_RATE_BASIC}},
    {"advanced", {"Avançada", Color(1.0f, 0.6f, 0.2f), TOWER_COST_ADVANCED, TOWER_DAMAGE_ADVANCED, TOWER_RANGE_ADVANCED, TOWER_FIRE_RATE_ADVANCED}}
};

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

// Caminho dos inimigos (será ajustado dinamicamente com base no tamanho da janela)
std::vector<Point> path;

// Variáveis de interface
std::string feedbackMessage = "";
double mouseX = 0, mouseY = 0;

// Adicionar antes das funções que usam showFeedback
std::string currentFeedback;
float feedbackTimer = 0.0f;

// Função para redimensionar o caminho quando a janela é redimensionada
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

// Callback para redimensionamento da janela - apenas declare o protótipo aqui
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void showFeedback(const std::string& message) {
    currentFeedback = message;
    feedbackTimer = 2.0f; // Mensagem ficará visível por 2 segundos
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

    Enemy(int wave) {
        position = path[0];
        pathIndex = 0;
        radius = 15.0f;
        // Cor aleatória usando HSL, ajustada para ser mais vibrante
        float hue = rng() % 360;
        color = hslToRgb(hue, 0.7f, 0.6f); // Saturação e luminosidade ajustadas
        speed = ENEMY_SPEED_BASE + (wave * 0.0005f * ENEMY_SPEED_BASE); // Ajuste mais sutil na velocidade
        maxHealth = ENEMY_HEALTH_BASE + (wave * 15);
        health = maxHealth;
        reward = ENEMY_REWARD_BASE + (wave * 2);
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
};

class Projectile {
public:
    Point position;
    Enemy* target;
    float radius;
    Color color;
    float speed;
    float damage;
    bool hasHit;

    Projectile(Point start, Enemy* target, float damage, Color color) {
        position = start;
        this->target = target;
        radius = 5.0f;
        this->color = color;
        speed = 6.0f;
        this->damage = damage;
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
            target->takeDamage(damage);
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
    Point position;         // Posição atual em pixels
    Point normalizedPos;    // Posição normalizada (0.0-1.0)
    std::string typeName;
    TowerType typeDetails;
    float radius;
    float normalizedRadius; // Raio normalizado em relação à altura da janela
    Color color;
    float range;
    float normalizedRange;  // Alcance normalizado em relação à altura da janela
    float damage;
    int fireRate;
    int lastShotTime;
    Enemy* target;

    Tower(float x, float y, const std::string& typeName) {
        // Armazenar coordenadas normalizadas
        normalizedPos.x = x / static_cast<float>(WINDOW_WIDTH);
        normalizedPos.y = y / static_cast<float>(WINDOW_HEIGHT);
        position = Point(x, y);
        
        this->typeName = typeName;
        typeDetails = towerTypes[typeName];
        
        // Calcular raio normalizado (em relação à altura da janela)
        normalizedRadius = 20.0f / static_cast<float>(WINDOW_HEIGHT);
        radius = 20.0f;
        
        color = typeDetails.color;
        
        // Calcular alcance normalizado
        normalizedRange = typeDetails.range / static_cast<float>(WINDOW_HEIGHT);
        range = typeDetails.range;
        
        damage = typeDetails.damage;
        fireRate = typeDetails.fireRate;
        lastShotTime = 0;
        target = nullptr;
    }

    // Atualizar a posição e dimensões com base no tamanho atual da janela
    void updateDimensions() {
        position.x = normalizedPos.x * static_cast<float>(WINDOW_WIDTH);
        position.y = normalizedPos.y * static_cast<float>(WINDOW_HEIGHT);
        radius = normalizedRadius * static_cast<float>(WINDOW_HEIGHT);
        range = normalizedRange * static_cast<float>(WINDOW_HEIGHT);
    }

    void findTarget(std::vector<Enemy>& enemies) {
        target = nullptr;
        float closestDistance = INFINITY;
        
        for (auto& enemy : enemies) {
            if (enemy.health <= 0) continue;

            float dx = enemy.position.x - position.x;
            float dy = enemy.position.y - position.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < range) {
                if (distance < closestDistance) {
                    closestDistance = distance;
                    target = &enemy;
                }
            }
        }
    }

    void shoot(std::vector<Projectile>& projectiles, int frame) {
        if (target && target->health > 0 && (frame - lastShotTime >= fireRate)) {
            projectiles.emplace_back(position, target, damage, color);
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

// Agora adicione a função updateTowerDimensions() aqui, depois da declaração de 'towers'
void updateTowerDimensions() {
    for (auto& tower : towers) {
        tower.updateDimensions();
    }
}

// Implementação completa do callback de redimensionamento
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    glViewport(0, 0, width, height);
    updatePath();
    updateTowerDimensions();
}

// Declarar protótipos de funções
void setProjectionMatrix(unsigned int currentShaderProgram);
void drawCircle(float x, float y, float radius, Color color);
void drawRectangle(float x, float y, float width, float height, Color color);
void drawLine(Point start, Point end, Color color, float width);
void drawHexagon(float x, float y, float radius, Color color); // Novo protótipo
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

// Adicione esta constante antes da função initTextRendering
const std::string EXTENDED_CHARS = "áàâãéèêíìîóòôõúùûçÁÀÂÃÉÈÊÍÌÎÓÒÔÕÚÙÛÇ";

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
    
    // Ativar suporte a Unicode
    FT_Select_Charmap(face, FT_ENCODING_UNICODE);

    // Configurar OpenGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Gerar texturas para os primeiros 128 caracteres ASCII
    for (int c = 0; c < 128; c++) {
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
        Characters.insert(std::pair<int, Character>(c, character));
    }

    // Lista de caracteres especiais para pré-carregar
    const wchar_t specialChars[] = L"áàâãéèêíìîóòôõúùûçÁÀÂÃÉÈÊÍÌÎÓÒÔÕÚÙÛÇ";
    
    // Adicionar caracteres especiais do português
    for (int i = 0; specialChars[i] != L'\0'; i++) {
        wchar_t c = specialChars[i];
        
        // Carregar caractere
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERRO::FREETYPE: Falha ao carregar glifo especial " << (int)c << std::endl;
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

        // Armazenar caractere para uso posterior usando o valor Unicode como chave
        Character character = {
            texture,
            Point(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            Point(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
        Characters.insert(std::pair<int, Character>((int)c, character));
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

// Função auxiliar para obter o próximo ponto de código UTF-8
int utf8_next(const std::string& str, size_t& pos) {
    if (pos >= str.length()) return 0;
    
    unsigned char c = str[pos++];
    
    // ASCII comum (0-127)
    if (c < 128) return c;
    
    // Obter o número de bytes adicionais
    int extra = 0;
    if ((c & 0xE0) == 0xC0) extra = 1;      // 110xxxxx
    else if ((c & 0xF0) == 0xE0) extra = 2; // 1110xxxx
    else if ((c & 0xF8) == 0xF0) extra = 3; // 11110xxx
    else return '?';  // Formato inválido
    
    // Calcular o ponto de código
    int codepoint = (c & (0x3F >> extra));
    
    // Ler bytes adicionais
    for (int i = 0; i < extra && pos < str.length(); i++) {
        c = str[pos++];
        if ((c & 0xC0) != 0x80) return '?'; // Formato inválido
        codepoint = (codepoint << 6) | (c & 0x3F);
    }
    
    return codepoint;
}

// Substituir a função RenderText com a implementação correta para Unicode
void RenderText(const std::string& text, float x, float y_baseline, float scale, Color color) {
    // Ativar o shader correspondente
    glUseProgram(textShaderProgram);
    setProjectionMatrix(textShaderProgram);
    
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.r, color.g, color.b);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(textShaderProgram, "textSampler"), 0);
    glBindVertexArray(textVAO);

    // Percorrer a string usando a função auxiliar para caracteres UTF-8
    size_t pos = 0;
    while (pos < text.length()) {
        int codepoint = utf8_next(text, pos);
        if (codepoint == 0) break;

        // Verificar se temos este caractere carregado
        if (Characters.find(codepoint) == Characters.end()) {
            x += (Characters['?'].Advance >> 6) * scale; // Corrigido: primeiro faz o shift, depois multiplica
            continue;
        }

        Character ch = Characters[codepoint];

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
        
        // Avançar para o próximo glifo
        x += (ch.Advance >> 6) * scale; // Corrigido: primeiro faz o shift, depois multiplica
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

// Nova função para desenhar hexágono
void drawHexagon(float x, float y, float radius, Color color) {
    const int segments = 6; // Um hexágono tem 6 lados
    std::vector<float> vertices;

    // Centro (para TRIANGLE_FAN)
    vertices.push_back(x);
    vertices.push_back(y);

    // Pontos do hexágono
    for (int i = 0; i <= segments; i++) { // <= para fechar o hexágono
        float angle = 2.0f * (float)M_PI * i / segments + (M_PI / 6.0f); // Adiciona rotação para alinhar a base
        vertices.push_back(x + cos(angle) * radius);
        vertices.push_back(y + sin(angle) * radius);
    }

    glUseProgram(shaderProgram);
    setProjectionMatrix(shaderProgram);

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
    const float TOWER_COST_BASIC = 50.0f;
const float TOWER_COST_ADVANCED = 70.0f;
const float TOWER_RANGE_BASIC = 120.0f;
const float TOWER_RANGE_ADVANCED = 150.0f;
const float TOWER_DAMAGE_BASIC = 0.35f;
const float TOWER_DAMAGE_ADVANCED = 0.80f;
const int TOWER_FIRE_RATE_BASIC = 10;
const int TOWER_FIRE_RATE_ADVANCED = 50;
const float ENEMY_HEALTH_BASE = 60.0f;
const float ENEMY_SPEED_BASE = 0.003f;
const float ENEMY_REWARD_BASE = 10.0f;
const int ENEMIES_PER_WAVE_BASE = 5;
const int ENEMY_SPAWN_DELAY = 700; // Frames entre spawns de inimigos

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

    // Verificar distância do caminho
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
    
    if (money >= towerTypes[placingTowerType].cost && canPlaceTower(x, y)) {
        towers.emplace_back(x, y, placingTowerType);
        money -= towerTypes[placingTowerType].cost;
        placingTowerType = "";
        showFeedback("Torre colocada com sucesso!");
    }
}

void selectTowerType(const std::string& type) {
    if (gameOver) return;
    
    if (money >= towerTypes[type].cost) {
        placingTowerType = type;
        showFeedback("Torre " + towerTypes[type].name + " selecionada. Clique para colocar.");
    } else {
        showFeedback("Dinheiro insuficiente para Torre " + towerTypes[type].name + ".");
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
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f); // Nova cor de fundo
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
        
        // Ajustar o raio e alcance proporcionalmente ao tamanho da janela
        float heightRatio = static_cast<float>(WINDOW_HEIGHT) / 720.0f;
        float previewRadius = 20.0f * heightRatio;
        float previewRange = towerTypes[placingTowerType].range * heightRatio;
        
        // Primeiro desenhar o círculo de alcance (mais transparente)
        Color rangeColor(0.5f, 0.5f, 0.5f, 0.3f); // Cor mais visível para o range
        drawCircle((float)mouseX_local, (float)mouseY_local, previewRange, rangeColor);
        
        // Depois desenhar a torre
        Color previewColor = towerTypes[placingTowerType].color;
        previewColor.a = 0.7f; // Aumentar opacidade
        drawCircle((float)mouseX_local, (float)mouseY_local, previewRadius, previewColor);
        
        // Não precisamos mais desenhar o contorno separadamente já que o círculo de range já está visível
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
                selectTowerType("basic");
                break;
            case GLFW_KEY_2:
                selectTowerType("advanced");
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

    // Obter a resolução do monitor primário
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    WINDOW_WIDTH = mode->width;
    WINDOW_HEIGHT = mode->height;

    // Definir dicas para a janela
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Criar janela
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tower Defense", NULL, NULL);
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
    }
    
    // Configurar callbacks
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Obter o tamanho real da janela após a maximização
    glfwGetFramebufferSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
    
    // Configurar OpenGL
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.82f, 0.88f, 0.82f, 1.0f); // Verde claro

    // Inicializar o caminho com base no tamanho da janela
    updatePath();

    // Inicializar renderização moderna
    initOpenGL();

    // Inicializar jogo
    initGame();

    std::cout << "=== Tower Defense ===" << std::endl;
    std::cout << "Controles:" << std::endl;
    std::cout << "1 - Selecionar Torre Básica" << std::endl;
    std::cout << "2 - Selecionar Torre Avançada" << std::endl;
    std::cout << "Espaço - Iniciar Onda" << std::endl;
    std::cout << "R - Reiniciar (quando game over)" << std::endl;
    std::cout << "ESC - Cancelar seleção de torre" << std::endl;
    std::cout << "Clique para colocar torre selecionada" << std::endl;
    std::cout << std::endl;
    std::cout << "Resolução da janela: " << WINDOW_WIDTH << "x" << WINDOW_HEIGHT << std::endl;
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

    // Configurar VAO e VBO para formas
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Não precisa de glBufferData aqui, será feito em cada draw call
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Inicializar renderização de texto (já faz seu próprio shader, VAO, VBO)
    initTextRendering();

    // Definir cor de fundo inicial (será sobrescrita em render())
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
}

// Implementação das funções que estavam faltando
void drawPath() {
    for (size_t i = 0; i < path.size() - 1; ++i) {
        drawLine(path[i], path[i+1], COLOR_PATH, 5.0f); // Usar nova cor e espessura
    }
}

void drawEnemy(const Enemy& enemy) {
    drawCircle(enemy.position.x, enemy.position.y, enemy.radius, enemy.color);
    // Barra de vida
    float healthPercentage = enemy.health / enemy.maxHealth;
    float barWidth = enemy.radius * 1.5f;
    float barHeight = 5.0f;
    float barX = enemy.position.x - barWidth / 2.0f;
    float barY = enemy.position.y + enemy.radius + 5.0f; // Acima do inimigo

    drawRectangle(barX, barY, barWidth, barHeight, Color(0.2f, 0.2f, 0.2f)); // Fundo da barra
    drawRectangle(barX, barY, barWidth * healthPercentage, barHeight, Color(0.0f, 1.0f, 0.0f)); // Vida atual
}

void drawTower(const Tower& tower) {
    float baseRadius = tower.radius * 1.1f; // Raio da base hexagonal um pouco maior
    float topRadius = tower.radius * 0.7f;  // Raio do círculo do topo
    float platformOffsetY = tower.radius * 0.3f; // Deslocamento Y para o topo

    // Cor da base (um pouco mais escura)
    Color baseColor = Color(tower.color.r * 0.8f, tower.color.g * 0.8f, tower.color.b * 0.8f, tower.color.a);
    // Cor do topo (cor original)
    Color topColor = tower.color;

    // Desenhar a base hexagonal
    drawHexagon(tower.position.x, tower.position.y - platformOffsetY / 2, baseRadius, baseColor);

    // Desenhar o topo circular (plataforma)
    // Simular uma pequena perspectiva deslocando um pouco para cima
    drawCircle(tower.position.x, tower.position.y + platformOffsetY, topRadius, topColor);
    
    // Desenhar um círculo interno menor no topo para detalhe
    drawCircle(tower.position.x, tower.position.y + platformOffsetY, topRadius * 0.6f, Color(topColor.r * 0.7f, topColor.g * 0.7f, topColor.b * 0.7f));


    // Desenhar alcance (círculo) se a torre estiver selecionada ou o mouse estiver sobre ela (simplificado aqui)
    // if (placingTowerType.empty() && /* mouse over tower, ou torre selecionada */ ) {
    //     // ... código para desenhar alcance ...
    // }
}

void drawProjectile(const Projectile& projectile) {
    // Projétil com um pequeno brilho/rastro (simulado com dois círculos)
    drawCircle(projectile.position.x, projectile.position.y, projectile.radius, projectile.color);
    drawCircle(projectile.position.x, projectile.position.y, projectile.radius * 0.5f, Color(1.0f, 1.0f, 1.0f, 0.8f)); // Miolo branco
}

void drawUI() {
    float uiMargin = 20.0f;
    float lineHeight = 30.0f; // Aumentado de 25.0f para 30.0f
    float currentY = WINDOW_HEIGHT - uiMargin - lineHeight;
    float scale = 0.8f; // Aumentado de 0.45f para 0.8f para textos maiores

    // Informações do Jogador (Canto Superior Esquerdo)
    std::stringstream moneyStream, livesStream, waveStream;
    moneyStream << std::fixed << std::setprecision(0) << money;
    RenderText("Dinheiro: " + moneyStream.str(), uiMargin, currentY, scale, COLOR_TEXT_UI);
    currentY -= lineHeight;
    RenderText("Vidas: " + std::to_string(lives), uiMargin, currentY, scale, COLOR_TEXT_UI);
    currentY -= lineHeight;
    RenderText("Onda: " + std::to_string(currentWave), uiMargin, currentY, scale, COLOR_TEXT_UI);

    // Botões de Torre (Inferior Central)
    float buttonWidth = 180.0f; // Aumentado de 150.0f para 180.0f
    float buttonHeight = 50.0f; // Aumentado de 40.0f para 50.0f
    float buttonSpacing = 25.0f; // Aumentado de 20.0f para 25.0f
    float totalButtonWidth = (towerTypes.size() * buttonWidth) + ((towerTypes.size() - 1) * buttonSpacing);
    float startX = (WINDOW_WIDTH - totalButtonWidth) / 2.0f;
    float buttonY = uiMargin + buttonHeight; // Posição Y a partir da base
    
    // Calcular textY fora do loop para que esteja disponível para todos os botões
    float textY = buttonY - buttonHeight / 2.0f - (Characters['A'].Size.y * scale / 3.0f); // Ajuste para centralizar

    int i = 0;
    for (const auto& pair : towerTypes) {
        float currentButtonX = startX + i * (buttonWidth + buttonSpacing);
        Color buttonColor = (placingTowerType == pair.first) ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_NORMAL;
        
        drawRectangle(currentButtonX, buttonY - buttonHeight, buttonWidth, buttonHeight, buttonColor); // Y ajustado para desenhar para cima
        
        std::string buttonText = pair.second.name + " ($ " + std::to_string(static_cast<int>(pair.second.cost)) + ")";
        // Centralizar texto no botão
        RenderText(buttonText, currentButtonX + 15.0f, textY, scale, COLOR_TEXT_UI);
        i++;
    }
    
    // Botão Iniciar Onda (Canto Inferior Direito)
    if (!waveInProgress && !gameOver) {
        float startWaveButtonX = WINDOW_WIDTH - buttonWidth - uiMargin;
        drawRectangle(startWaveButtonX, buttonY - buttonHeight, buttonWidth, buttonHeight, COLOR_BUTTON_NORMAL);
        RenderText("Iniciar Onda (Espaço)", startWaveButtonX + 10.0f, textY, scale, COLOR_TEXT_UI);
    }

    // Mensagem de Feedback (Centralizada na parte superior)
    if (feedbackTimer > 0.0f) {
        feedbackTimer -= 1.0f / 60.0f; // Assumindo 60 FPS, ajuste se necessário
        float feedbackTextWidth = 0; // Precisaria de uma função para medir o texto
        // Simulação da largura do texto para centralização
        for(char c : currentFeedback) feedbackTextWidth += (Characters[c].Advance >> 6) * scale;

        float feedbackBgWidth = feedbackTextWidth + 60.0f; // Aumentado de 40.0f para 60.0f
        float feedbackBgHeight = 50.0f; // Aumentado de 40.0f para 50.0f
        float feedbackBgX = (WINDOW_WIDTH - feedbackBgWidth) / 2.0f;
        float feedbackBgY = WINDOW_HEIGHT - uiMargin - feedbackBgHeight;
        
        drawRectangle(feedbackBgX, feedbackBgY, feedbackBgWidth, feedbackBgHeight, COLOR_FEEDBACK_BG);
        RenderText(currentFeedback, feedbackBgX + 30.0f, feedbackBgY + (feedbackBgHeight / 2.0f) - (Characters['A'].Size.y * scale / 3.0f), scale, COLOR_TEXT_UI);
    }

    // Mensagem de Game Over (Centralizada)
    if (gameOver) {
        float gameOverBgWidth = 500.0f; // Aumentado de 400.0f para 500.0f
        float gameOverBgHeight = 250.0f; // Aumentado de 200.0f para 250.0f
        float gameOverBgX = (WINDOW_WIDTH - gameOverBgWidth) / 2.0f;
        float gameOverBgY = (WINDOW_HEIGHT - gameOverBgHeight) / 2.0f;

        drawRectangle(gameOverBgX, gameOverBgY, gameOverBgWidth, gameOverBgHeight, COLOR_GAMEOVER_BG);
        
        float textScaleLarge = 1.2f; // Aumentado de 0.8f para 1.2f
        float textScaleMedium = 0.9f; // Aumentado de 0.5f para 0.9f
        
        std::string gameOverText = "Fim de Jogo!";
        float textX = gameOverBgX + (gameOverBgWidth - gameOverText.length() * 15 * textScaleLarge) / 2.0f; // Aproximação da largura
        float textY = gameOverBgY + gameOverBgHeight - 70.0f; // Ajustado de 60.0f para 70.0f
        RenderText(gameOverText, textX, textY, textScaleLarge, Color(1.0f, 0.3f, 0.3f));

        std::string waveReachedText = "Você alcançou a onda: " + std::to_string(currentWave);
        textX = gameOverBgX + (gameOverBgWidth - waveReachedText.length() * 15 * textScaleMedium) / 2.0f;
        textY -= 60.0f; // Aumentado de 50.0f para 60.0f
        RenderText(waveReachedText, textX, textY, textScaleMedium, COLOR_TEXT_UI);

        std::string restartText = "Pressione R para reiniciar";
        textX = gameOverBgX + (gameOverBgWidth - restartText.length() * 15 * textScaleMedium) / 2.0f;
        textY -= 50.0f; // Aumentado de 40.0f para 50.0f
        RenderText(restartText, textX, textY, textScaleMedium, COLOR_TEXT_UI);
    }
}