#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <ShObjIdl.h>
#include "camera.hpp"
#include "shader.hpp"
#include "model.hpp"

int WIDTH, HEIGHT;
float lastX, lastY;
float deltaTime = 0.0f, lastFrame = 0.0f;
bool firstMouse = true, mouse_right = false, mouse_left = false;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    WIDTH = width; HEIGHT = height;
    glViewport(0, 0, WIDTH, HEIGHT);
}
void processInput(GLFWwindow* window)
{
    if (mouse_left && mouse_right) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!mouse_right)
        return;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
        mouse_left = true;
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
        mouse_left = false;
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouse_right = true;
        firstMouse = true;
    }
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouse_right = false;
    }
}

class GlfwWindow {
public:
	GlfwWindow() {
        glfwSetErrorCallback(error_callback);
        glfwInit();
        if (!glfwInit())
        {
            std::cerr << "Failed to init GLFW" << std::endl;
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

        window = glfwCreateWindow(1, 1, "MyRenderer", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetMouseButtonCallback(window, mousebutton_callback);
        glfwGetWindowSize(window, &WIDTH, &HEIGHT);
        lastX = WIDTH / 2.0f; lastY = HEIGHT / 2.0f;

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            exit(-1);
        }

        imguiBeforeLoop();
	}

    void draw(Model& ourModel, Shader& faceShader, Shader& pointShader, Shader& lineShader) {
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, WIDTH, HEIGHT);
        while (!glfwWindowShouldClose(window))
        {
            processInput(window);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("control pannel");
                static unsigned int frames = 0, fps = 0;
                static float start_time = glfwGetTime();
                ++frames; float currentFrame = glfwGetTime(); deltaTime = currentFrame - lastFrame; lastFrame = currentFrame;
                if (currentFrame - start_time >= 1.0f) { fps = frames; frames = 0; start_time = currentFrame; }
                ImGui::Text("fps: %d    spf: %f", fps, deltaTime);
                static float background[4] = { 1.0, 1.0, 1.0, 1.0 };
                glClearColor(background[0], background[1], background[2], 1.0); 
                ImGui::ColorEdit3("background", background);
                if (ImGui::Button("Input Model")) { std::string path = inputModel(); if (path.size() != 0) ourModel = Model(path); }
                static bool fill = true, line = true, point = true;
                ImGui::Checkbox("fill", &fill);
                ImGui::Checkbox("line", &line);
                ImGui::Checkbox("point", &point);
                static float scale = 1.0;
                ImGui::SliderFloat("scale", &scale, 0.3, 2.0);
                static float rotation[3] = { 0.0, 0.0, 0.0 };
                ImGui::SliderFloat3("rotation", rotation, -180.0, 180.0);
                static float translate[3] = { 0.0, 0.0, 0.0 };
                ImGui::SliderFloat3("translate", translate, -2.0, 2.0);
                if (ImGui::Button("LoopSubdivision")) { ourModel.loopSub(); }
                static int faceNum = 300;
                ImGui::SliderInt("facesNum", &faceNum, 100, 1000);
                if (ImGui::Button("QEMSimplification")) { ourModel.qemSim(faceNum); }
                if (ImGui::Button("MinimalSurface")) { ourModel.miniSur(); }
            ImGui::End();

            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(translate[0], translate[1], translate[2]));
            model = glm::rotate(model, glm::radians(rotation[2]), glm::vec3(0.0, 0.0, 1.0));
            model = glm::rotate(model, glm::radians(rotation[1]), glm::vec3(0.0, 1.0, 0.0));
            model = glm::rotate(model, glm::radians(rotation[0]), glm::vec3(1.0, 0.0, 0.0));
            model = glm::scale(model, glm::vec3(scale, scale, scale));
            glm::mat4 tempM = glm::mat4(1.0f);
            float originS = 2.0 / std::max({ ourModel.xmax - ourModel.xmin, ourModel.ymax - ourModel.ymin, ourModel.zmax - ourModel.zmin });
            tempM = glm::scale(tempM, glm::vec3(originS, originS, originS));
            glm::vec3 originT(-(ourModel.xmax + ourModel.xmin) / 2.0, -(ourModel.ymax + ourModel.ymin) / 2.0, -(ourModel.zmax + ourModel.zmin) / 2.0);
            tempM = glm::translate(tempM, originT);
            model = model * tempM;
            if (point) {
                pointShader.use();
                pointShader.setMat4("projection", projection);
                pointShader.setMat4("view", view);
                pointShader.setMat4("model", model);
                pointShader.setMat4("tempM", tempM);
                glPointSize(3.0);
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                ourModel.Draw(pointShader);
            }
            if (line) {
                lineShader.use();
                lineShader.setMat4("projection", projection);
                lineShader.setMat4("view", view);
                lineShader.setMat4("model", model);
                lineShader.setMat4("tempM", tempM);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                ourModel.Draw(lineShader);
            }
            if (fill) {
                faceShader.use();
                faceShader.setMat4("projection", projection);
                faceShader.setMat4("view", view);
                faceShader.setMat4("model", model);
                faceShader.setMat4("tempM", tempM);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                ourModel.Draw(faceShader);
            }

            ImGui::Begin("log");
                ImGui::Text("Mesh Num: %d", ourModel.mSize());
                ImGui::Text("Vertex Num: %d", ourModel.vSize());
                ImGui::Text("Edge Num: %d", ourModel.eSize());
                ImGui::Text("Face Num: %d", ourModel.fSize());
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    void cleanUp() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
private:
	GLFWwindow* window;

    void imguiBeforeLoop() {
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

    std::string& inputModel() {
        std::string res;
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    size_t origsize = wcslen(pszFilePath) + 1;
                    size_t convertedChars = 0;

                    const size_t newsize = origsize * 2;
                    char* nstring = new char[newsize];

                    wcstombs_s(&convertedChars, nstring, newsize, pszFilePath, _TRUNCATE);

                    res = nstring;
                    // Display the file name to the user.
                    /*if (SUCCEEDED(hr))
                    {
                        MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                        CoTaskMemFree(pszFilePath);
                    }*/
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
        for (auto& c : res) {
            if (c == '\\')
                c = '/';
        }
        return res;
    }
};