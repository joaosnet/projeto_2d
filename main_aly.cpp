#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr float M_PI = 3.14159265358979323846f;
constexpr float ENEMY_RADIUS = 7.5f;
constexpr float ENEMY_SPEED = 0.267f; // 1/3 da velocidade anterior
constexpr float TOWER_RADIUS = 20.0f;
constexpr float TOWER_RANGE = 120.0f;
constexpr float PROJECTILE_SPEED = 2.0f; // 1/3 da velocidade original
constexpr int MAX_TOWERS = 4;
constexpr float MIN_TOWER_DISTANCE = 120.0f;
constexpr float MAX_TOWER_DISTANCE = 240.0f;
constexpr float PATH_WIDTH = 30.0f;

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
    {0, 360}, {200, 360}, {200, 100}, {500, 100}, {500, 600}, {1000, 600}, {1000, 360}, {1280, 360}
};

struct Enemy {
    Point position;
    int pathIndex;
    float health;
    float size;

    Enemy() : position(path[0]), pathIndex(0), health(100.0f), size(ENEMY_RADIUS) {}

    void update() {
        if (health <= 0) return;
        if (pathIndex < path.size() - 1) {
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
    }
};

struct Projectile {
    Point position;
    Enemy* target;

    bool update() {
        if (!target || target->health <= 0) return true;
        float dx = target->position.x - position.x;
        float dy = target->position.y - position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 5.0f) {
            target->health -= 50;
            target->size *= 0.5f;
            return true;
        }
        position.x += (dx / dist) * PROJECTILE_SPEED;
        position.y += (dy / dist) * PROJECTILE_SPEED;
        return false;
    }
};

struct Tower {
    Point position;
    Enemy* currentTarget = nullptr;
    float lastShotTime = 0.0f;

    Tower(float x, float y) : position(x, y) {}

    void update(std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles, float time) {
        if (currentTarget && (currentTarget->health <= 0 ||
            std::hypot(currentTarget->position.x - position.x, currentTarget->position.y - position.y) > TOWER_RANGE)) {
            currentTarget = nullptr;
        }
        if (!currentTarget) {
            for (auto& e : enemies) {
                if (e.health > 0 && std::hypot(e.position.x - position.x, e.position.y - position.y) <= TOWER_RANGE) {
                    currentTarget = &e;
                    break;
                }
            }
        }
        if (currentTarget && time - lastShotTime > 0.5f) {
            projectiles.push_back({ position, currentTarget });
            lastShotTime = time;
        }
    }
};

std::vector<Enemy> enemies;
std::vector<Tower> towers;
std::vector<Projectile> projectiles;
std::vector<Point> towerSpots = {
    {300, 200}, {600, 250}, {800, 500}, {1100, 200}
};
int currentWave = 0;
bool waveInProgress = false;
float waveStartTime = 0;
float gameTime = 0.0f;
int fibonacci[100] = { 1, 1 };

void beginRender() {
    glUseProgram(shaderProgram);
    float left = 0.0f, right = (float)WINDOW_WIDTH;
    float bottom = 0.0f, top = (float)WINDOW_HEIGHT;
    float near = -1.0f, far = 1.0f;
    float projection[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f, 0.0f, 0.0f, -2.0f / (far - near), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
    };
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
}

void drawCircle(float x, float y, float radius, Color color) {
    const int segments = 32;
    std::vector<float> vertices;
    vertices.push_back(x);
    vertices.push_back(y);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(x + cosf(angle) * radius);
        vertices.push_back(y + sinf(angle) * radius);
    }
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW); glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);
}

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
})";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
})";

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void createShaderProgram() {
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
}

void initOpenGL() { createShaderProgram();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

bool isOnPath(float x, float y) {
    for (size_t i = 0; i < path.size() - 1; ++i) {
        float dx = path[i + 1].x - path[i].x;
        float dy = path[i + 1].y - path[i].y;
        float length = std::sqrt(dx * dx + dy * dy);
        float t = ((x - path[i].x) * dx + (y - path[i].y) * dy) / (length * length);
        t = std::fmax(0.0f, std::fmin(1.0f, t));
        float projX = path[i].x + t * dx;
        float projY = path[i].y + t * dy;
        if (std::hypot(projX - x, projY - y) < PATH_WIDTH) return true;
    }
    return false;
}

int main() {
    for (int i = 2; i < 100; ++i)
        fibonacci[i] = fibonacci[i - 1] + fibonacci[i - 2];

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tower Defense", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    initOpenGL();

    auto start = std::chrono::high_resolution_clock::now();
    glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && towers.size() < MAX_TOWERS) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            my = WINDOW_HEIGHT - my;
            if (!isOnPath(mx, my)) {
                bool valid = true;
                for (auto& t : towers)
                    if (std::hypot(t.position.x - mx, t.position.y - my) < MIN_TOWER_DISTANCE)
                        valid = false;
                if (valid)
                    towers.emplace_back(mx, my);
            }
        }
    });

    while (!glfwWindowShouldClose(window)) {
        auto now = std::chrono::high_resolution_clock::now();
        gameTime = std::chrono::duration<float>(now - start).count();

        glfwPollEvents();

        if (!waveInProgress && enemies.empty()) {
            if (gameTime - waveStartTime >= 10.0f) {
                for (int i = 0; i < fibonacci[currentWave]; ++i)
                    enemies.emplace_back();
                currentWave++;
                waveInProgress = true;
            }
        }

        bool anyAlive = false;
        for (auto& e : enemies) {
            e.update();
            if (e.health > 0) anyAlive = true;
        }
        if (!anyAlive && waveInProgress) {
            enemies.clear();
            waveStartTime = gameTime;
            waveInProgress = false;
        }

        for (auto& t : towers)
            t.update(enemies, projectiles, gameTime);

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
            [](Projectile& p) { return p.update(); }), projectiles.end());

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        beginRender();

        for (size_t i = 0; i < path.size() - 1; ++i) {
            Point p1 = path[i], p2 = path[i + 1];
            for (float t = 0; t <= 1.0f; t += 0.02f)
                drawCircle(p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y), PATH_WIDTH / 2.0f, Color(1.0f, 1.0f, 0.0f, 0.3f));
        }

        for (auto& s : towerSpots)
            drawCircle(s.x, s.y, 10.0f, Color(0.0f, 0.4f, 1.0f, 0.7f));

        for (auto& t : towers)
            drawCircle(t.position.x, t.position.y, TOWER_RADIUS, Color(0.2f, 0.2f, 1.0f, 1.0f));

        for (auto& e : enemies)
            if (e.health > 0)
                drawCircle(e.position.x, e.position.y, e.size, Color(1.0f, 0.2f, 0.2f, 1.0f));

        for (auto& p : projectiles)
            drawCircle(p.position.x, p.position.y, 5.0f, Color(1.0f, 1.0f, 1.0f, 1.0f)); glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}