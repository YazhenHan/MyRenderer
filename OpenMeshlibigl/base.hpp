#pragma once
#ifndef BASE_HPP
#define BASE_HPP

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <Eigen/Dense>

typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;
void openMesh_to_igl(MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F);
double calc_angle(MyMesh::HalfedgeHandle he, MyMesh& mesh);
#endif // !BASE_HPP
