#include <iostream>
#include <vector>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
// -------------------- libigl
#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>

#include "base.hpp"
#include "topics.hpp"

MyMesh tm;
Eigen::MatrixXd V;
Eigen::MatrixXi F;

MyMesh mesh;

bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifier)
{
	if (key == '1')
	{
		tm = simple_mesh_smoother(tm);
		openmeshtoigl(tm, V, F);
		viewer.data().clear();
		viewer.data().set_mesh(V, F);
		viewer.core().align_camera_center(V, F);
	}
	if (key == 'r' || key == 'R')
	{
		tm = mesh;
		openmeshtoigl(tm, V, F);
		viewer.data().clear();
		viewer.data().set_mesh(V, F);
		viewer.core().align_camera_center(V, F);
	}
	return false;
}

int main(int argc, char* argv[])
{
	OpenMesh::IO::read_mesh(mesh, "assets/models/Nefertiti_face.obj");
	tm = mesh;

	openmeshtoigl(tm, V, F);
	OpenMesh::IO::write_mesh(tm, "output.obj");

	igl::opengl::glfw::Viewer viewer;
	viewer.callback_key_down = &key_down;
	viewer.data().set_mesh(V, F);
	viewer.launch();

	return 0;
}