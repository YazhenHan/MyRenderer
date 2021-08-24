#include <iostream>

#include "base.hpp"

void openmeshtoigl(const MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F) {
	std::cout << mesh.n_vertices() << " " << mesh.n_faces() << std::endl;
	V.setZero(mesh.n_vertices(), 3);
	F.setZero(mesh.n_faces(), 3);
	int i = 0;
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++, i++) {
		V(i, 0) = mesh.point(*v_it)[0];
		V(i, 1) = mesh.point(*v_it)[1];
		V(i, 2) = mesh.point(*v_it)[2];
	}
	i = 0;
	for (auto f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++, i++) {
		int j = 0;
		for (auto fv_it = mesh.cfv_begin(*f_it); fv_it != mesh.cfv_end(*f_it); fv_it++, j++)
			F(i, j) = fv_it->idx();
	}
}