#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "head.hpp"

int main()
{
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
        return -1;
    }

    // Setup Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Setup Dear ImGui Style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

            ImGui::Begin("canvas");
            static ImVector<ImVec2> points;
            static ImVec2 scrolling(0.0f, 0.0f);
            static bool opt_enable_grid = true;
            static bool opt_enable_context_menu = true;
            static bool adding_line = false;
            ImGui::ColorEdit3("background color", background_color);
            ImGui::Checkbox("Enable grid", &opt_enable_grid);
            ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
            ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

            // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!

            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
            
            // Draw border and background color
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(0, 0, 0, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(0, 255, 0, 255));

            // This will catch our interactions
            ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            const bool is_active = ImGui::IsItemActive();   // Held
            const bool is_clicked = ImGui::IsItemClicked();
            const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
            const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

            // Add first and second point
            if (is_clicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                points.push_back(mouse_pos_in_canvas);

            // Pan (we use a zero mouse threshold when there's no context menu)
            // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
            const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
            {
                scrolling.x += io.MouseDelta.x;
                scrolling.y += io.MouseDelta.y;
            }

            // Context menu (under default mouse threshold)
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            if (opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
                ImGui::OpenPopupOnItemClick("context");
            if (ImGui::BeginPopup("context"))
            {
                if (adding_line)
                    points.resize(points.size() - 2);
                adding_line = false;
                if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 1); }
                if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
                ImGui::EndPopup();
            }

            // Draw grid + all lines in the canvas
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);
            if (opt_enable_grid)
            {
                const float GRID_STEP = 64.0f;
                for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
                    draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
                for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
                    draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
            }
            for (int n = 0; n + 1 < points.Size; n += 1)
                draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
            for (int n = 0; n < points.Size; n += 1)
                draw_list->AddRectFilled(ImVec2(origin.x + points[n].x - 2.0, origin.y + points[n].y - 2.0), ImVec2(origin.x + points[n].x + 2.0, origin.y + points[n].y + 2.0), IM_COL32(255, 0, 0, 255));
            int n = points.size();
            Eigen::MatrixXf X(n, n);
            Eigen::VectorXf Y(n);
            Eigen::VectorXf A;
            if (n >= 1)
            {
                for (int i = 0; i < n; i++)
                {
                    float temp = 1;
                    for (int j = 0; j < n; j++)
                    {
                        X(i, j) = temp;
                        temp = temp * points[i].x;
                    }
                    Y[i] = points[i].y;
                }
                A = X.colPivHouseholderQr().solve(Y);
                for (int x = points[0].x; x <= points.back().x; x = x + 1)
                {
                    float y = A[0];
                    double temp = x;
                    for (int j = 1; j < n; j++)
                    {
                        y = y + A[j] * pow(x, j);
                        temp = temp * temp;
                    }
                    draw_list->AddRectFilled(ImVec2(origin.x + x - 1.0, origin.y + y - 1.0), ImVec2(origin.x + x + 1.0, origin.y + y + 1.0), IM_COL32(255, 255, 255, 255));
                }
            }
            draw_list->PopClipRect();

            ImGui::End();

            {
                ImGui::Begin("log");
                for (int n = 0; n < points.Size; n += 1)
                    ImGui::Text("%f...%f", points[n].x, points[n].y);
                for (int n = 0; n < points.Size; n += 1)
                    ImGui::Text("%f", A[n]);
                ImGui::NewLine();
                ImGui::NewLine();
                ImGui::NewLine();
                for (int i = 0; i < n; i++)
                {
                    ImGui::NewLine();
                    for (int j = 0; j < n; j++)
                        ImGui::Text("%f", X(i, j));
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                ImGui::NewLine();
                ImGui::NewLine();
                for (int n = 0; n < points.Size; n += 1)
                    ImGui::Text("%f", Y[n]);

                ImGui::End();
            }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
	return 0;
}