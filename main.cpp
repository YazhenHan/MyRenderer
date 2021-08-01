#include "head.hpp"
#include "IBL/util.h"

int main()
{
    GLFWwindow* window = createWindow();
    
    imguiBeforeLoop(window);

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    Shader modelLoadingShader("model_loading.vs", "model_loading.fs");
    Model ourModel("C:/Users/gxucg/Documents/GitHub/MyRenderer/assets/african_head.obj");

    float background_color[4] = { 1.0, 1.0, 1.0, 1.0 };
    bool wiremode = true;
    int faceNum = ourModel.getFaceNum();
    unsigned int frames = 0, fps = 0;
    float start_time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        ++frames;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (currentFrame - start_time >= 1.0f) { fps = frames; frames = 0; start_time = currentFrame; }

        wiremode ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if (mouse_left && mouse_right) camera.ProcessKeyboard(FORWARD, deltaTime);
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);

        modelLoadingShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        modelLoadingShader.setMat4("projection", projection);
        modelLoadingShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        modelLoadingShader.setMat4("model", model);

        ourModel.Draw(modelLoadingShader);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("settings", &wiremode, ImGuiWindowFlags_MenuBar);
        ImGui::Text("fps: %d    spf: %f", fps, deltaTime);
        ImGui::ColorEdit4("background", background_color);
        ImGui::Checkbox("wiremode", &wiremode);
        ImGui::Text("Face Number: %d", ourModel.getFaceNum());
        ImGui::Text("Vertex Number: %d", ourModel.getVertexNum());
        ImGui::Text("Mesh Number: %d", ourModel.meshes.size());
        ImGui::Text("Texture Number: %d", ourModel.textures_loaded.size());
        if (ImGui::Button("Input Model")) {
            string path = inputModel();
            if (path.size() != 0)
                ourModel = Model(path);
        }
        if (ImGui::Button("toHalfEdge")) {
            ourModel.toHalfEdge();
            ImGui::SameLine();
            ImGui::Text("Success!");
        }
        if (ImGui::Button("toGLMesh")) {
            ourModel.toGLMesh();
            ImGui::SameLine();
            ImGui::Text("Success!");
        }
        if (ImGui::Button("LoopSubdivision")) {
            ourModel.loopSub();
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cleanUp(window);
	return 0;
}