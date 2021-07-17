#include "head.hpp"
#include "fitting.hpp"

int main()
{
    GLFWwindow* window = createWindow();
    imguiBeforeLoop(window);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        fittingCanvas();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Cleanup
    cleanUp(window);
	return 0;
}