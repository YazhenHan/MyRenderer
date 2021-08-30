#include <iostream>
#include <vector>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
// -------------------- libigl
#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>

#include "base.hpp"
#include "topics.hpp"

MyMesh mesh, pm;

bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifier) {
	switch (key)
	{
	case '0':
		mesh = pm;
		OpenMesh::IO::write_mesh(mesh, "output.obj");
		display_mesh(mesh, viewer);
		break;
	case '1':
		simple_mesh_smoother(mesh);
		OpenMesh::IO::write_mesh(mesh, "simple_smoother.obj");
		display_mesh(mesh, viewer);
		break;
	case '2':
		local_laplace_smoother(mesh);
		OpenMesh::IO::write_mesh(mesh, "local_laplace_smoother.obj");
		display_mesh(mesh, viewer);
		break;
	case '3':
		global_laplace_smoother(mesh);
		OpenMesh::IO::write_mesh(mesh, "global_laplace_smoother.obj");
		display_mesh(mesh, viewer);
		break;
	case '4':
		loop_subdivision(mesh);
		OpenMesh::IO::write_mesh(mesh, "loop_sub.obj");
		display_mesh(mesh, viewer);
		break;
	case '5':
		qem_simplification(mesh);
		OpenMesh::IO::write_mesh(mesh, "qem_sim.obj");
		display_mesh(mesh, viewer);
		break;
	default:
		break;
	}
	return false;
}

int main(int argc, char* argv[])
{
	OpenMesh::IO::read_mesh(mesh, "../assets/models/african_head.obj");
	pm = mesh;
	igl::opengl::glfw::Viewer viewer;
	display_mesh(mesh, viewer);
	viewer.callback_key_down = &key_down;
	viewer.launch();

	return 0;
}