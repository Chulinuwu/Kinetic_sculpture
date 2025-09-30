#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include <iostream>
#include <vector>
#include <cmath>

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float yaw = -90.0f;
float pitch = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void generateParametricSurface(std::vector<float>& vertices, std::vector<unsigned int>& indices, 
                                int uRes, int vRes, float time);

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Kinetic Sculpture - Parametric Surface", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);

    // Build and compile shaders
    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Generate initial mesh
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int uResolution = 100;  // Higher = smoother surface
    int vResolution = 100;
    
    generateParametricSurface(vertices, indices, uResolution, vResolution, 0.0f);

    // Setup VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Light positions
    glm::vec3 lightPositions[] = {
        glm::vec3(3.0f, 3.0f, 3.0f),
        glm::vec3(-3.0f, 3.0f, 3.0f),
        glm::vec3(3.0f, -3.0f, 3.0f),
        glm::vec3(-3.0f, -3.0f, 3.0f)
    };

    glm::vec3 lightColors[] = {
        glm::vec3(1.0f, 0.8f, 0.8f),   // Warm white
        glm::vec3(0.8f, 0.8f, 1.0f),   // Cool white
        glm::vec3(1.0f, 0.9f, 0.7f),   // Yellowish
        glm::vec3(0.9f, 0.8f, 1.0f)    // Purple-ish
    };

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Render
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update parametric surface with animation
        vertices.clear();
        indices.clear();
        generateParametricSurface(vertices, indices, uResolution, vResolution, currentFrame);

        // Update VBO and EBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

        // Activate shader
        shader.use();

        // Set lights
        for (int i = 0; i < 4; i++)
        {
            std::string number = std::to_string(i);
            shader.setVec3("lightPositions[" + number + "]", lightPositions[i]);
            shader.setVec3("lightColors[" + number + "]", lightColors[i]);
        }
        shader.setVec3("viewPos", cameraPos);
        shader.setVec3("objectColor", 0.8f, 0.6f, 0.9f);  // Purple-ish color

        // View/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT, 
                                                0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // Model transformation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, currentFrame * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", model);

        // Draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

// Generate parametric surface with animation
void generateParametricSurface(std::vector<float>& vertices, std::vector<unsigned int>& indices, 
                                int uRes, int vRes, float time)
{
    // Parametric equations for animated shell-like surface
    // u: [0, 2π], v: [0, π]
    
    for (int i = 0; i <= vRes; i++)
    {
        for (int j = 0; j <= uRes; j++)
        {
            float u = (float)j / uRes * 2.0f * 3.14159265f;  // 0 to 2π
            float v = (float)i / vRes * 3.14159265f;          // 0 to π
            
            // Animated wave parameters
            float wave1 = sin(u * 3.0f - time * 2.0f) * 0.15f;
            float wave2 = cos(v * 2.0f + time * 1.5f) * 0.15f;
            float ripple = sin(u * 5.0f + v * 3.0f - time * 3.0f) * 0.08f;
            
            // Shell-like parametric surface with waves
            float radius = 1.5f + wave1 + wave2 + ripple;
            float x = radius * sin(v) * cos(u);
            float y = radius * sin(v) * sin(u);
            float z = radius * cos(v);
            
            // Calculate normal using cross product of tangent vectors
            // Partial derivatives approximation
            float du = 0.01f;
            float dv = 0.01f;
            
            float wave1_du = sin((u + du) * 3.0f - time * 2.0f) * 0.15f;
            float wave2_dv = cos((v + dv) * 2.0f + time * 1.5f) * 0.15f;
            float ripple_du = sin((u + du) * 5.0f + v * 3.0f - time * 3.0f) * 0.08f;
            float ripple_dv = sin(u * 5.0f + (v + dv) * 3.0f - time * 3.0f) * 0.08f;
            
            float r_du = 1.5f + wave1_du + wave2 + ripple_du;
            float r_dv = 1.5f + wave1 + wave2_dv + ripple_dv;
            
            float x_du = r_du * sin(v) * cos(u + du);
            float y_du = r_du * sin(v) * sin(u + du);
            float z_du = r_du * cos(v);
            
            float x_dv = r_dv * sin(v + dv) * cos(u);
            float y_dv = r_dv * sin(v + dv) * sin(u);
            float z_dv = r_dv * cos(v + dv);
            
            glm::vec3 tangentU = glm::vec3(x_du - x, y_du - y, z_du - z);
            glm::vec3 tangentV = glm::vec3(x_dv - x, y_dv - y, z_dv - z);
            glm::vec3 normal = glm::normalize(glm::cross(tangentU, tangentV));
            
            // Add vertex data: position + normal
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }
    
    // Generate indices for triangles
    for (int i = 0; i < vRes; i++)
    {
        for (int j = 0; j < uRes; j++)
        {
            int p0 = i * (uRes + 1) + j;
            int p1 = p0 + 1;
            int p2 = (i + 1) * (uRes + 1) + j;
            int p3 = p2 + 1;
            
            // Two triangles per quad
            indices.push_back(p0);
            indices.push_back(p2);
            indices.push_back(p1);
            
            indices.push_back(p1);
            indices.push_back(p2);
            indices.push_back(p3);
        }
    }
}

// Process input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Optional: implement zoom
}