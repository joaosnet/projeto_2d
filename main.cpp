#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ================================
// CONFIGURAÇÕES DO JOGO
// ================================
namespace Config {
    // Janela e mundo
    const float WINDOW_WIDTH = 1000.0f;
    const float WINDOW_HEIGHT = 800.0f;
    const float WORLD_WIDTH = 2.0f;
    const float WORLD_HEIGHT = 1.6f;
    
    // Física
    const float PLAYER_THRUST = 3.0f;
    const float PLAYER_ROTATION_SPEED = 5.0f;
    const float PLAYER_MAX_SPEED = 1.0f;
    const float BULLET_SPEED = 2.0f;
    const float BULLET_LIFETIME = 3.0f;
    const float BULLET_COOLDOWN_NORMAL = 0.2f;
    const float BULLET_COOLDOWN_RAPID = 0.08f;
    
    // Spawn rates
    const float ASTEROID_BASE_SPAWN_RATE = 2.0f;
    const float ENEMY_BASE_SPAWN_RATE = 8.0f;
    const float POWERUP_DROP_CHANCE = 0.15f;
    
    // Power-ups
    const float POWERUP_DURATION = 8.0f;
    const float POWERUP_LIFETIME = 15.0f;
    
    // Pontuação
    const int SCORE_ASTEROID = 10;
    const int SCORE_ENEMY = 25;
    const int SCORE_POWERUP = 50;
    const int SCORE_WAVE_BONUS = 100;
    
    // Dificuldade
    const float DIFFICULTY_INCREASE_RATE = 0.1f;
    const float WAVE_DURATION = 30.0f;
    
    // Visuais
    const int BACKGROUND_STARS = 150;
    const int EXPLOSION_PARTICLES = 12;
    const int THRUST_PARTICLE_INTERVAL = 0.03f;
}

// Enums para tipos de objetos
enum ObjectType {
    PLAYER_SHIP,
    ASTEROID,
    ENEMY,
    POWERUP,
    BULLET,
    PARTICLE,
    STAR
};

// Novos tipos de power-ups
enum PowerUpType {
    RAPID_FIRE = 0,
    SHIELD = 1,
    MULTI_SHOT = 2,
    SPEED_BOOST = 3,
    LIFE_UP = 4
};

// Estrutura para representar um objeto 2D do jogo
struct GameObject {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
    
    // Transformações geométricas
    float posX, posY;           // Posição (translação)
    float rotation;             // Rotação em radianos
    float scaleX, scaleY;       // Escala
    
    // Física e movimento
    float velX, velY;           // Velocidade
    float angularVel;           // Velocidade angular
    float friction;             // Atrito
    
    // Propriedades visuais
    float colorR, colorG, colorB; // Cor
    float alpha;                  // Transparência
    float pulseSpeed;            // Velocidade de pulsação
    float pulseOffset;           // Offset da pulsação
    
    // Propriedades do jogo
    ObjectType type;
    bool active;                // Se o objeto está ativo
    float lifetime;             // Tempo de vida (-1 = infinito)
    float radius;               // Raio para colisão
    
    // Power-up específico
    int powerType;              // Tipo de power-up
    
    GameObject() : posX(0.0f), posY(0.0f), rotation(0.0f), 
                   scaleX(1.0f), scaleY(1.0f),
                   velX(0.0f), velY(0.0f), angularVel(0.0f), friction(0.98f),
                   colorR(1.0f), colorG(1.0f), colorB(1.0f), alpha(1.0f),
                   pulseSpeed(1.0f), pulseOffset(0.0f),
                   type(PLAYER_SHIP), active(true), lifetime(-1.0f), radius(0.1f),
                   powerType(0) {}
};

// Estado do jogo melhorado
struct GameState {
    int score;
    int lives;
    int level;
    int wave;
    float timeElapsed;
    float waveStartTime;
    bool gameOver;
    bool paused;
    float lastAsteroidSpawn;
    float lastEnemySpawn;
    float difficulty;
    
    // Power-ups ativos (expandido)
    bool rapidFire;
    float rapidFireTime;
    bool shield;
    float shieldTime;
    bool multiShot;
    float multiShotTime;
    bool speedBoost;
    float speedBoostTime;
    
    // Estatísticas
    int asteroidsDestroyed;
    int enemiesDestroyed;
    int powerupsCollected;
    
    GameState() : score(0), lives(3), level(1), wave(1), timeElapsed(0.0f), waveStartTime(0.0f),
                  gameOver(false), paused(false), lastAsteroidSpawn(0.0f),
                  lastEnemySpawn(0.0f), difficulty(1.0f),
                  rapidFire(false), rapidFireTime(0.0f),
                  shield(false), shieldTime(0.0f),
                  multiShot(false), multiShotTime(0.0f),
                  speedBoost(false), speedBoostTime(0.0f),
                  asteroidsDestroyed(0), enemiesDestroyed(0), powerupsCollected(0) {}
};

// Variáveis globais
std::vector<GameObject> gameObjects;
GameState gameState;
int playerIndex = 0;
float lastBulletTime = 0.0f;
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

// Sistema de partículas
std::vector<GameObject> particles;

// Gerador de números aleatórios
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> randomFloat(0.0f, 1.0f);

// Declarações forward das funções
GameObject createParticle(float x, float y, float r, float g, float b);
void setupObject(GameObject& obj);
void createThrustParticles(const GameObject& player);
GameObject createBullet();
void createExplosion(float x, float y, float r, float g, float b, int numParticles = 8);
GameObject createPlayerShip();
GameObject createAsteroid();
GameObject createEnemy();
GameObject createPowerUp();
GameObject createStar();
void initializeGame(); // Forward declaration para uso no reinício
void createVisualsForWrapping(const GameObject& obj); // Nova função para efeitos visuais de wrap

// Função de callback para redimensionamento da janela
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Funções utilitárias
float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

