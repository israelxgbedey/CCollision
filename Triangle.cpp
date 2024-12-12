#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vertex shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 transform;
void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
}
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 triangleColor; // Color of the triangle

void main()
{
    FragColor = triangleColor;
}
)";

// Function to compile shader and check for errors
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

// Function to create shader program
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Helper function to initialize identity matrix
void identityMatrix(float* matrix) {
    for (int i = 0; i < 16; i++) {
        matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;  // Set diagonal to 1, others to 0
    }
}

// Helper function to create a translation matrix
void createTranslationMatrix(float x, float y, float* matrix) {
    identityMatrix(matrix);  // Start with identity matrix
    matrix[3] = x;  // Translate on x-axis
    matrix[7] = y;  // Translate on y-axis
}

// Collision detection function for two AABBs (Axis-Aligned Bounding Boxes)
bool checkCollision(float x1, float y1, float width1, float height1,
    float x2, float y2, float width2, float height2) {
    return !(x1 + width1 < x2 || x1 > x2 + width2 || y1 + height1 < y2 || y1 > y2 + height2);
}

int checkCollisionDirection(float x1, float y1, float width1, float height1,
    float x2, float y2, float width2, float height2) {
    if (x1 + width1 > x2 && x1 < x2 + width2 && y1 + height1 > y2 && y1 < y2 + height2) {
        // Left collision
        if (x1 + width1 > x2 && x1 < x2) {
            if (y1 + height1 > y2 && y1 < y2 + height2) {
                return 1; // Collision from the left
            }
        }

        // Right collision
        if (x1 < x2 + width2 && x1 + width1 > x2 + width2) {
            if (y1 + height1 > y2 && y1 < y2 + height2) {
                return 2; // Collision from the right
            }
        }

        // Top collision
        if (y1 + height1 > y2 && y1 < y2) {
            if (x1 + width1 > x2 && x1 < x2 + width2) {
                return 3; // Collision from the top
            }
        }

        // Bottom collision
        if (y1 < y2 + height2 && y1 + height1 > y2 + height2) {
            if (x1 + width1 > x2 && x1 < x2 + width2) {
                return 4; // Collision from the bottom
            }
        }
    }

    return 0; // No collision
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create GLFW window and context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Controllable Triangle with Collision", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    float vertices[] = {
        0.0f,  0.25f, 0.0f,  // Top vertex (smaller)
       -0.25f, -0.25f, 0.0f,  // Bottom-left vertex
        0.25f, -0.25f, 0.0f   // Bottom-right vertex
    };

    // Square vertices
    float squareVertices[] = {
        -0.5f, -0.5f, 0.0f,
        -0.5f, -0.25f, 0.0f,
        -0.25f, -0.25f, 0.0f,
        -0.25f, -0.5f, 0.0f
    };

    // Generate VAOs and VBOs
    GLuint VAO[2], VBO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);

    // Triangle
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Square
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Compile and link shader program
    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    // Initial object positions
    float translationX = -1.0f;
    float translationY = -0.75f;  // Align triangle with square
    float squareTranslationX = 0.0f;
    float squareTranslationY = -0.5f;  // Square aligned to same horizontal axis

    // Jumping variables
    bool isJumping = false;
    bool isFalling = false;
    float jumpHeight = 0.0f;
    float jumpSpeed = 0.1f;
    float jumpStartTime = 0.0f;
    float jumpDuration = 1.0f;

    // Time tracking
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Control triangle movement
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            translationX -= 0.01f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            translationX += 0.01f;

        // Start jump when space is pressed
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isJumping) {
            isJumping = true;
            jumpStartTime = currentFrame;
            isFalling = false;
        }

        // Handle jumping logic
        if (isJumping) {
            float jumpProgress = (currentFrame - jumpStartTime) / jumpDuration;
            if (jumpProgress < 1.0f) {
                jumpHeight = sinf(jumpProgress * M_PI) * 0.5f;  // Upward motion
            }
            else {
                jumpHeight = 0.0f;
                isJumping = false;
                isFalling = true;  // Start falling
            }
        }

        // Handle falling logic
        if (isFalling) {
            jumpHeight -= jumpSpeed;  // Move down after jump
            if (jumpHeight <= 0.0f) {
                jumpHeight = 0.0f;
                isFalling = false;  // Land when touching the ground (adjust this logic if you have a ground)
            }
        }

        // Only check for collision when falling
        bool isColliding = checkCollision(
            translationX, translationY + jumpHeight, 0.5f, 0.5f,  // Triangle position and size
            squareTranslationX - 0.25f, squareTranslationY - 0.25f, 0.5f, 0.5f // Square position and size
        );

        // Set the triangle's color based on the collision
        float triangleColor[4] = { 0.4f, 0.8f, 0.6f, 1.0f }; // Default color (green)
        if (isColliding) {
            triangleColor[0] = 1.0f; // Red
            triangleColor[1] = 0.0f; // No green
            triangleColor[2] = 0.0f; // No blue
        }

        // Set the uniform for the triangle color
        GLuint colorLoc = glGetUniformLocation(shaderProgram, "triangleColor");
        glUniform4fv(colorLoc, 1, triangleColor);

        // Rendering the scene (triangle and square)
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the triangle
        float transform[16];
        createTranslationMatrix(translationX, translationY + jumpHeight, transform);
        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_TRUE, transform);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw the square
        createTranslationMatrix(squareTranslationX, squareTranslationY, transform);
        glUniformMatrix4fv(transformLoc, 1, GL_TRUE, transform);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_QUADS, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
