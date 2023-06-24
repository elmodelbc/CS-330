#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Camera class
#include <learnOpengl/camera.h> 

 // Standard namespace
using namespace std;

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Macro for window title
    const char* const WINDOW_TITLE = "Palmer 6-5 Milestone";

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        // Handle for the vertex array object
        GLuint vao;
        // Handle for the vertex buffer object
        GLuint vbo;
        // Number of indices of the mesh
        GLuint nVertices;
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;

    // Texture
    GLuint gTextureId;
    GLuint gTextureId1;
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader programs
    GLuint gCubeProgramId;
    GLuint gLampProgramId;
    GLuint gLampProgramId1;
    GLuint gLampProgramId2;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // time between current frame
    float gDeltaTime = 0.0f;
    // time between last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gCubeScale(1.0f);

    // Cube and light color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightColor1(0.0f, 1.0f, 0.0f);
    glm::vec3 gLightColor2(1.0f, 1.0f, 1.0f);

    // Light position and scale
    //glm::vec3 gLightPosition(6.5f, 2.0f, -0.5f);
    glm::vec3 gLightPosition(4.5f, 1.2f, -0.2f);
    glm::vec3 gLightScale(0.01f);

    // Light position and scale
    glm::vec3 gLightPosition1(-1.5f, 2.0f, 1.0f);
    glm::vec3 gLightScale1(0.03f);

    // Light position and scale
    glm::vec3 gLightPosition2(7.5f, 1.0f, -1.0f);
    glm::vec3 gLightScale2(0.03f);
}

// functions
bool Start(int, char* [], GLFWwindow** window);
void ResizeWindow(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window);
void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CreateMesh(GLMesh& mesh);
void DestroyMesh(GLMesh& mesh);
bool CreateTexture(const char* filename, GLuint& textureId);
void DestroyTexture(GLuint textureId);
void Render();
bool CreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void DestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    // VAP position 0 for vertex position data
    layout(location = 0) in vec3 position;
// VAP position 1 for normals
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textureCoordinate;

// For outgoing normals to fragment shader
out vec3 vertexNormal;
// For outgoing color / pixels to fragment shader
out vec3 vertexFragmentPos;
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transforms vertices into clip coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);

    // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));

    // get normal vectors in world space only and exclude normal translation properties
    vertexNormal = mat3(transpose(inverse(model))) * normal;
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    // For incoming normals
    in vec3 vertexNormal;
// For incoming fragment position
in vec3 vertexFragmentPos;
in vec2 vertexTextureCoordinate;

// For outgoing cube color to the GPU
out vec4 fragmentColor;

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightColor1;
uniform vec3 lightColor2;
uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 viewPosition;
uniform vec3 viewPosition1;
uniform vec3 viewPosition2;
// Useful when working with multiple textures
uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    // VAP position 0 for vertex position data
    layout(location = 0) in vec3 position;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transforms vertices into clip coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    // For outgoing lamp color (smaller cube) to the GPU
    out vec4 fragmentColor;