bool checkCollision(const GameObject& a, const GameObject& b) {
    return distance(a.posX, a.posY, b.posX, b.posY) < (a.radius + b.radius);
}

// Função para teletransportar o objeto para o lado oposto da tela
void wrapPosition(GameObject& obj) {
    const float buffer = 0.01f; 
    if (obj.posX - obj.radius > Config::WORLD_WIDTH / 2) obj.posX = -Config::WORLD_WIDTH / 2 - obj.radius + buffer;
    if (obj.posX + obj.radius < -Config::WORLD_WIDTH / 2) obj.posX = Config::WORLD_WIDTH / 2 + obj.radius - buffer;
    if (obj.posY - obj.radius > Config::WORLD_HEIGHT / 2) obj.posY = -Config::WORLD_HEIGHT / 2 - obj.radius + buffer;
    if (obj.posY + obj.radius < -Config::WORLD_HEIGHT / 2) obj.posY = Config::WORLD_HEIGHT / 2 + obj.radius - buffer;
}

// Utilitários para renderização de texto/HUD
struct TextRenderer {
    static std::string formatScore(int score) {
        return "Score: " + std::to_string(score);
    }
    
    static std::string formatLives(int lives) {
        return "Lives: " + std::to_string(lives);
    }
    
    static std::string formatWave(int wave) {
        return "Wave: " + std::to_string(wave);
    }
    
    static std::string formatPowerUp(const std::string& name, float timeLeft) {
        return name + ": " + std::to_string(static_cast<int>(timeLeft + 1)) + "s";
    }
};

// Função para criar efeito de borda da tela (wraparound visual)
// Esta função foi removida e substituída por createVisualsForWrapping e wrapPosition
// void createWrapEffect(GameObject& obj) { ... } // REMOVIDA

// Nova função para criar partículas visuais quando um objeto está prestes a fazer o wrap
void createVisualsForWrapping(const GameObject& obj) {
    if (obj.type != PLAYER_SHIP && obj.type != ASTEROID && obj.type != ENEMY) return;

    float particleLifetime = 0.3f;
    float particleSpeedFactor = 0.1f;
    float edgeProximity = obj.radius * 0.8f; 

    // Saindo pela direita
    if (obj.velX > 0 && (obj.posX + edgeProximity) > (Config::WORLD_WIDTH / 2)) {
        GameObject p = createParticle(Config::WORLD_WIDTH / 2, obj.posY, obj.colorR, obj.colorG, obj.colorB);
        p.lifetime = particleLifetime;
        p.velX = (randomFloat(gen) - 1.0f) * particleSpeedFactor - obj.velX * 0.1f;
        p.velY = (randomFloat(gen) - 0.5f) * particleSpeedFactor * 2.0f;
        setupObject(p);
        particles.push_back(p);
    }
    // Saindo pela esquerda
    else if (obj.velX < 0 && (obj.posX - edgeProximity) < (-Config::WORLD_WIDTH / 2)) {
        GameObject p = createParticle(-Config::WORLD_WIDTH / 2, obj.posY, obj.colorR, obj.colorG, obj.colorB);
        p.lifetime = particleLifetime;
        p.velX = (randomFloat(gen) + 0.0f) * particleSpeedFactor - obj.velX * 0.1f;
        p.velY = (randomFloat(gen) - 0.5f) * particleSpeedFactor * 2.0f;
        setupObject(p);
        particles.push_back(p);
    }

    // Saindo por cima
    if (obj.velY > 0 && (obj.posY + edgeProximity) > (Config::WORLD_HEIGHT / 2)) {
        GameObject p = createParticle(obj.posX, Config::WORLD_HEIGHT / 2, obj.colorR, obj.colorG, obj.colorB);
        p.lifetime = particleLifetime;
        p.velY = (randomFloat(gen) - 1.0f) * particleSpeedFactor - obj.velY * 0.1f;
        p.velX = (randomFloat(gen) - 0.5f) * particleSpeedFactor * 2.0f;
        setupObject(p);
        particles.push_back(p);
    }
    // Saindo por baixo
    else if (obj.velY < 0 && (obj.posY - edgeProximity) < (-Config::WORLD_HEIGHT / 2)) {
        GameObject p = createParticle(obj.posX, -Config::WORLD_HEIGHT / 2, obj.colorR, obj.colorG, obj.colorB);
        p.lifetime = particleLifetime;
        p.velY = (randomFloat(gen) + 0.0f) * particleSpeedFactor - obj.velY * 0.1f;
        p.velX = (randomFloat(gen) - 0.5f) * particleSpeedFactor * 2.0f;
        setupObject(p);
        particles.push_back(p);
    }
}

