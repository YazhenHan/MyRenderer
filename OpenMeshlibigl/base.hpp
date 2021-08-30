#pragma once
#include <OpenMesh/Core/Mesh/DefaultTriMesh.hh>
#include <Eigen/Core>
#include <igl/opengl/glfw/Viewer.h>

typedef OpenMesh::TriMesh MyMesh;

void openmesh_igl(const MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F);

void display_mesh(const MyMesh& mesh, igl::opengl::glfw::Viewer& viewer);