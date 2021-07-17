#pragma once

// flags and functions

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Eigen/Dense>

#include <iostream>
#include <vector>

float background_color[4] = { 0.0, 0.0, 0.0, 0.0 };

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

GLFWwindow* createWindow() {
    glfwSetErrorCallback(error_callback);
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    if (!glfwInit())
    {
        std::cout << "Failed to init GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(1, 1, "MyRenderer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    return window;
}

void imguiBeforeLoop(GLFWwindow* window) {
    // Setup Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Setup Dear ImGui Style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void cleanUp(GLFWwindow* window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

int jiecheng(int m)
{
    if (m == 0)
        return 1;
    int res = 1;
    while (m >= 1)
    {
        res *= m;
        m--;
    }
    return res;
}

int zuhe(int m, int n)
{
    if (n == 0 || m == n)
        return 1;
    return jiecheng(m) / (jiecheng(n) * jiecheng(n - m));
}