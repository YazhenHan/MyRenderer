#include <iostream>
#include <vector>
#include <eigen/Core>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
// -------------------- libigl
#include <igl/opengl/glfw/Viewer.h>

#include "base.hpp"
#include "topics.hpp"

const int cache_size = 10;
Eigen::MatrixXd V[cache_size], tV;
Eigen::MatrixXi F[cache_size], tF;
bool flag[cache_size] = { false };
int ti = 1;
MyMesh mesh, tm;
//Eigen::MatrixXd V, tV;
//Eigen::MatrixXi F, tF;

bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifier) {
    switch (key)
    {
    case '1':
        tm = global_minimal_surface(mesh);
        openMesh_to_igl(tm, tV, tF);
        viewer.data().clear();
        viewer.data().set_mesh(tV, tF);
        viewer.core().align_camera_center(tV, tF);
        break;
    case '2':
        tm = local_minimal_surface(tm);
        openMesh_to_igl(tm, tV, tF);
        viewer.data().clear();
        viewer.data().set_mesh(tV, tF);
        viewer.core().align_camera_center(tV, tF);
        break;
    case '3':
        if (ti < cache_size) {
            if (!flag[ti]) {
                qemSimplification(mesh, 0.5);
                std::cout << "vertices: " << mesh.n_vertices() << std::endl;
                std::cout << "faces: " << mesh.n_faces() << std::endl;
                openMesh_to_igl(mesh, V[ti], F[ti]);
                flag[ti] = true;
            }
            viewer.data().clear();
            viewer.data().set_mesh(V[ti], F[ti]);
            viewer.core().align_camera_center(V[ti], F[ti]);
            ti++;
        }
        break;
    case '4':
        if (ti > 1) {
            ti--;
            viewer.data().clear();
            viewer.data().set_mesh(V[ti], F[ti]);
            viewer.core().align_camera_center(V[ti], F[ti]);
        }
        break;
    case 'r':
    case 'R':
        tm = mesh;
        viewer.data().clear();
        viewer.data().set_mesh(V[0], F[0]);
        viewer.core().align_camera_center(V[0], F[0]);
        break;
    default:
        break;
    }
    return false;
}

int main(int argc, char** argv)
{
    mesh.request_vertex_normals();
    mesh.request_face_normals();
    // read mesh from stdin
    OpenMesh::IO::read_mesh(mesh, "../assets/Nefertiti_face.obj");
    tm = mesh;
    mesh.update_face_normals();
    mesh.update_vertex_normals();

    igl::opengl::glfw::Viewer viewer;
    viewer.callback_key_down = &key_down;
    //V.setZero(mesh.n_vertices(), 3);
    //F.setZero(mesh.n_faces(), 3);
    openMesh_to_igl(mesh, V[0], F[0]);
    viewer.data().set_mesh(V[0], F[0]);
    viewer.launch();

    // write mesh to stdout
    if (!OpenMesh::IO::write_mesh(mesh, "output.obj"))
    {
        std::cerr << "Error: cannot write mesh to output.obj" << std::endl;
        return 1;
    }
    return 0;
}