// Função para processar input do jogo (melhorada)
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Pausar jogo
    static bool pausePressed = false;
    if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pausePressed) {
        gameState.paused = !gameState.paused;
        pausePressed = true;
    }
    if(glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pausePressed = false;
    }
    
    // Reiniciar jogo se game over
    static bool restartPressed = false;
    if(gameState.gameOver && glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !restartPressed) {
        gameState = GameState();
        gameObjects.clear();
        particles.clear();
        initializeGame(); 
        restartPressed = true;
    }
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        restartPressed = false;
    }
    
    if (gameState.paused || gameState.gameOver) return;

    if (gameObjects.empty() || playerIndex < 0 || playerIndex >= gameObjects.size() || !gameObjects[playerIndex].active) {
        return;
    }
    
    GameObject& player = gameObjects[playerIndex];
    
    // Controles da nave (usando constantes configuráveis)
    float thrust = Config::PLAYER_THRUST * deltaTime;
    float rotSpeed = Config::PLAYER_ROTATION_SPEED * deltaTime;
    float maxSpeed = Config::PLAYER_MAX_SPEED;
    
    // Aplicar speed boost se ativo
    if (gameState.speedBoost) {
        thrust *= 1.5f;
        maxSpeed *= 1.3f;
    }
    
    // Rotação
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        player.rotation += rotSpeed;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        player.rotation -= rotSpeed;
    
    // Propulsão
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        player.velX += cos(player.rotation) * thrust;
        player.velY += sin(player.rotation) * thrust;
        
        createThrustParticles(player);
        
        // Limitar velocidade máxima
        float speed = sqrt(player.velX * player.velX + player.velY * player.velY);
        if (speed > maxSpeed) {
            player.velX = (player.velX / speed) * maxSpeed;
            player.velY = (player.velY / speed) * maxSpeed;
        }
    }
    
    // Tiro (melhorado com multi-shot)
    float bulletCooldown = gameState.rapidFire ? Config::BULLET_COOLDOWN_RAPID : Config::BULLET_COOLDOWN_NORMAL;
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && 
       gameState.timeElapsed - lastBulletTime > bulletCooldown) {
        
        if (gameState.multiShot) {
            // Tiro triplo
            for (int i = -1; i <= 1; i++) {
                GameObject bullet = createBullet();
                bullet.posX = player.posX;
                bullet.posY = player.posY;
                bullet.rotation = player.rotation + i * 0.3f; // Spread de 0.3 radianos
                bullet.velX = cos(bullet.rotation) * Config::BULLET_SPEED + player.velX * 0.5f;
                bullet.velY = sin(bullet.rotation) * Config::BULLET_SPEED + player.velY * 0.5f;
                setupObject(bullet);
                gameObjects.push_back(bullet);
            }
        } else {
            // Tiro normal
            GameObject bullet = createBullet();
            bullet.posX = player.posX;
            bullet.posY = player.posY;
            bullet.rotation = player.rotation;
            bullet.velX = cos(player.rotation) * Config::BULLET_SPEED + player.velX * 0.5f;
            bullet.velY = sin(player.rotation) * Config::BULLET_SPEED + player.velY * 0.5f;
            setupObject(bullet);
            gameObjects.push_back(bullet);
        }
        
        lastBulletTime = gameState.timeElapsed;
    }
}

// Função para criar a nave do jogador (triângulo)
GameObject createPlayerShip() {
    GameObject ship;
    ship.vertices = {
        // Nave espacial estilizada
         0.0f,  0.08f, 0.0f,  // ponta
        -0.05f, -0.05f, 0.0f,  // asa esquerda
         0.05f, -0.05f, 0.0f,  // asa direita
         0.0f, -0.02f, 0.0f   // motor central
    };
    ship.indices = {0, 1, 3, 0, 3, 2}; // Dois triângulos formando a nave
    ship.colorR = 0.2f; ship.colorG = 0.8f; ship.colorB = 1.0f; // Azul ciano
    ship.type = PLAYER_SHIP;
    ship.radius = 0.06f;
    ship.friction = 0.98f;
    return ship;
}

// Função para criar um asteroide (hexágono irregular)
GameObject createAsteroid() {
    GameObject asteroid;
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
    
    // Índices para formar triângulos
    for(int i = 0; i < sides; i++) {
        asteroid.indices.push_back(0);
        asteroid.indices.push_back(i + 1);
        asteroid.indices.push_back((i + 1) % sides + 1);
    }
    
    asteroid.colorR = 0.7f; asteroid.colorG = 0.4f; asteroid.colorB = 0.2f; // Marrom
    asteroid.type = ASTEROID;
    asteroid.radius = baseRadius;
    asteroid.angularVel = (randomFloat(gen) - 0.5f) * 2.0f;
    asteroid.friction = 1.0f; // Sem atrito para asteroides
    return asteroid;
}

// Função para criar um inimigo (quadrado)
GameObject createEnemy() {
    GameObject enemy;
    enemy.vertices = {
        -0.06f, -0.06f, 0.0f,
         0.06f, -0.06f, 0.0f,
         0.06f,  0.06f, 0.0f,
        -0.06f,  0.06f, 0.0f
    };
    enemy.indices = {0, 1, 2, 0, 2, 3};
    enemy.colorR = 1.0f; enemy.colorG = 0.2f; enemy.colorB = 0.2f; // Vermelho
    enemy.type = ENEMY;
    enemy.radius = 0.06f;
    enemy.friction = 0.99f;
    return enemy;
}

// Função para criar um power-up (melhorada com novos tipos)
GameObject createPowerUp() {
    GameObject powerup;
    powerup.vertices = {
         0.0f,  0.05f, 0.0f,  // topo
        -0.03f,  0.0f, 0.0f,  // esquerda
         0.0f, -0.05f, 0.0f,  // baixo
         0.03f,  0.0f, 0.0f   // direita
    };
    powerup.indices = {0, 1, 2, 0, 2, 3};
    
    // Diferentes tipos de power-up
    powerup.powerType = static_cast<int>(randomFloat(gen) * 5);
    switch (powerup.powerType) {
        case RAPID_FIRE:
            powerup.colorR = 1.0f; powerup.colorG = 1.0f; powerup.colorB = 0.0f; // Amarelo
            break;
        case SHIELD:
            powerup.colorR = 0.0f; powerup.colorG = 1.0f; powerup.colorB = 1.0f; // Ciano
            break;
        case MULTI_SHOT:
            powerup.colorR = 1.0f; powerup.colorG = 0.0f; powerup.colorB = 1.0f; // Magenta
            break;
        case SPEED_BOOST:
            powerup.colorR = 0.0f; powerup.colorG = 1.0f; powerup.colorB = 0.0f; // Verde
            break;
        case LIFE_UP:
            powerup.colorR = 1.0f; powerup.colorG = 0.0f; powerup.colorB = 0.0f; // Vermelho
            break;
    }
    
    powerup.type = POWERUP;
    powerup.radius = 0.04f;
    powerup.pulseSpeed = 3.0f;
    powerup.lifetime = Config::POWERUP_LIFETIME;
    return powerup;
}

