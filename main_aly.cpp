#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float M_PI = 3.14159265358979323846f;

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

void beginRender() {
    glUseProgram(shaderProgram);
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
    if (projLoc == -1)
        std::cerr << "Uniform 'projection' não encontrado.\n";
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
}

void drawCircle(float x, float y, float radius, Color color) {
    const int segments = 32;
    static std::vector<float> vertices;
    vertices.clear();
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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool isTooCloseToPath(float x, float y, const std::vector<Point>& path) {
    const float MIN_DIST = 30.0f;
    for (int i = 0; i < path.size() - 1; ++i) {
        Point p1 = path[i], p2 = path[i + 1];
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float len2 = dx * dx + dy * dy;
        if (len2 == 0) continue;
        float t = ((x - p1.x) * dx + (y - p1.y) * dy) / len2;
        t = std::max(0.0f, std::min(1.0f, t));
        float projX = p1.x + t * dx;
        float projY = p1.y + t * dy;
        float dist = std::sqrt((x - projX) * (x - projX) + (y - projY) * (y - projY));
        if (dist < MIN_DIST) return true;
    }
    return false;
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
void main() {
    FragColor = color;
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
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
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

    std::vector<Point> path = {
        {50, 300}, {200, 300}, {200, 100}, {600, 100}, {600, 400}, {750, 400}
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.82f, 0.88f, 0.82f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        beginRender();

        drawCircle(400, 300, 40.0f, Color(0.2f, 0.6f, 1.0f, 1.0f));  // Exemplo de torre

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}