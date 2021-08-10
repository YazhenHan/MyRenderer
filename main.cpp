#include "head.hpp"
#include "model.hpp"

int main()
{
    GLFWwindow* window = createWindow();
    
    imguiBeforeLoop(window);

    glEnable(GL_DEPTH_TEST);

    Shader modelLoadingShader("universal.vs", "face.fs");
    Shader pointShader("universal.vs", "point.fs");
    Shader lineShader("universal.vs", "line.fs");
    Model ourModel;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0, 1.0, 1.0, 1.0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw(ourModel, modelLoadingShader, pointShader, lineShader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cleanUp(window);
	return 0;
}