// Função para criar uma bala (melhorada)
GameObject createBullet() {
    GameObject bullet;
    bullet.vertices = {
        -0.01f, -0.01f, 0.0f,
         0.01f, -0.01f, 0.0f,
         0.01f,  0.01f, 0.0f,
        -0.01f,  0.01f, 0.0f
    };
    bullet.indices = {0, 1, 2, 0, 2, 3};
    bullet.colorR = 1.0f; bullet.colorG = 1.0f; bullet.colorB = 1.0f; // Branco
    bullet.type = BULLET;
    bullet.radius = 0.01f;
    bullet.lifetime = Config::BULLET_LIFETIME;
    bullet.friction = 1.0f; // Sem atrito
    return bullet;
}

// Função para criar uma partícula de explosão
GameObject createParticle(float x, float y, float r, float g, float b) {
    GameObject particle;
    particle.vertices = {
        -0.005f, -0.005f, 0.0f,
         0.005f, -0.005f, 0.0f,
         0.005f,  0.005f, 0.0f,
        -0.005f,  0.005f, 0.0f
    };
    particle.indices = {0, 1, 2, 0, 2, 3};
    particle.colorR = r; particle.colorG = g; particle.colorB = b;
    particle.posX = x; particle.posY = y;
    particle.type = PARTICLE;
    particle.radius = 0.005f;
    particle.lifetime = 1.0f + randomFloat(gen) * 0.5f;
    particle.friction = 0.95f;
    
    // Velocidade aleatória
    float angle = randomFloat(gen) * 2.0f * M_PI;
    float speed = randomFloat(gen) * 0.5f + 0.1f;
    particle.velX = cos(angle) * speed;
    particle.velY = sin(angle) * speed;
    
    return particle;
}

// Função para criar uma estrela do fundo
GameObject createStar() {
    GameObject star;
    float size = 0.002f + randomFloat(gen) * 0.003f;
    star.vertices = {
        -size, -size, 0.0f,
         size, -size, 0.0f,
         size,  size, 0.0f,
        -size,  size, 0.0f
    };
    star.indices = {0, 1, 2, 0, 2, 3};
    
    float brightness = 0.3f + randomFloat(gen) * 0.7f;
    star.colorR = brightness; star.colorG = brightness; star.colorB = brightness;
    star.type = STAR;
    star.radius = size;
    star.pulseSpeed = 0.5f + randomFloat(gen) * 1.0f;
    star.pulseOffset = randomFloat(gen) * 2.0f * M_PI;
    return star;
}

