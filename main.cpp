#include "glfwwindow.hpp"

int main()
{
    GlfwWindow glfwWindow;
    Shader modelLoadingShader("universal.vs", "face.fs");
    Shader pointShader("universal.vs", "point.fs");
    Shader lineShader("universal.vs", "line.fs");
    Model ourModel;
    glfwWindow.draw(ourModel, modelLoadingShader, pointShader, lineShader);
    glfwWindow.cleanUp();
    return 0;
}