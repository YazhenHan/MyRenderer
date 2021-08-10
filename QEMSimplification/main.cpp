#include "head.hpp"

int main()
{
    GLFWwindow* window = createWindow();

    imguiBeforeLoop(window);

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    Shader modelLoadingShader("model_loading.vs", "model_loading.fs");
    Shader pointShader("model_loading.vs", "point.fs");
    Shader lineShader("model_loading.vs", "line.fs");
    Model ourModel("C:/Users/gxucg/Documents/GitHub/MyRenderer/assets/african_head.obj");

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0, 1.0, 1.0, 1.0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        qemSimplification(ourModel, modelLoadingShader, pointShader, lineShader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cleanUp(window);
    return 0;
}