// Função para configurar buffers de um objeto
void setupObject(GameObject& obj) {
    glGenVertexArrays(1, &obj.VAO);
    glGenBuffers(1, &obj.VBO);
    glGenBuffers(1, &obj.EBO);
    
    glBindVertexArray(obj.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, obj.VBO);
    glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(float), obj.vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size() * sizeof(unsigned int), obj.indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

// Função para criar partículas de explosão
void createExplosion(float x, float y, float r, float g, float b, int numParticles) {
    for (int i = 0; i < numParticles; i++) {
        GameObject particle = createParticle(x, y, r, g, b);
        setupObject(particle);
        particles.push_back(particle);
    }
}

// Função para criar efeito de trail da nave (melhorada)
void createThrustParticles(const GameObject& player) {
    static float lastThrustTime = 0.0f;
    if (gameState.timeElapsed - lastThrustTime > Config::THRUST_PARTICLE_INTERVAL) {
        // Posição atrás da nave
        float thrustX = player.posX - cos(player.rotation) * 0.08f;
        float thrustY = player.posY - sin(player.rotation) * 0.08f;
        
        // Criar múltiplas partículas para efeito mais denso
        for (int i = 0; i < 2; i++) {
            GameObject thrust = createParticle(thrustX, thrustY, 0.0f, 0.5f + randomFloat(gen) * 0.5f, 1.0f);
            thrust.lifetime = 0.2f + randomFloat(gen) * 0.2f;
            thrust.velX = -cos(player.rotation) * (0.15f + randomFloat(gen) * 0.1f) + (randomFloat(gen) - 0.5f) * 0.05f;
            thrust.velY = -sin(player.rotation) * (0.15f + randomFloat(gen) * 0.1f) + (randomFloat(gen) - 0.5f) * 0.05f;
            thrust.friction = 0.92f;
            setupObject(thrust);
            particles.push_back(thrust);
        }
        
        lastThrustTime = gameState.timeElapsed;
    }
}

// Função para spawnar asteroides
void spawnAsteroid() {
    GameObject asteroid = createAsteroid();
    
    // Spawnar na borda da tela
    float side = randomFloat(gen) * 4;
    if (side < 1) { // Topo
        asteroid.posX = (randomFloat(gen) - 0.5f) * Config::WORLD_WIDTH;
        asteroid.posY = Config::WORLD_HEIGHT / 2 + 0.1f;
        asteroid.velY = -randomFloat(gen) * 0.3f - 0.1f;
    } else if (side < 2) { // Direita
        asteroid.posX = Config::WORLD_WIDTH / 2 + 0.1f;
        asteroid.posY = (randomFloat(gen) - 0.5f) * Config::WORLD_HEIGHT;
        asteroid.velX = -randomFloat(gen) * 0.3f - 0.1f;
    } else if (side < 3) { // Baixo
        asteroid.posX = (randomFloat(gen) - 0.5f) * Config::WORLD_WIDTH;
        asteroid.posY = -Config::WORLD_HEIGHT / 2 - 0.1f;
        asteroid.velY = randomFloat(gen) * 0.3f + 0.1f;
    } else { // Esquerda
        asteroid.posX = -Config::WORLD_WIDTH / 2 - 0.1f;
        asteroid.posY = (randomFloat(gen) - 0.5f) * Config::WORLD_HEIGHT;
        asteroid.velX = randomFloat(gen) * 0.3f + 0.1f;
    }
    
    setupObject(asteroid);
    gameObjects.push_back(asteroid);
}

// Função para spawnar inimigos
void spawnEnemy() {
    GameObject enemy = createEnemy();
    
    // Spawnar longe do jogador
    GameObject& player = gameObjects[playerIndex];
    do {
        enemy.posX = (randomFloat(gen) - 0.5f) * Config::WORLD_WIDTH;
        enemy.posY = (randomFloat(gen) - 0.5f) * Config::WORLD_HEIGHT;
    } while (distance(enemy.posX, enemy.posY, player.posX, player.posY) < 0.3f);
    
    setupObject(enemy);
    gameObjects.push_back(enemy);
}

// Função para spawnar power-ups
void spawnPowerUp() {
    GameObject powerup = createPowerUp();
    powerup.posX = (randomFloat(gen) - 0.5f) * Config::WORLD_WIDTH;
    powerup.posY = (randomFloat(gen) - 0.5f) * Config::WORLD_HEIGHT;
    setupObject(powerup);
    gameObjects.push_back(powerup);
}

// Função para atualizar física dos objetos
void updateGameObjects() {
    // Verificar se há objetos e se o jogador existe para referências
    if (gameObjects.empty()) {
        return; // Nada para atualizar
    }
    // A referência ao jogador é usada pela IA do inimigo.
    // Se o jogador não estiver ativo/válido, a IA pode ter comportamento padrão ou não atualizar.
    // Assumindo que playerIndex é 0 e o jogador é o primeiro elemento.
    GameObject* player_ptr = nullptr;
    if (playerIndex < gameObjects.size() && gameObjects[playerIndex].type == PLAYER_SHIP && gameObjects[playerIndex].active) {
        player_ptr = &gameObjects[playerIndex];
    }
    
    // Atualizar posições e aplicar transformações
    for (auto& obj : gameObjects) {
        if (!obj.active) continue;
        
        // Atualizar posição baseada na velocidade
        obj.posX += obj.velX * deltaTime;
        obj.posY += obj.velY * deltaTime;
        
        // Atualizar rotação
        obj.rotation += obj.angularVel * deltaTime;
        
        // Aplicar atrito
        obj.velX *= obj.friction;
        obj.velY *= obj.friction;
        
        // Atualizar lifetime
        if (obj.lifetime > 0) {
            obj.lifetime -= deltaTime;
            if (obj.lifetime <= 0) {
                obj.active = false;
                // Se o objeto que expirou era o jogador (improvável com lifetime -1), tratar gameOver.
                // if (obj.type == PLAYER_SHIP) gameState.gameOver = true; // Exemplo, não aplicável aqui
                continue;
            }
        }
        
        // Comportamento específico por tipo
        switch (obj.type) {
            case PLAYER_SHIP:
                createVisualsForWrapping(obj); // Chamada ANTES de wrapPosition
                wrapPosition(obj);
                // A lógica de atualização de tempo dos power-ups foi movida para updateGame()
                break;
                
            case ASTEROID:
                createVisualsForWrapping(obj); // Chamada ANTES de wrapPosition
                wrapPosition(obj);
                break;
                
            case ENEMY:
                createVisualsForWrapping(obj); // Chamada ANTES de wrapPosition
                wrapPosition(obj);
                // IA simples: mover em direção ao jogador
                if (player_ptr) { // Somente se o jogador existir e estiver ativo
                    float dx = player_ptr->posX - obj.posX;
                    float dy = player_ptr->posY - obj.posY;
                    float dist = sqrt(dx * dx + dy * dy);
                    if (dist > 0) {
                        obj.velX += (dx / dist) * 0.5f * deltaTime;
                        obj.velY += (dy / dist) * 0.5f * deltaTime;
                    }
                }
                break;
                
            case BULLET:
                // Balas saem da tela
                if (obj.posX < -Config::WORLD_WIDTH/2 - 0.1f || obj.posX > Config::WORLD_WIDTH/2 + 0.1f ||
                    obj.posY < -Config::WORLD_HEIGHT/2 - 0.1f || obj.posY > Config::WORLD_HEIGHT/2 + 0.1f) {
                    obj.active = false;
                }
                break;
                  case POWERUP:
                // Animação de pulsação
                obj.scaleX = 1.0f + 0.2f * sin(gameState.timeElapsed * obj.pulseSpeed + obj.pulseOffset);
                obj.scaleY = obj.scaleX;
                break;
                
            case PARTICLE:
                // Partículas não fazem wraparound
                break;
                
            case STAR:
                // Efeito de cintilação
                obj.alpha = 0.3f + 0.7f * (0.5f + 0.5f * sin(gameState.timeElapsed * obj.pulseSpeed + obj.pulseOffset));
                break;
        }
    }
    
    // Atualizar partículas
    for (auto& particle : particles) {
        if (!particle.active) continue;
        
        particle.posX += particle.velX * deltaTime;
        particle.posY += particle.velY * deltaTime;
        particle.velX *= particle.friction;
        particle.velY *= particle.friction;
        
        if (particle.lifetime > 0) {
            particle.lifetime -= deltaTime;
            particle.alpha = particle.lifetime; // Fade out
            if (particle.lifetime <= 0) {
                particle.active = false;
            }
        }
    }
}

// Função para verificar colisões
void checkCollisions() {
    if (gameObjects.empty() || playerIndex < 0 || playerIndex >= gameObjects.size() || !gameObjects[playerIndex].active) {
        return; // Jogador não disponível para colisões
    }
    GameObject& player = gameObjects[playerIndex];
    
    for (size_t i = 0; i < gameObjects.size(); i++) { // Iterar usando size_t e verificar se é o jogador
        GameObject& obj = gameObjects[i];
        if (!obj.active || i == playerIndex) continue; // Pular objetos inativos ou o próprio jogador
        
        // Colisões com o jogador
        if (obj.type != BULLET && checkCollision(player, obj)) {
            switch (obj.type) {
                case ASTEROID:
                case ENEMY:
                    if (!gameState.shield) {
                        gameState.lives--;
                        createExplosion(player.posX, player.posY, 0.2f, 0.8f, 1.0f, 12);
                        if (gameState.lives <= 0) {
                            gameState.gameOver = true;
                        }
                        // Respawn do jogador
                        player.posX = 0.0f;
                        player.posY = 0.0f;
                        player.velX = 0.0f;
                        player.velY = 0.0f;
                    }
                    obj.active = false;
                    createExplosion(obj.posX, obj.posY, obj.colorR, obj.colorG, obj.colorB);
                    break;
                      case POWERUP:
                    // Ativar power-up baseado no tipo
                    gameState.powerupsCollected++;
                    switch (obj.powerType) {
                        case RAPID_FIRE:
                            gameState.rapidFire = true;
                            gameState.rapidFireTime = Config::POWERUP_DURATION;
                            break;
                        case SHIELD:
                            gameState.shield = true;
                            gameState.shieldTime = Config::POWERUP_DURATION;
                            break;
                        case MULTI_SHOT:
                            gameState.multiShot = true;
                            gameState.multiShotTime = Config::POWERUP_DURATION;
                            break;
                        case SPEED_BOOST:
                            gameState.speedBoost = true;
                            gameState.speedBoostTime = Config::POWERUP_DURATION;
                            break;
                        case LIFE_UP:
                            gameState.lives++;
                            break;
                    }
                    obj.active = false;
                    createExplosion(obj.posX, obj.posY, obj.colorR, obj.colorG, obj.colorB, 6);
                    gameState.score += Config::SCORE_POWERUP;
                    break;
            }
        }
        
        // Colisões de balas
        if (obj.type == BULLET) {
            for (size_t j = 0; j < gameObjects.size(); j++) { // Iterar usando size_t
                if (i == j) continue; // Bala não colide consigo mesma

                GameObject& target = gameObjects[j];
                // Balas não colidem com o jogador que as disparou, nem com outras balas, nem com powerups ou estrelas
                if (!target.active || target.type == PLAYER_SHIP || target.type == BULLET || target.type == POWERUP || target.type == STAR) continue;
                
                if ((target.type == ASTEROID || target.type == ENEMY) && 
                    checkCollision(obj, target)) {
                    // Destruir bala e alvo
                    obj.active = false;
                    target.active = false;
                    
                    // Criar explosão
                    createExplosion(target.posX, target.posY, target.colorR, target.colorG, target.colorB);
                      // Pontuação e estatísticas
                    if (target.type == ASTEROID) {
                        gameState.score += Config::SCORE_ASTEROID;
                        gameState.asteroidsDestroyed++;
                    } else {
                        gameState.score += Config::SCORE_ENEMY;
                        gameState.enemiesDestroyed++;
                    }
                    
                    // Chance de spawnar power-up
                    if (randomFloat(gen) < Config::POWERUP_DROP_CHANCE) {
                        spawnPowerUp();
                    }
                    break;
                }
            }
        }
    }
}

// Função para limpar objetos inativos
void cleanupObjects() {
    // Garantir que o jogador (índice 0) não seja removido se estiver inativo por engano aqui.
    // A lógica de gameOver deve tratar a inatividade do jogador.
    // Se o jogador (gameObjects[0]) se tornar inativo, ele será removido aqui, o que pode ser um problema
    // a menos que o jogo já esteja em estado de gameOver e prestes a reiniciar.
    // Por segurança, vamos preservar o jogador se ele for o primeiro elemento, a menos que o jogo tenha terminado.
    
    auto player_obj_iter = gameObjects.begin(); // Assumindo que o jogador é o primeiro

    gameObjects.erase(
        std::remove_if(gameObjects.begin() + (gameObjects.empty() ? 0 : 1), gameObjects.end(), // Começa após o jogador, se houver jogador
                      [](const GameObject& obj) { return !obj.active; }),
        gameObjects.end()
    );
    
    // Se o jogador (primeiro elemento) ficou inativo e o jogo não acabou, isso é um problema.
    // Mas a lógica atual de vidas e gameOver deve cobrir isso.
    // Se o jogador (gameObjects[0]) estiver inativo e o jogo não acabou, algo está errado.
    // Se o jogo acabou, o jogador pode ser "inativo" visualmente, mas o objeto ainda existe até o reinício.

    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                      [](const GameObject& obj) { return !obj.active; }),
        particles.end()
    );
}

