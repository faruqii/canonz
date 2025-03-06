#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GRAVITY = 9.81f;
const float PI = 3.14159265359f;

// Cannon properties
float cannonAngle = 45.0f; // Initial angle in degrees
float cannonPower = 50.0f; // Initial power
glm::vec2 cannonPosition(50.0f, 50.0f);

// Projectile class
class Projectile {
public:
    glm::vec2 position;
    glm::vec2 velocity;
    float radius;
    bool active;
    float timeAlive;

    Projectile(glm::vec2 pos, glm::vec2 vel, float r)
        : position(pos), velocity(vel), radius(r), active(true), timeAlive(0.0f) {}

    void update(float deltaTime) {
        // Apply gravity
        velocity.y -= GRAVITY * deltaTime;
        
        // Update position
        position += velocity * deltaTime;
        
        // Increase time alive
        timeAlive += deltaTime;
        
        // Check if projectile hits the ground
        if (position.y <= radius) {
            position.y = radius;
            velocity *= 0.5f; // Dampen velocity (bounce)
            
            // If velocity is very low, make the projectile inactive
            if (glm::length(velocity) < 1.0f) {
                active = false;
            } else {
                velocity.y = -velocity.y * 0.7f; // Bounce with energy loss
            }
        }
        
        // Check if projectile hits the wall
        if (position.x >= WINDOW_WIDTH - radius) {
            position.x = WINDOW_WIDTH - radius;
            velocity.x *= -0.7f; // Bounce off wall
        }
    }
};

// Global variables
std::vector<Projectile> projectiles;
float lastFrameTime = 0.0f;
bool fireCannon = false;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void drawCannon();
void drawProjectiles();
void drawGround();
void fireProjectile();

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Cannon Simulator", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Configure viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        // Process input
        processInput(window);
        
        // Fire cannon if requested
        if (fireCannon) {
            fireProjectile();
            fireCannon = false;
        }
        
        // Update projectiles
        for (auto& projectile : projectiles) {
            if (projectile.active) {
                projectile.update(deltaTime);
            }
        }
        
        // Remove inactive projectiles
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                [](const Projectile& p) { return !p.active || p.timeAlive > 10.0f; }),
            projectiles.end()
        );
        
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw ground
        drawGround();
        
        // Draw cannon
        drawCannon();
        
        // Draw projectiles
        drawProjectiles();
        
        // Display cannon stats
        // (In a real implementation, you would use text rendering here)
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Clean up
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        fireCannon = true;
    }
}

void processInput(GLFWwindow* window) {
    // Adjust cannon angle
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cannonAngle = std::min(cannonAngle + 1.0f, 90.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cannonAngle = std::max(cannonAngle - 1.0f, 0.0f);
    }
    
    // Adjust cannon power
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cannonPower = std::min(cannonPower + 1.0f, 100.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cannonPower = std::max(cannonPower - 1.0f, 10.0f);
    }
}

void drawCannon() {
    // Base of the cannon (circle)
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cannonPosition.x, cannonPosition.y);
    for (int i = 0; i <= 360; i += 10) {
        float radian = i * PI / 180.0f;
        glVertex2f(cannonPosition.x + 20.0f * cos(radian),
                   cannonPosition.y + 20.0f * sin(radian));
    }
    glEnd();
    
    // Barrel of the cannon
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(cannonPosition.x, cannonPosition.y, 0.0f);
    glRotatef(cannonAngle, 0.0f, 0.0f, 1.0f);
    
    glBegin(GL_QUADS);
    glVertex2f(0.0f, -5.0f);
    glVertex2f(40.0f, -5.0f);
    glVertex2f(40.0f, 5.0f);
    glVertex2f(0.0f, 5.0f);
    glEnd();
    
    glPopMatrix();
}

void drawProjectiles() {
    for (const auto& projectile : projectiles) {
        if (projectile.active) {
            glColor3f(0.9f, 0.1f, 0.1f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(projectile.position.x, projectile.position.y);
            for (int i = 0; i <= 360; i += 10) {
                float radian = i * PI / 180.0f;
                glVertex2f(projectile.position.x + projectile.radius * cos(radian),
                           projectile.position.y + projectile.radius * sin(radian));
            }
            glEnd();
        }
    }
}

void drawGround() {
    glColor3f(0.0f, 0.7f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(WINDOW_WIDTH, 0.0f);
    glVertex2f(WINDOW_WIDTH, 50.0f);
    glVertex2f(0.0f, 50.0f);
    glEnd();
}

void fireProjectile() {
    // Calculate initial velocity based on angle and power
    float radianAngle = cannonAngle * PI / 180.0f;
    glm::vec2 initialVelocity(
        cannonPower * cos(radianAngle),
        cannonPower * sin(radianAngle)
    );
    
    // Calculate the barrel end position
    glm::vec2 barrelEnd(
        cannonPosition.x + 40.0f * cos(radianAngle),
        cannonPosition.y + 40.0f * sin(radianAngle)
    );
    
    // Create a new projectile
    Projectile projectile(barrelEnd, initialVelocity, 5.0f);
    projectiles.push_back(projectile);
}