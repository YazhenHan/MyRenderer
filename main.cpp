//#include "glfwwindow.hpp"
//
//int main()
//{
//    GlfwWindow glfwWindow;
//    Shader modelLoadingShader("universal.vs", "face.fs");
//    Shader pointShader("universal.vs", "point.fs");
//    Shader lineShader("universal.vs", "line.fs");
//    Model ourModel;
//    glfwWindow.draw(ourModel, modelLoadingShader, pointShader, lineShader);
//    glfwWindow.cleanUp();
//    return 0;
//}

#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>

Eigen::MatrixXd V;
Eigen::MatrixXi F;

int main()
{
    igl::readOBJ("assets/diablo3_pose.obj", V, F);
    igl::opengl::glfw::Viewer viewer;
    viewer.data().set_mesh(V, F);
    viewer.launch();
    return 0;
}