#include "base.hpp"
#include "topics.hpp"

MyMesh simple_mesh_smoother(MyMesh mesh) {
	std::vector<MyMesh::Point> cogs;
	cogs.reserve(mesh.n_vertices());
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
		MyMesh::Point cog;
		cog[0] = cog[1] = cog[2] = 0.0;
		for (auto vv_it = mesh.vv_iter(*v_it); vv_it.is_valid(); ++vv_it)
			cog += mesh.point(*vv_it);
		cogs.push_back(cog / v_it->valence());
	}
	auto cog_it = cogs.begin();
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it, ++cog_it)
		if (!mesh.is_boundary(*v_it))
			mesh.set_point(*v_it, *cog_it);
	return mesh;
}

MyMesh loop_subdivision(MyMesh mesh) {

}

MyMesh qem_simplification(MyMesh mesh) {

}

MyMesh local_minimal_surface(MyMesh mesh, float lambda = 0.001, int in_num = 100) {

}

MyMesh global_minimal_surface(MyMesh mesh) {

}