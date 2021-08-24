#pragma once
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <Eigen/Dense>

typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;

void openmeshtoigl(const MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F);