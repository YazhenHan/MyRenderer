#pragma once
#include "head.hpp"

// InterpolationPolynomialBaseFunction
Eigen::VectorXd interpolationPolynomial(const ImVector<ImVec2>& points) {
    int n = points.size();

    Eigen::MatrixXd normal_equation = Eigen::MatrixXd::Zero(n, n);
    Eigen::VectorXd y = Eigen::VectorXd::Zero(n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            normal_equation(i, j) = pow(points[i].x, j);
        y[i] = points[i].y;
    }

    return normal_equation.inverse() * y;
}

// InterpolationBernsteinBaseFunction
Eigen::VectorXd interpolationBernstein(const ImVector<ImVec2>& points) {
    int n = points.size();

    Eigen::MatrixXd normal_equation = Eigen::MatrixXd::Zero(n, n);
    Eigen::VectorXd y = Eigen::VectorXd::Zero(n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            normal_equation(i, j) = zuhe(n - 1, j) * pow(points[i].x, j) * pow(1 - points[i].x, n - 1 - j);
        y[i] = points[i].y;
    }

    return normal_equation.inverse() * y;
}

// InterpolationGaussBaseFunction
float gauss(float x, float xi, float sigma) {
    return expf(-(x - xi) * (x - xi) / (2 * sigma * sigma));
}

Eigen::VectorXf interpolationGauss(const ImVector<ImVec2>& points, float sigma = 1.0) {
    int n = points.size();

    Eigen::MatrixXf normal_equation = Eigen::MatrixXf::Zero(n + 1, n + 1);
    Eigen::VectorXf y = Eigen::VectorXf::Zero(n + 1);

    for (int i = 0; i < n; i++) {
        normal_equation(i, 0) = 1;
        for (int j = 1; j < n + 1; j++)
            normal_equation(i, j) = gauss(points[i].x, points[j - 1].x, sigma);
        y[i] = points[i].y;
    }
    normal_equation(n, 0) = 1;
    for (int j = 1; j < n; j++)
        normal_equation(n, j) = gauss((points[n - 1].x + points[n - 2].x) / 2.0, points[j - 1].x, sigma);
    y[n] = (points[n - 1].y + points[n - 2].y) / 2.0;

    return normal_equation.inverse() * y;
}

// Approximation_LeastSquare
Eigen::VectorXf approximation_LeastSquare(const ImVector<ImVec2>& points, int order = 3) {
    int n = points.size();

    Eigen::MatrixXf normal_equation = Eigen::MatrixXf::Zero(n, order + 1);
    Eigen::VectorXf y = Eigen::VectorXf::Zero(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= order; j++)
            normal_equation(i, j) = pow(points[i].x, j);
        y[i] = points[i].y;
    }

    return (normal_equation.transpose() * normal_equation).inverse() * (normal_equation.transpose() * y);
}

// Approximation_RidgeRegression
Eigen::VectorXf approximation_RidgeRegression(const ImVector<ImVec2>& points, int order = 3, float lambda = 0.1) {
    int n = points.size();

    Eigen::MatrixXf normal_equation = Eigen::MatrixXf::Zero(n, order + 1);
    Eigen::VectorXf y = Eigen::VectorXf::Zero(n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j <= order; j++)
            normal_equation(i, j) = pow(points[i].x, j);
        y[i] = points[i].y;
    }
    Eigen::MatrixXf I;
    I.setIdentity(order + 1, order + 1);
    return (normal_equation.transpose() * normal_equation + I * lambda).inverse() * normal_equation.transpose() * y;
}

double gauss(double x) {
    return exp(-0.5 * x * x);
}

