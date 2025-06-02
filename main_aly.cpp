#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float M_PI = 3.14159265358979323846f;
constexpr float ENEMY_RADIUS = 15.0f * 4.0f; // Aumentado 4x
constexpr float ENEMY_SPEED = 0.8f / 3.0f;   // Diminuído 1/3
constexpr float TOWER_RADIUS = 20.0f;
constexpr float TOWER_RANGE = 120.0f;
constexpr float PROJECTILE_SPEED = 3.0f; // Velocidade reduzida para 1/3 do padrão

struct Point {
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Color {
    float r, g, b, a;
    Color(float r = 1, float g = 1, float b = 1, float a = 1) : r(r), g(g), b(b), a(a) {}
};

unsigned int shaderProgram;
unsigned int VBO, VAO;

std::vector<Point> path = {
    {0, WINDOW_HEIGHT / 2.0f}, {150, WINDOW_HEIGHT / 2.0f}, {150, 100},
    {400, 100}, {400, 400}, {650, 400}, {650, WINDOW_HEIGHT / 2.0f},
    {WINDOW_WIDTH, WINDOW_HEIGHT / 2.0f}
};

struct Enemy {
    Point position;
    int pathIndex;
    float health;
    float radius;
    bool alive;

    Enemy() : position(path[0]), pathIndex(0), health(100), radius(ENEMY_RADIUS), alive(true) {}

    void update() {
        if (!alive || pathIndex >= path.size() - 1) return;
        Point target = path[pathIndex + 1];
        float dx = target.x - position.x;
        float dy = target.y - position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < ENEMY_SPEED) {
            position = target;
            pathIndex++;
        } else {
            position.x += (dx / dist) * ENEMY_SPEED;
            position.y += (dy / dist) * ENEMY_SPEED;
        }
    }
};

struct Tower {
    Point position;
    float cooldown;
    Tower(float x, float y) : position(x, y), cooldown(0) {}
};

struct Projectile {
    Point position, target;
    bool active;

    Projectile(Point start, Point end) : position(start), target(end), active(true) {}

    void update() { if (!active) return;
        float dx = target.x - position.x;
        float dy = target.y - position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < PROJECTILE_SPEED) {
            position = target;
            active = false;
        } else {
            position.x += (dx / dist) * PROJECTILE_SPEED;
            position.y += (dy / dist) * PROJECTILE_SPEED;
        }
    }
};

std::vector<Enemy> enemies;
std::vector<Tower> towers = {
    Tower(250, 250), Tower(350, 150), Tower(550, 250), Tower(700, 350)
};
std::vector<Projectile> projectiles;

void beginRender() {
    glUseProgram(shaderProgram);
    float projection[16] = {
        2.0f / WINDOW_WIDTH, 0, 0, 0,
        0, 2.0f / WINDOW_HEIGHT, 0, 0,
        0, 0, -1, 0,
        -1, -1, 0, 1
    };
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
}

void drawCircle(float x, float y, float radius, Color color) {
    const int segments = 32;
    std::vector<float> vertices = {x, y};
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(x + std::cos(angle) * radius);
        vertices.push_back(y + std::sin(angle) * radius);
    }
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() { FragColor = color;
}
)";

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Erro de compilação de shader:\n" << infoLog << "\n";
    }
    return shader;
}

void createShaderProgram() {
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512]; glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Erro ao linkar shader program:\n" << infoLog << "\n";
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

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

int main() {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tower Defense AFD", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD\n";
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    initOpenGL();

    for (int i = 0; i < 3; ++i)
        enemies.emplace_back();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        for (auto& e : enemies) e.update();
        for (auto& p : projectiles) p.update();

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
            [](Projectile& p) { return !p.active; }), projectiles.end());

        for (auto& p : projectiles) {
            for (auto& e : enemies) {
                if (!e.alive) continue;
                float dx = p.position.x - e.position.x;
                float dy = p.position.y - e.position.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < e.radius) { e.health -= 50;
                    e.radius *= 0.5f;
                    if (e.health <= 0) e.alive = false;
                    p.active = false;
                    break;
                }
            }
        }
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
            [](Enemy& e) { return !e.alive; }), enemies.end());

        for (auto& tower : towers) {
            tower.cooldown -= 1.0f;
            if (tower.cooldown <= 0) {
                for (auto& e : enemies) {
                    if (!e.alive) continue;
                    float dx = tower.position.x - e.position.x;
                    float dy = tower.position.y - e.position.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist <= TOWER_RANGE) {
                        projectiles.emplace_back(tower.position, e.position);
                        tower.cooldown = 60.0f;
                        break;
                    }
                }          }
        }

        glClearColor(0.82f, 0.88f, 0.82f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        beginRender();

        for (auto& tower : towers) {
            drawCircle(tower.position.x, tower.position.y, TOWER_RADIUS, Color(0.2f, 0.2f, 1.0f, 1.0f));
            drawCircle(tower.position.x, tower.position.y, TOWER_RANGE, Color(0.2f, 0.2f, 1.0f, 0.1f));
        }

        for (auto& p : projectiles)
            drawCircle(p.position.x, p.position.y, 6.0f, Color(1.0f, 1.0f, 0.0f, 1.0f));

        for (auto& e : enemies)
            drawCircle(e.position.x, e.position.y, e.radius, Color(1.0f, 0.2f, 0.2f, 1.0f));

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}