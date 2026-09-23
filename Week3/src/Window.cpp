#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include "Window.h"
#include <cassert>
#include <iostream>
#include <memory>

struct App
{
	GLFWwindow* window = nullptr;
    int keysPrev[KEY_COUNT]{};
    int keysCurr[KEY_COUNT]{};
} gApp;

void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_REPEAT) return;
    gApp.keysCurr[key] = action;
    
    // Uncomment to see how key events work!
    //const char* name = glfwGetKeyName(key, scancode);
    //if (action == GLFW_PRESS)
    //    printf("%s is down\n", name);
    //else if (action == GLFW_RELEASE)
    //    printf("%s is up\n", name);
}

void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

void CreateWindow(int width, int height, const char* title)
{
    /* Initialize the library */
    int initialized = glfwInit();
    assert(initialized == GLFW_TRUE);
    (void)initialized;

    // Request OpenGL 3.3 core (macOS supports at most 4.1, and glad is generated for 3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

#ifdef NDEBUG
    // Don't make a debug context if in release mode
#else
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    /* Create a windowed mode window and its OpenGL context */
    gApp.window = glfwCreateWindow(width, height, title, NULL, NULL);
    assert(gApp.window != nullptr);

    /* Make the window's context current */
    glfwMakeContextCurrent(gApp.window);

    // Load OpenGL extensions
    int version = gladLoadGL(glfwGetProcAddress);
    assert(version != 0);
    (void)version;

    glfwSetKeyCallback(gApp.window, KeyboardCallback);
#ifdef NDEBUG
#else
    // Debug output requires GL 4.3 / KHR_debug, which macOS doesn't provide
    if (GLAD_GL_KHR_debug)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(DebugCallback, nullptr);
    }
#endif
}

void SetWindowShouldClose(bool close)
{
    glfwSetWindowShouldClose(gApp.window, close ? GLFW_TRUE : GLFW_FALSE);
}

bool WindowShouldClose()
{
    return glfwWindowShouldClose(gApp.window);
}

void Loop()
{
    // Last frame escape down
    // This frame escape up
    memcpy(gApp.keysPrev, gApp.keysCurr, sizeof(int) * KEY_COUNT);

    /* Swap front and back buffers */
    glfwSwapBuffers(gApp.window);

    /* Poll for and process events */
    glfwPollEvents();
}

bool IsKeyDown(int key)
{
    return gApp.keysCurr[key] == GLFW_PRESS;
}

bool IsKeyUp(int key)
{
    return gApp.keysCurr[key] == GLFW_RELEASE;
}

bool IsKeyPressed(int key)
{
    return 
        gApp.keysPrev[key] == GLFW_PRESS &&
        gApp.keysCurr[key] == GLFW_RELEASE;
}

void DestroyWindow()
{
    glfwTerminate();
}