Eigen::VectorXd a;
Eigen::VectorXd b;
Eigen::VectorXd W;
double B;
std::vector<double> losses;
static bool train(const ImVector<ImVec2>& points, const int m, const int loops, const double lr, const int batch) {
    int n = points.size();

    a.resize(m); b.resize(m); W.resize(m); losses.clear();
    for (int i = 0; i < m; i++)
    {
        a[i] = rand() / double(RAND_MAX) * 2.0 - 1.0;
        b[i] = 0.0;
        W[i] = rand() / double(RAND_MAX) * 2.0 - 1.0;
    }
    B = 0.0;

    for (int loop = 0; loop < loops / n * n; loop++)
    {
        int rp = rand() % n;
        double X = 0.0, Y = 0.0;
        for (int i = 0; i < batch; i++)
        {
            X += points[(rp + i) % n].x;
            Y += points[(rp + i) % n].y;
        }
        X /= batch;
        Y /= batch;

        Eigen::VectorXd S = X * a + b;
        Eigen::VectorXd SS(m);
        for (int i = 0; i < m; i++)
            SS[i] = gauss(S[i]);
        double fx = (SS.transpose() * W)[0] + B;

        double loss = 0.0;
        for (int i = 0; i < n; i++)
        {
            double x = points[i].x;
            Eigen::VectorXd r1 = x * a + b;
            for (int i = 0; i < r1.size(); i++)
                r1[i] = gauss(r1[i]);
            double y = (r1.transpose() * W)[0] + B;
            loss += 0.5 * (y - Y) * (y - Y);
        }
        loss /= n;
        losses.push_back(loss);
        /*if (loop % (loops / 10) == 0)
            printf("%d\t%f\n", loop / (loops / 10), loss);*/

        Eigen::VectorXd kw = (fx - Y) * SS;
        double k0 = fx - Y;
        Eigen::VectorXd ka = -(fx - Y) * X * W.cwiseProduct(SS).cwiseProduct(S);
        Eigen::VectorXd kb = -(fx - Y) * W.cwiseProduct(SS).cwiseProduct(S);
        W -= lr * kw;
        B -= lr * k0;
        a -= lr * ka;
        b -= lr * kb;
    }

    return true;
}