void main()
{
    // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    fragmentColor = vec4(1.0f);
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource1 = GLSL(440,

    // VAP position 0 for vertex position data
    layout(location = 0) in vec3 position;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transforms vertices into clip coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource1 = GLSL(440,

    // For outgoing lamp color (smaller cube) to the GPU
    out vec4 fragmentColor;

void main()
{
    // Set color to green (0.0f,1.0f,0.0f) with alpha 1.0
    fragmentColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource2 = GLSL(440,

    // VAP position 0 for vertex position data
    layout(location = 0) in vec3 position;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transforms vertices into clip coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource2 = GLSL(440,

    // For outgoing lamp color (smaller cube) to the GPU
    out vec4 fragmentColor;

void main()
{
    // Set color to green (0.0f,1.0f,0.0f) with alpha 1.0
    fragmentColor = vec4(1.0f);
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!Start(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    CreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!CreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;

    if (!CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    if (!CreateShaderProgram(lampVertexShaderSource1, lampFragmentShaderSource1, gLampProgramId1))
        return EXIT_FAILURE;

    if (!CreateShaderProgram(lampVertexShaderSource2, lampFragmentShaderSource2, gLampProgramId2))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "../resources/book_pages.png";
    if (!CreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture
    const char* texFilename1 = "../resources/brick.png";
    if (!CreateTexture(texFilename1, gTextureId1))
    {
        cout << "Failed to load texture " << texFilename1 << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gCubeProgramId);

    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        ProcessInput(gWindow);

        // Render this frame
        Render();

        glfwPollEvents();
    }

    // Release mesh data
    DestroyMesh(gMesh);

    // Release texture
    DestroyTexture(gTextureId);

    // Release shader programs
    DestroyShaderProgram(gCubeProgramId);
    DestroyShaderProgram(gLampProgramId);
    DestroyShaderProgram(gLampProgramId1);
    DestroyShaderProgram(gLampProgramId2);

    // Terminates the program successfully
    exit(EXIT_SUCCESS);
}


// Initialize GLFW, GLEW, and create a window
bool Start(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW: window creation
    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, ResizeWindow);
    glfwSetCursorPosCallback(*window, MousePositionCallback);
    glfwSetScrollCallback(*window, MouseScrollCallback);
    glfwSetMouseButtonCallback(*window, MouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void ProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void ResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
void MousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    // reversed since y-coordinates go from bottom to top
    float yoffset = gLastY - ypos;

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void Render()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the cube VAO (used by cube and lamp)
    glBindVertexArray(gMesh.vao);

    // CUBE: draw cube
    // 
    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    // 2. Rotates shape by 75 degrees in the z axis
    glm::mat4 rotation = glm::rotate(glm::radians(75.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gCubePosition) * rotation * glm::scale(gCubeScale);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightColorLoc1 = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightColorLoc2 = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint lightPositionLoc1 = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint lightPositionLoc2 = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");
    GLint viewPositionLoc1 = glGetUniformLocation(gCubeProgramId, "viewPosition");
    GLint viewPositionLoc2 = glGetUniformLocation(gCubeProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightColorLoc1, gLightColor1.r, gLightColor1.g, gLightColor1.b);
    glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(lightPositionLoc1, gLightPosition1.x, gLightPosition1.y, gLightPosition1.z);
    glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(viewPositionLoc1, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(viewPositionLoc2, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // LAMP: draw lamp

    // Set the shader to be used
    glUseProgram(gLampProgramId);

    // 2. Rotates shape by 75 degrees in the z axis
    rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * rotation * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // LAMP: draw lamp

    // Set the shader to be used
    glUseProgram(gLampProgramId1);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition1) * glm::scale(gLightScale1);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId1, "model");
    viewLoc = glGetUniformLocation(gLampProgramId1, "view");
    projLoc = glGetUniformLocation(gLampProgramId1, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // LAMP: draw lamp

// Set the shader to be used
    glUseProgram(gLampProgramId2);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition2) * glm::scale(gLightScale2);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId2, "model");
    viewLoc = glGetUniformLocation(gLampProgramId2, "view");
    projLoc = glGetUniformLocation(gLampProgramId2, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // Flips the the back buffer with the front buffer every frame.
    glfwSwapBuffers(gWindow);
}


// Implements the UCreateMesh function
void CreateMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions          //Normals
        // --------------------------------------

        // SIDE 2, top, left, paper, white TOP
        -1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // SIDE 14, top, left, paper, white BOTTOM
        -1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 5, bottom, left, paper, white TOP
        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // SIDE 14, top, left, paper, white BOTTOM
        -1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 5, bottom, left, paper, white TOP
        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // SIDE 17, bottom, left, paper, white BOTTOM
        -1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // SIDE 1, top, center, paper, white TOP
        0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
        // SIDE 2, top, left, paper, white TOP
        -1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // SIDE 13, top, center, paper, white BOTTOM
        0.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // SIDE 14, top, left, paper, white BOTTOM
        -1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 13, top, center, paper, white BOTTOM
        0.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 2, top, left, paper, white TOP
        -1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,

        // SIDE 12, top, right, paper, white BOTTOM
        1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 1, top, center, paper, white TOP
        0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
        // SIDE 13, top, center, paper, white BOTTOM
        0.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // SIDE 0, top, right, paper, white TOP
        1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // SIDE 12, top, right, paper, white BOTTOM
        1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 1, top, center, paper, white TOP
        0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 1.0f,

        // SIDE 4, bottom, center, paper, white TOP
        0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // SIDE 15, bottom, right, paper, white BOTTOM
        1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 3, bottom, right, paper, white TOP
        1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,

        // SIDE 16, bottom, center, paper, white BOTTOM
        0.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
        // SIDE 4, bottom, center, paper, white TOP
        0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // SIDE 15, bottom, right, paper, white BOTTOM
        1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // SIDE 15, bottom, right, paper, white BOTTOM
        1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 0, top, right, paper, white TOP
        1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // SIDE 12, top, right, paper, white BOTTOM
        1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // SIDE 3, bottom, right, paper, white TOP
        1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
        // SIDE 15, bottom, right, paper, white BOTTOM
        1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 0, top, right, paper, white TOP
        1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,

        // SIDE 16, bottom, center, paper, white BOTTOM
        0.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
        // SIDE 5, bottom, left, paper, white TOP
        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 4, bottom, center, paper, white TOP
        0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,

        // SIDE 5, bottom, left, paper, white TOP
        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
        // SIDE 17, bottom, left, paper, white BOTTOM
        -1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // SIDE 16, bottom, center, paper, white BOTTOM
        0.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,

        // 1, top, middle, paper, white TOP
        0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
        // 2, top, left, paper, white TOP
        -1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // 4, bottom, center, paper, white TOP
        0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // 2, top, left, paper, white TOP
        -1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // 4, bottom, center, paper, white TOP
        0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 5, bottom, left, paper, white TOP
        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,

        // 0, top, right, paper, white TOP
        1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
        // 1, top, center, paper, white TOP
        0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
        // 3, bottom, right, paper, white TOP
        1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,

        // 1, top, center, paper, white TOP
        0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
        // 4, bottom, center, paper, white TOP
        0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 3, bottom, right, paper, white TOP
        1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f,

        // 15, bottom, right, paper, white BOTTOM
        1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 12, top, right, paper, white BOTTOM
        1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 13, top, center, paper, white BOTTOM
        0.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // 16, bottom, center, paper, white BOTTOM
        0.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 17, bottom, left, paper, white BOTTOM
        -1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 13, top, center, paper, white BOTTOM
        0.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // 17, bottom, left, paper, white BOTTOM
        -1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 14, top, left, paper, white BOTTOM
        -1.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 13, top, center, paper, white BOTTOM
        0.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // 13, top, center, paper, white BOTTOM
        0.0f, 1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 16, bottom, center, paper, white BOTTOM
        0.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
        // 15, bottom, right, paper, white BOTTOM
        1.0f, -1.0f, -0.1f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,

        // 10, bottom, center, pad, yellow TOP
        0.0f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 9, bottom, right, pad, yellow TOP
        1.1f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 7, top, middle, pad, yellow TOP
        0.0f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // 6, top, right, pad, yellow TOP
        1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 9, bottom, right, pad, yellow TOP
        1.1f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 7, top, middle, pad, yellow TOP
        0.0f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 6, top, right, pad, yellow TOP
        1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 18, top, right, pad, yellow BOTTOM
        1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 7, top, middle, pad, yellow TOP
        0.0f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 7, top, middle, pad, yellow TOP
        0.0f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 18, top, right, pad, yellow BOTTOM
        1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 19, top, center, pad, yellow BOTTOM
        0.0f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 6, top, right, pad, yellow TOP
        1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 18, top, right, pad, yellow BOTTOM
        1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 21, bottom, right, pad, yellow BOTTOM
        1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 6, top, right, pad, yellow TOP
        1.1f, 1.1f, -0.1f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,
        // SIDE 21, bottom, right, pad, yellow BOTTOM
        1.1f, -1.1f, -0.2f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,
        // SIDE 9, bottom, right, pad, yellow TOP
        1.1f, -1.1f, -0.1f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,

        // SIDE 21, bottom, right, pad, yellow BOTTOM
        1.1f, -1.1f, -0.2f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,
        // SIDE 9, bottom, right, pad, yellow TOP
        1.1f, -1.1f, -0.1f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,
        // SIDE 10, bottom, center, pad, yellow TOP
        0.0f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 10, bottom, center, pad, yellow TOP
        0.0f, -1.1f, -0.1f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,
        // SIDE 21, bottom, right, pad, yellow BOTTOM
        1.1f, -1.1f, -0.2f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,
        // SIDE 22, bottom, center, pad, yellow BOTTOM
        0.0f, -1.1f, -0.2f, 1.0f, 1.0f, 0.0f, 2.0f, 2.0f,

        // 8, top, left, pad, yellow TOP
        -1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 7, top, center, pad, yellow TOP
        0.0f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 10, bottom, center, pad, yellow TOP
        0.0f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // 8, top, left, pad, yellow TOP
        -1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 10, bottom, center, pad, yellow TOP
        0.0f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // 11, bottom, left, pad, yellow TOP
        -1.1f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 7, top, center, pad, yellow TOP
        0.0f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 8, top, left, pad, yellow TOP
        -1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 20, top, left, pad, yellow BOTTOM
        -1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 7, top, center, pad, yellow TOP
        0.0f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 20, top, left, pad, yellow BOTTOM
        -1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 19, top, center, pad, yellow BOTTOM
        0.0f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 8, top, left, pad, yellow TOP
        -1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 20, top, left, pad, yellow BOTTOM
        -1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 23, bottom, left, pad, yellow BOTTOM
        -1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 23, bottom, left, pad, yellow BOTTOM
        -1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 11, bottom, left, pad, yellow TOP
        -1.1f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 8, top, left, pad, yellow TOP
        -1.1f, 1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 11, bottom, left, pad, yellow TOP
        -1.1f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 23, bottom, left, pad, yellow BOTTOM
        -1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 10, bottom, center, pad, yellow TOP
        0.0f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // SIDE 23, bottom, left, pad, yellow BOTTOM
        -1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 10, bottom, center, pad, yellow TOP
        0.0f, -1.1f, -0.1f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,
        // SIDE 22, bottom, center, pad, yellow BOTTOM
        0.0f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  2.0f, 2.0f,

        // 18, top, right, pad, yellow BOTTOM
        1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 19, top, center, pad, yellow BOTTOM
        0.0f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 21, bottom, right, pad, yellow BOTTOM
        1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,

        // 22, bottom, center, pad, yellow BOTTOM
        0.0f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 21, bottom, right, pad, yellow BOTTOM
        1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 19, top, center, pad, yellow BOTTOM
        0.0f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,

        // 20, top, left, pad, yellow BOTTOM
        -1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 19, top, center, pad, yellow BOTTOM
        0.0f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 22, bottom, center, pad, yellow BOTTOM
        0.0f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,

        // 20, top, left, pad, yellow BOTTOM
        -1.1f, 1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 22, bottom, center, pad, yellow BOTTOM
        0.0f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        // 23, bottom, left, pad, yellow BOTTOM
        -1.1f, -1.1f, -0.2f,   1.0f, 1.0f, 0.0f,  0.0f, 0.0f,

        //plane
        // 24, top, right, plane, blue
        8.0f, 2.0f, -0.2f,   1.0f, 1.0f, 1.0f,  2.0f, 2.0f,
        // 25, bottom, right, plane, blue
        8.0f, -2.0f, -0.2f,   1.0f, 1.0f, 1.0f,  2.0f, 2.0f,
        // 26, top, left, plane
        -4.0f, 2.0f, -0.2f,   1.0f, 1.0f, 1.0f,  2.0f, 2.0f,

        // 25, bottom, right, plane, blue
        8.0f, -2.0f, -0.2f,   1.0f, 1.0f, 1.0f,  2.0f, 2.0f,
        // 26, top, left, plane
        -4.0f, 2.0f, -0.2f,   1.0f, 1.0f, 1.0f,  2.0f, 2.0f,
        // 27, bottom, left, plane
        -4.0f, -2.0f, -0.2f,  1.0f, 1.0f, 1.0f,  2.0f, 2.0f,

            //Positions          //Normals
    // --------------------------------------
            //Back Face          //Negative Z Normals
            // 20, top, left, pad, yellow BOTTOM
            1.5f, -0.9f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 19, top, right, pad, yellow BOTTOM
            1.7f, -0.9f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 22, bottom, right, pad, yellow BOTTOM
            1.7f, -1.1f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // 20, top, left, pad, yellow BOTTOM
            1.5f, -0.9f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 22, bottom, right, pad, yellow BOTTOM
            1.7f, -1.1f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 23, bottom, left, pad, yellow BOTTOM
            1.5f, -1.1f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // top, back face
            1.6f, -1.0, -0.1f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 20, top, left, pad, yellow BOTTOM
            1.5f, -0.9f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 19, top, right, pad, yellow BOTTOM
            1.7f, -0.9f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // top, right face
            1.6f, -1.0, -0.1f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 19, top, right, pad, yellow BOTTOM
            1.7f, -0.9f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 22, bottom, right, pad, yellow BOTTOM
            1.7f, -1.1f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // top, left face
            1.6f, -1.0, -0.1f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 20, top, left, pad, yellow BOTTOM
            1.5f, -0.9f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 23, bottom, left, pad, yellow BOTTOM
            1.5f, -1.1f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // top, front face
            1.6f, -1.0, -0.1f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 22, bottom, right, pad, yellow BOTTOM
            1.7f, -1.1f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 23, bottom, left, pad, yellow BOTTOM
            1.5f, -1.1f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // lamp bottom
            // 24, top, right, plane, blue
            5.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 25, bottom, right, plane, blue
            5.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane
            4.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // 25, bottom, right, plane, blue
            5.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane
            4.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane
            4.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // lamp top
            // 24, top, right, plane, blue
            5.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 25, bottom, right, plane, blue
            5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane
            4.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // 25, bottom, right, plane, blue
            5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane
            4.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane
            4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // lamp side
            // 24, top, right, plane, blue BOTTOM
            5.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane BOTTOM
            4.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 24, top, right, plane, blue TOP
            5.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // lamp side
            // 24, top, right, plane, blue TOP
            5.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane TOP
            4.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane BOTTOM
            4.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // lamp side
            // 24, top, right, plane, blue BOTTOM
            5.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 25, bottom, right, plane, blue BOTTOM
            5.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 24, top, right, plane, blue TOP
            5.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // lamp side
            // 25, bottom, right, plane, blue BOTTOM
            5.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 24, top, right, plane, blue TOP
            5.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 25, bottom, right, plane, blue TOP
            5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // 26, top, left, plane TOP
            4.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane TOP
            4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 26, top, left, plane BOTTOM
            4.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // 26, top, left, plane BOTTOM
            4.0f, 1.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane BOTTOM
            4.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane TOP
            4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // 25, bottom, right, plane, blue BOTTOM
            5.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane BOTTOM
            4.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane TOP
            4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

            // 25, bottom, right, plane, blue TOP
            5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 27, bottom, left, plane TOP
            4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,
            // 25, bottom, right, plane, blue BOTTOM
            5.0f, 0.0f, -0.2f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f,

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // we can also generate multiple VAOs or buffers at the same time
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    // Activates the buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    // Sends vertex or coordinate data to the GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // The number of floats before each
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void DestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


/*Generate and load the texture*/
bool CreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        // Unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }

    // Error loading the image
    return false;
}


void DestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool CreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // compile the vertex shader
    glCompileShader(vertexShaderId);
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // compile the fragment shader
    glCompileShader(fragmentShaderId);
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // links the shader program
    glLinkProgram(programId);
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Uses the shader program
    glUseProgram(programId);

    return true;
}


void DestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
