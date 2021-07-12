#pragma once

// flags and functions

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Eigen/Dense>

#include <iostream>

float background_color[3] = { 0.0, 0.0, 0.0 };

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void solve(const ImVector<ImVec2>& points)
{
    int n = points.size();
    Eigen::MatrixXf X(n, n);
    Eigen::VectorXf Y(n);
    for (int i = 0; i < n; i++)
    {
        auto temp = 1;
        for (int j = 0; j < n; j++)
        {
            X << temp;
            temp = temp * points[i].x;
        }
        Y << points[i].y;
    }
    Eigen::VectorXf A = X.colPivHouseholderQr().solve(Y);
}