// Função para atualizar lógica do jogo (melhorada)
void updateGame() {
    if (gameState.paused || gameState.gameOver) return;
    
    gameState.timeElapsed += deltaTime;

    // Atualizar todos os power-ups ativos
    if (gameState.rapidFire) {
        gameState.rapidFireTime -= deltaTime;
        if (gameState.rapidFireTime <= 0) {
            gameState.rapidFire = false;
            gameState.rapidFireTime = 0.0f; 
        }
    }
    if (gameState.shield) {
        gameState.shieldTime -= deltaTime;
        if (gameState.shieldTime <= 0) {
            gameState.shield = false;
            gameState.shieldTime = 0.0f;
        }
    }
    if (gameState.multiShot) {
        gameState.multiShotTime -= deltaTime;
        if (gameState.multiShotTime <= 0) {
            gameState.multiShot = false;
            gameState.multiShotTime = 0.0f;
        }
    }
    if (gameState.speedBoost) {
        gameState.speedBoostTime -= deltaTime;
        if (gameState.speedBoostTime <= 0) {
            gameState.speedBoost = false;
            gameState.speedBoostTime = 0.0f;
        }
    }
    
    // Sistema de ondas
    float timeSinceWaveStart = gameState.timeElapsed - gameState.waveStartTime;
    if (timeSinceWaveStart >= Config::WAVE_DURATION) {
        gameState.wave++;
        gameState.waveStartTime = gameState.timeElapsed;
        gameState.score += Config::SCORE_WAVE_BONUS * gameState.wave;
    }
    
    // Aumentar dificuldade gradualmente
    gameState.difficulty = 1.0f + gameState.timeElapsed * Config::DIFFICULTY_INCREASE_RATE;
    gameState.level = gameState.wave;
    
    // Spawnar asteroides (taxa ajustada pela dificuldade)
    float asteroidSpawnRate = Config::ASTEROID_BASE_SPAWN_RATE / gameState.difficulty;
    if (gameState.timeElapsed - gameState.lastAsteroidSpawn > asteroidSpawnRate) {
        spawnAsteroid();
        gameState.lastAsteroidSpawn = gameState.timeElapsed;
    }
    
    // Spawnar inimigos ocasionalmente
    float enemySpawnRate = Config::ENEMY_BASE_SPAWN_RATE / gameState.difficulty;
    if (gameState.timeElapsed - gameState.lastEnemySpawn > enemySpawnRate) {
        spawnEnemy();
        gameState.lastEnemySpawn = gameState.timeElapsed;
    }
    
    // Atualizar objetos
    updateGameObjects();
    
    // Verificar colisões
    checkCollisions();
    
    // Limpar objetos inativos
    cleanupObjects();
}