void fittingCanvas() {
    ImGui::Begin("canvas");
    static ImVector<ImVec2> points;
    static ImVector<ImVec2> points2;
    static ImVec2 scrolling(0.0f, 0.0f);
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    static bool adding_line = false;

    static float sigma = 1.0;
    static int order = 3;
    static bool polynomialI = false, lagrangeI = false, gaussI = false, bernsteinI = false,
        leastSquareA = false, ridgeRegressionA = false, neuralnetworkA = false;
    static float lambda = 0.1;
    static int m = 10, loops = 10000, batch = 1;
    static float lr = 0.1;

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Interpolation"))
        {
            ImGui::NewLine();
            ImGui::Checkbox("Polynomial", &polynomialI); ImGui::SameLine();
            ImGui::Checkbox("Lagrange", &lagrangeI); ImGui::SameLine();
            ImGui::Checkbox("Bernstein", &bernsteinI); ImGui::SameLine();
            ImGui::Checkbox("Gauss", &gaussI);
            ImGui::NewLine();
            ImGui::SliderFloat("sigma", &sigma, 1.0, 200.0);
            ImGui::NewLine();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Approach"))
        {
            ImGui::NewLine();
            ImGui::Checkbox("LeastSquare", &leastSquareA); ImGui::SameLine();
            ImGui::Checkbox("RidgeRegression", &ridgeRegressionA);
            ImGui::NewLine();
            ImGui::SliderInt("Least Square", &order, 0, 20);
            ImGui::SliderFloat("lambda", &lambda, 0.0, 0.2);
            ImGui::NewLine();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Neural Network"))
        {
            ImGui::SliderFloat("Step size", &lr, 0.0, 0.3);
            ImGui::SliderInt("Loops", &loops, 100, 30000);
            ImGui::SliderInt("Hidden layer size", &m, 3, 300);
            ImGui::SliderInt("Batch size", &batch, 1, points.size());
            if (ImGui::Button("Train") && points.size() > 1)
            {
                neuralnetworkA = false;
                points2.resize(points.size());
                for (int i = 0; i < points.size(); i++)
                    points2[i] = ImVec2(points[i].x / 100.0, points[i].y / 100.0);
                neuralnetworkA = train(points2, m, loops, lr, batch);
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
                neuralnetworkA = false;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
    if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(0, 0, 0, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(0, 255, 0, 255));
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool is_active = ImGui::IsItemActive();   // Held
    const bool is_clicked = ImGui::IsItemClicked();
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
    if (is_clicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        points.push_back(mouse_pos_in_canvas);
    const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
        ImGui::OpenPopupOnItemClick("context");
    if (ImGui::BeginPopup("context"))
    {
        if (adding_line)
            points.resize(points.size() - 2);
        adding_line = false;
        if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 1); }
        if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); neuralnetworkA = false; }
        ImGui::EndPopup();
    }
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    if (opt_enable_grid)
    {
        const float GRID_STEP = 64.0f;
        for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
    }
    //for (int n = 0; n + 1 < points.Size; n += 1)
        //draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 255, 255), 2.0f);
    for (int n = 0; n < points.Size; n++)
        draw_list->AddRectFilled(ImVec2(origin.x + points[n].x - 2.0, origin.y + points[n].y - 2.0), ImVec2(origin.x + points[n].x + 2.0, origin.y + points[n].y + 2.0), IM_COL32(255, 255, 255, 255));

    int n = points.size();

    if (n >= 2)
    {
        if (polynomialI) {
            Eigen::VectorXd an = interpolationPolynomial(points);
            ImVec2 tempP = points[0];
            for (double x = points[0].x + 1; x <= points.back().x; x++) {
                double y = 0;
                for (int j = 0; j < n; j++)
                    y += an[j] * pow(x, j);
                //draw_list->AddRectFilled(ImVec2(origin.x + x * 1000 - 1.0, origin.y + y * 1000 - 1.0), ImVec2(origin.x + x * 1000 + 1.0, origin.y + y * 1000 + 1.0), IM_COL32(0, 255, 255, 255));
                draw_list->AddLine(ImVec2(origin.x + tempP.x, origin.y + tempP.y), ImVec2(origin.x + x, origin.y + y), IM_COL32(255, 0, 0, 255));
                tempP = ImVec2(x, y);
            }
        }

        if (gaussI) {
            Eigen::VectorXf an = interpolationGauss(points, sigma);
            ImVec2 tempP = points[0];
            for (double x = points[0].x + 1; x <= points.back().x; x++) {
                double y = an[0];
                for (int j = 1; j <= n; j++)
                    y += an[j] * gauss(x, points[j - 1].x, sigma);
                //draw_list->AddRectFilled(ImVec2(origin.x + x * 1000 - 1.0, origin.y + y * 1000 - 1.0), ImVec2(origin.x + x * 1000 + 1.0, origin.y + y * 1000 + 1.0), IM_COL32(0, 255, 255, 255));
                draw_list->AddLine(ImVec2(origin.x + tempP.x, origin.y + tempP.y), ImVec2(origin.x + x, origin.y + y), IM_COL32(0, 255, 0, 255));
                tempP = ImVec2(x, y);
            }
        }

        if (lagrangeI) {
            Eigen::MatrixXf D(n, n);
            Eigen::VectorXf DD(n);
            Eigen::VectorXf U(n);
            Eigen::VectorXf UU(n);
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    D(i, j) = points[i].x - points[j].x;
            for (int i = 0; i < n; i++)
            {
                DD[i] = 1.0;
                for (int j = 0; j < n; j++)
                    if (j != i)
                        DD[i] *= D(i, j);
            }

            ImVec2 tempP = points[0];
            for (int x = points[0].x + 1; x <= points.back().x; x = x + 1)
            {
                for (int i = 0; i < n; i++)
                    U[i] = x - points[i].x;

                int y = 0;
                for (int i = 0; i < n; i++)
                {
                    UU[i] = 1.0;
                    for (int j = 0; j < n; j++)
                        if (j != i)
                            UU[i] *= U[j];
                    y += points[i].y * UU[i] / DD[i];
                }
                //draw_list->AddRectFilled(ImVec2(origin.x + x - 1.0, origin.y + y - 1.0), ImVec2(origin.x + x + 1.0, origin.y + y + 1.0), IM_COL32(255, 255, 255, 255));
                draw_list->AddLine(ImVec2(origin.x + tempP.x, origin.y + tempP.y), ImVec2(origin.x + x, origin.y + y), IM_COL32(0, 255, 255, 255));
                tempP = ImVec2(x, y);
            }
        }

        if (bernsteinI) {
            auto points2 = points;
            for (int i = 0; i < n; i++)
                points2[i] = ImVec2(points[i].x / 1000.0, points[i].y / 1000.0);
            Eigen::VectorXd an = interpolationBernstein(points2);
            ImVec2 tempP = points2[0];
            for (double x = points2[0].x + 0.001; x <= points2.back().x; x = x + 0.001)
            {
                double y = 0.0;
                for (int j = 0; j < n; j++)
                    y += an[j] * zuhe(n - 1, j) * pow(x, j) * pow(1 - x, n - 1 - j);
                //draw_list->AddRectFilled(ImVec2(origin.x + x - 1.0, origin.y + y - 1.0), ImVec2(origin.x + x + 1.0, origin.y + y + 1.0), IM_COL32(255, 255, 255, 255));
                draw_list->AddLine(ImVec2(origin.x + tempP.x * 1000, origin.y + tempP.y * 1000), ImVec2(origin.x + x * 1000, origin.y + y * 1000), IM_COL32(255, 255, 255, 255));
                tempP = ImVec2(x, y);
            }
        }

        if (leastSquareA) {
            Eigen::VectorXf an = approximation_LeastSquare(points, order);
            int x = points[0].x, y = 0;
            for (int j = 0; j <= order; j++)
                y += an[j] * pow(x, j);
            ImVec2 tempP(x, y);
            for (int x = points[0].x + 1; x <= points.back().x; x++) {
                int y = 0;
                for (int j = 0; j <= order; j++)
                    y += an[j] * pow(x, j);
                //draw_list->AddRectFilled(ImVec2(origin.x + x - 1.0, origin.y + y - 1.0), ImVec2(origin.x + x + 1.0, origin.y + y + 1.0), IM_COL32(255, 255, 0, 255));
                draw_list->AddLine(ImVec2(origin.x + tempP.x, origin.y + tempP.y), ImVec2(origin.x + x, origin.y + y), IM_COL32(255, 255, 0, 255));
                tempP = ImVec2(x, y);
            }
        }

        if (ridgeRegressionA) {
            Eigen::VectorXf an = approximation_RidgeRegression(points, order, lambda);
            int x = points[0].x, y = 0;
            for (int j = 0; j <= order; j++)
                y += an[j] * pow(x, j);
            ImVec2 tempP(x, y);
            for (int x = points[0].x + 1; x <= points.back().x; x++) {
                int y = 0;
                for (int j = 0; j <= order; j++)
                    y += an[j] * pow(x, j);
                //draw_list->AddRectFilled(ImVec2(origin.x + x - 1.0, origin.y + y - 1.0), ImVec2(origin.x + x + 1.0, origin.y + y + 1.0), IM_COL32(255, 255, 0, 255));
                draw_list->AddLine(ImVec2(origin.x + tempP.x, origin.y + tempP.y), ImVec2(origin.x + x, origin.y + y), IM_COL32(255, 0, 255, 255));
                tempP = ImVec2(x, y);
            }
        }

        if (neuralnetworkA) {
            double x = points2[0].x;
            Eigen::VectorXd r1 = x * a + b;
            for (int i = 0; i < r1.size(); i++)
                r1[i] = gauss(r1[i]);
            double y = (r1.transpose() * W)[0] + B;
            ImVec2 tempP(x, y);
            for (double x = points2[0].x + 0.01; x <= points2.back().x; x += 0.01) {
                r1 = x * a + b;
                for (int i = 0; i < r1.size(); i++)
                    r1[i] = gauss(r1[i]);
                double y = (r1.transpose() * W)[0] + B;
                //draw_list->AddRectFilled(ImVec2(origin.x + x * 100 - 1.0, origin.y + y * 100 - 1.0), ImVec2(origin.x + x * 100 + 1.0, origin.y + y * 100 + 1.0), IM_COL32(0, 255, 0, 255));
                draw_list->AddLine(ImVec2(origin.x + tempP.x * 100, origin.y + tempP.y * 100), ImVec2(origin.x + x * 100, origin.y + y * 100), IM_COL32(179, 63, 244, 255));
                tempP = ImVec2(x, y);
            }
        }
    }
    draw_list->PopClipRect();
    ImGui::End();

    ImGui::Begin("points:");
    for (int i = 0; i < points.size(); i++)
        ImGui::Text("%f...%f", points[i].x, points[i].y);
    ImGui::End();

    ImGui::Begin("neural network:");
    for (int i = 0; i < losses.size(); i++)
        if (i % (loops / 10) == 0)
            ImGui::Text("%d...%f", i / (loops / 10) + 1, losses[i]);
    ImGui::End();
}