// Função para inicializar o jogo (melhorada)
void initializeGame() {
    gameObjects.clear();
    particles.clear();
    
    // Criar nave do jogador
    GameObject player = createPlayerShip();
    setupObject(player);
    gameObjects.push_back(player);
    playerIndex = 0;
    
    // Criar fundo estrelado mais denso
    for (int i = 0; i < Config::BACKGROUND_STARS; i++) {
        GameObject star = createStar();
        star.posX = (randomFloat(gen) - 0.5f) * Config::WORLD_WIDTH;
        star.posY = (randomFloat(gen) - 0.5f) * Config::WORLD_HEIGHT;
        setupObject(star);
        gameObjects.push_back(star);
    }    
    // Spawnar alguns asteroides iniciais (baseado na onda)
    for (int i = 0; i < 3; i++) {
        spawnAsteroid();
    }
    

}

// Função para imprimir informações do jogo (melhorada)
void printGameInfo() {
    static float lastPrint = 0.0f;
    if (gameState.timeElapsed - lastPrint > 1.0f) {
        system("cls");
        std::cout << "🚀 COSMIC DRIFT - Enhanced Edition 🚀\n";
        std::cout << "=====================================\n";
        std::cout << "Pontuação: " << gameState.score << "\n";
        std::cout << "Vidas: " << gameState.lives << "\n";
        std::cout << "Onda: " << gameState.wave << " | Nível: " << gameState.level << "\n";
        std::cout << "Dificuldade: " << std::fixed << std::setprecision(1) << gameState.difficulty << "x\n";
        
        // Power-ups ativos
        std::cout << "\n🎮 POWER-UPS ATIVOS:\n";
        if (gameState.rapidFire) {
            std::cout << "⚡ TIRO RÁPIDO: " << static_cast<int>(gameState.rapidFireTime) + 1 << "s\n";
        }
        if (gameState.shield) {
            std::cout << "🛡️  ESCUDO: " << static_cast<int>(gameState.shieldTime) + 1 << "s\n";
        }
        if (gameState.multiShot) {
            std::cout << "🔫 TIRO TRIPLO: " << static_cast<int>(gameState.multiShotTime) + 1 << "s\n";
        }
        if (gameState.speedBoost) {
            std::cout << "💨 VELOCIDADE: " << static_cast<int>(gameState.speedBoostTime) + 1 << "s\n";
        }
        
        // Estatísticas
        std::cout << "\n📊 ESTATÍSTICAS:\n";
        std::cout << "Asteroides: " << gameState.asteroidsDestroyed << " | ";
        std::cout << "Inimigos: " << gameState.enemiesDestroyed << " | ";
        std::cout << "Power-ups: " << gameState.powerupsCollected << "\n";
        
        if (gameState.paused) {
            std::cout << "\n⏸️  PAUSADO - Pressione P para continuar\n";
        }
        if (gameState.gameOver) {
            std::cout << "\n💀 GAME OVER! Pressione R para reiniciar\n";
            std::cout << "Pontuação Final: " << gameState.score << "\n";
        }
        
        lastPrint = gameState.timeElapsed;
    }
}

int main() {
    // Inicializar GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criar janela
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(Config::WINDOW_WIDTH), static_cast<int>(Config::WINDOW_HEIGHT), 
                                          "🚀 COSMIC DRIFT - Jogo 2D OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    // Configurar viewport
    glViewport(0, 0, static_cast<int>(Config::WINDOW_WIDTH), static_cast<int>(Config::WINDOW_HEIGHT));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Habilitar blending para transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaders com suporte a transformações e transparência
    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 transform;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = transform * vec4(aPos, 1.0);\n"
        "}\0";

    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec3 objectColor;\n"
        "uniform float alpha;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(objectColor, alpha);\n"
        "}\n\0";

    // Compilar e linkar shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERRO::SHADER::VERTEX::COMPILACAO_FALHOU\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERRO::SHADER::FRAGMENT::COMPILACAO_FALHOU\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERRO::SHADER::PROGRAMA::LINKAGEM_FALHOU\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Obter localizações dos uniforms
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    unsigned int alphaLoc = glGetUniformLocation(shaderProgram, "alpha");

    // Inicializar jogo
    initializeGame();

    // Loop de renderização
    while (!glfwWindowShouldClose(window)) {
        // Calcular deltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;
        
        // Input
        processInput(window);

        // Atualizar jogo
        updateGame();
        
        // Imprimir informações
        printGameInfo();

        // Renderização
        glClearColor(0.05f, 0.05f, 0.15f, 1.0f); // Fundo azul escuro espacial
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Renderizar objetos do jogo
        for(const auto& obj : gameObjects) {
            if (!obj.active) continue;
            
            // Aplicar transformações geométricas: T * R * S
            float transform[16] = {0};
            
            // Matriz de escala
            float scaleMatrix[16] = {0};
            scaleMatrix[0] = obj.scaleX; scaleMatrix[5] = obj.scaleY; 
            scaleMatrix[10] = 1.0f; scaleMatrix[15] = 1.0f;
            
            // Matriz de rotação
            float rotMatrix[16] = {0};
            float cosR = cos(obj.rotation);
            float sinR = sin(obj.rotation);
            rotMatrix[0] = cosR; rotMatrix[1] = sinR;
            rotMatrix[4] = -sinR; rotMatrix[5] = cosR;
            rotMatrix[10] = 1.0f; rotMatrix[15] = 1.0f;
            
            // Matriz de translação
            float transMatrix[16] = {0};
            transMatrix[0] = 1.0f; transMatrix[5] = 1.0f; transMatrix[10] = 1.0f; transMatrix[15] = 1.0f;
            transMatrix[12] = obj.posX; transMatrix[13] = obj.posY;
            
            // Multiplicar matrizes: T * R * S
            float tempMatrix[16] = {0};
            
            // R * S
            for(int row = 0; row < 4; row++) {
                for(int col = 0; col < 4; col++) {
                    for(int k = 0; k < 4; k++) {
                        tempMatrix[row * 4 + col] += rotMatrix[row * 4 + k] * scaleMatrix[k * 4 + col];
                    }
                }
            }
            
            // T * (R * S)
            for(int row = 0; row < 4; row++) {
                for(int col = 0; col < 4; col++) {
                    transform[row * 4 + col] = 0;
                    for(int k = 0; k < 4; k++) {
                        transform[row * 4 + col] += transMatrix[row * 4 + k] * tempMatrix[k * 4 + col];
                    }
                }
            }
            
            // Cores especiais para diferentes estados
            float color[3] = {obj.colorR, obj.colorG, obj.colorB};
            float alpha = obj.alpha;
              // Efeitos visuais especiais melhorados
            if (obj.type == PLAYER_SHIP) {
                if (gameState.shield) {
                    // Efeito de escudo pulsante
                    float pulse = 0.7f + 0.3f * sin(gameState.timeElapsed * 8.0f);
                    color[0] = 0.0f; color[1] = 1.0f; color[2] = 1.0f; // Ciano
                    alpha = pulse * 0.8f;
                } else if (gameState.speedBoost) {
                    // Efeito de velocidade com brilho verde
                    float pulse = 0.8f + 0.2f * sin(gameState.timeElapsed * 12.0f);
                    color[0] = obj.colorR * 0.5f; 
                    color[1] = obj.colorG + 0.3f; 
                    color[2] = obj.colorB * 0.5f;
                    alpha = pulse;
                } else if (gameState.rapidFire) {
                    // Efeito de tiro rápido com brilho amarelo
                    float pulse = 0.9f + 0.1f * sin(gameState.timeElapsed * 15.0f);
                    color[0] = obj.colorR + 0.2f; 
                    color[1] = obj.colorG + 0.2f; 
                    color[2] = obj.colorB;
                    alpha = pulse;
                } else if (gameState.multiShot) {
                    // Efeito de multi-shot com brilho magenta
                    float pulse = 0.9f + 0.1f * sin(gameState.timeElapsed * 10.0f);
                    color[0] = obj.colorR + 0.3f; 
                    color[1] = obj.colorG; 
                    color[2] = obj.colorB + 0.3f;
                    alpha = pulse;
                } else {
                    alpha = 1.0f;
                }
            } else if (obj.type == STAR) {
                // Estrelas piscam suavemente
                alpha = 0.3f + 0.4f * (0.5f + 0.5f * sin(gameState.timeElapsed * obj.pulseSpeed + obj.pulseOffset));
            } else if (obj.type == POWERUP) {
                // Power-ups têm brilho pulsante mais intenso
                float pulse = 0.7f + 0.3f * sin(gameState.timeElapsed * obj.pulseSpeed + obj.pulseOffset);
                alpha = pulse;
            }
            
            // Enviar uniforms
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);
            glUniform3fv(colorLoc, 1, color);
            glUniform1f(alphaLoc, alpha);
            
            // Desenhar objeto
            glBindVertexArray(obj.VAO);
            glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_INT, 0);
        }
        
        // Renderizar partículas
        for(const auto& particle : particles) {
            if (!particle.active) continue;
            
            // Transformação simples para partículas
            float transform[16] = {0};
            transform[0] = 1.0f; transform[5] = 1.0f; transform[10] = 1.0f; transform[15] = 1.0f;
            transform[12] = particle.posX; transform[13] = particle.posY;
            
            float color[3] = {particle.colorR, particle.colorG, particle.colorB};
            
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);
            glUniform3fv(colorLoc, 1, color);
            glUniform1f(alphaLoc, particle.alpha);
            
            glBindVertexArray(particle.VAO);
            glDrawElements(GL_TRIANGLES, particle.indices.size(), GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpar recursos
    for(auto& obj : gameObjects) {
        glDeleteVertexArrays(1, &obj.VAO);
        glDeleteBuffers(1, &obj.VBO);
        glDeleteBuffers(1, &obj.EBO);
    }
    for(auto& particle : particles) {
        glDeleteVertexArrays(1, &particle.VAO);
        glDeleteBuffers(1, &particle.VBO);
        glDeleteBuffers(1, &particle.EBO);
    }
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
