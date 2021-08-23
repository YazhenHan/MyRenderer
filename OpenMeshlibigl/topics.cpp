#include "topics.hpp"
#include <OpenMesh/Core/Utils/PropertyManager.hh>
#include <Eigen/Sparse>
#include <queue>

struct edge_Collapse_structure {
	MyMesh::HalfedgeHandle hf;
	MyMesh::Point np;
	MyMesh::VertexHandle vto;
	MyMesh::VertexHandle vfrom;

	// 下面两个用来判断该点对是否已被更新过的点对取代
	int vto_flag = 0;
	int vfrom_flag = 0;
	Eigen::Matrix4f Q_new;
	float cost;
	bool operator < (const edge_Collapse_structure& a) const {
		return cost > a.cost;
	}
};

void qemSimplification(MyMesh& mesh, float ratio) {
	assert(ratio >= 0 && ratio <= 1);
	int it_num = (1.0f - ratio) * mesh.n_vertices();

	// 1. Compute the Q matrix for all the initial vertices;
	auto Q = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Matrix4f>(mesh);
	auto v = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Vector4f>(mesh);
	auto flag = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, int>(mesh); // 与vto_flag vfrom_flag配合判断点对是否还有效
	auto p = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, Eigen::Vector4f>(mesh);

	for (auto fit = mesh.faces_begin(); fit != mesh.faces_end(); fit++) {
		float a, b, c, d;
		a = mesh.normal(*fit)[0];
		b = mesh.normal(*fit)[1];
		c = mesh.normal(*fit)[2];
		MyMesh::Point tp = mesh.point(fit->halfedge().to());
		d = -(a * tp[0] + b * tp[1] + c * tp[2]);
		p[*fit][0] = a;
		p[*fit][1] = b;
		p[*fit][2] = c;
		p[*fit][3] = d;
	}
	for (auto vit = mesh.vertices_begin(); vit != mesh.vertices_end(); vit++) {
		Eigen::Matrix4f mat;
		mat.setZero();
		for (auto vf_it = mesh.vf_iter(*vit); vf_it.is_valid(); vf_it++)
			mat += p[*vf_it] * p[*vf_it].transpose();
		Q[*vit] = mat;
		v[*vit][0] = mesh.point(*vit)[0];
		v[*vit][1] = mesh.point(*vit)[1];
		v[*vit][2] = mesh.point(*vit)[2];
		v[*vit][3] = 1.0f;
		flag[*vit] = 0;
	}
	// 2. Select all valid pairs (only vertices in an edge are considered)
	// 3. Compute the optimal contraction target
	std::priority_queue<edge_Collapse_structure, std::vector<edge_Collapse_structure>, std::less<edge_Collapse_structure>> q;
	for (auto eit = mesh.edges_begin(); eit != mesh.edges_end(); eit++) {
		Eigen::Matrix4f newQ = Q[eit->v0()] + Q[eit->v1()];
		Eigen::Matrix4f tQ = newQ;
		Eigen::Vector4f b(0.0f, 0.0f, 0.0f, 1.0f);
		tQ(3, 0) = 0.0f;
		tQ(3, 1) = 0.0f;
		tQ(3, 2) = 0.0f;
		tQ(3, 3) = 1.0f;
		Eigen::FullPivLU<Eigen::Matrix4f> lu(tQ);
		Eigen::Vector4f vnew;
		// if is invertible, solve the linear equation
		if (lu.isInvertible())
			vnew = tQ.inverse() * b;
		// else take the midpoint
		else
			vnew = (v[eit->v0()] + v[eit->v1()]) / 2.0f;
		edge_Collapse_structure ts;
		ts.hf = eit->halfedge(0);
		ts.cost = vnew.transpose() * newQ * vnew;
		MyMesh::Point np(vnew[0], vnew[1], vnew[2]);
		ts.np = np;
		ts.vto = eit->halfedge(0).to();
		ts.vfrom = eit->halfedge(0).from();
		ts.Q_new = newQ;
		q.push(ts);
	}
	mesh.request_vertex_status();
	mesh.request_edge_status();
	mesh.request_face_status();

	int i = 0;
	while (i < it_num)
	{
		edge_Collapse_structure s = q.top();
		q.pop();
		if (mesh.status(s.vfrom).deleted() || mesh.status(s.vto).deleted())
			continue;
		if (s.vto_flag != flag[s.vto] || s.vfrom_flag != flag[s.vfrom])
			continue;
		MyMesh::VertexHandle tvh;
		if (mesh.is_collapse_ok(s.hf)) {
			mesh.collapse(s.hf);
			tvh = s.vto;
			flag[s.vto]++;
			flag[s.vfrom]++;
		}
		else if (mesh.is_collapse_ok(mesh.opposite_halfedge_handle(s.hf))) {
			mesh.collapse(mesh.opposite_halfedge_handle(s.hf));
			tvh = s.vfrom;
			flag[s.vto]++;
			flag[s.vfrom]++;
		}
		else
			continue;
		mesh.set_point(tvh, s.np);
		Q[tvh] = s.Q_new;
		v[tvh][0] = s.np[0];
		v[tvh][1] = s.np[1];
		v[tvh][2] = s.np[2];
		v[tvh][3] = 1.0f;
		for (auto vh_it = mesh.voh_iter(tvh); vh_it.is_valid(); vh_it++) {
			auto tt = vh_it->to();
			Eigen::Matrix4f newQ = s.Q_new + Q[tt];
			Eigen::Matrix4f tQ = newQ;
			Eigen::Vector4f b(0.0f, 0.0f, 0.0f, 1.0f);
			tQ(3, 0) = 0.0f;
			tQ(3, 1) = 0.0f;
			tQ(3, 2) = 0.0f;
			tQ(3, 3) = 1.0f;
			Eigen::FullPivLU<Eigen::Matrix4f> lu(tQ);
			Eigen::Vector4f vnew;
			// if is invertible, solve the linear equation
			if (lu.isInjective())
				vnew = tQ.inverse() * b;
			// else take the midpoint
			else
				vnew = (v[tvh] + v[tt]) / 2.0f;
			edge_Collapse_structure ts;
			ts.hf = *vh_it;
			ts.cost = vnew.transpose() * newQ * vnew;
			MyMesh::Point np(vnew[0], vnew[1], vnew[2]);
			ts.np = np;
			ts.vto = tt;
			ts.vto_flag = flag[tt];
			ts.vfrom = tvh;
			ts.vfrom_flag = flag[tvh];
			ts.Q_new = newQ;
			q.push(ts);
		}
		i++;
	}
	mesh.garbage_collection();
}

std::vector<MyMesh::Point> calc_Hn(MyMesh& mesh) {
	std::vector<MyMesh::Point> ret(mesh.n_vertices());
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
		double area = 0.0;
		// 1-ring
		MyMesh::Point tp(0.0f, 0.0f, 0.0f);
		for (auto vh_it = mesh.voh_iter(*v_it); vh_it.is_valid(); ++vh_it) {
			double w = 0.0f;
			w += 1.0f / tan(calc_angle(*vh_it, mesh));

			if (vh_it->opp().is_valid())
				w += 1.0f / tan(calc_angle(vh_it->opp(), mesh));
			if (w < 0) w = 0;
			if (w > 10) w = 10;
			tp += w * (mesh.point(*v_it) - mesh.point(vh_it->to()));
			area += w * OpenMesh::sqrnorm(mesh.point(*v_it) - mesh.point(vh_it->to()));
		}
		area /= 8.0;
		tp /= (-4 * area);
		ret[v_it->idx()] = tp;
	}
	return ret;
}
MyMesh local_minimal_surface(MyMesh mesh, float lambda, int it_num) {
	for (int it = 0; it < it_num; it++) {
		std::vector<MyMesh::Point> Hn = calc_Hn(mesh);
		// for all vertices
		for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
			// fix boundary
			if (mesh.is_boundary(*v_it)) continue;
			// update the vertex position
			mesh.point(*v_it) += lambda * Hn[v_it->idx()];
		}
	}
	return mesh;
}

MyMesh global_minimal_surface(MyMesh mesh) {
	Eigen::SparseMatrix<float> A(mesh.n_vertices(), mesh.n_vertices());
	Eigen::SparseLU<Eigen::SparseMatrix<float>, Eigen::COLAMDOrdering<int>> solver;
	std::vector<Eigen::Triplet<float>> tv;
	Eigen::VectorXf b_x(mesh.n_vertices());
	Eigen::VectorXf b_y(mesh.n_vertices());
	Eigen::VectorXf b_z(mesh.n_vertices());

	// fill the coefficient matrix
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
		// if is boundary, a_i = 1
		if (mesh.is_boundary(*v_it)) {
			Eigen::Triplet<float> temp((*v_it).idx(), (*v_it).idx(), 1.0f);
			tv.push_back(temp);
			b_x[(*v_it).idx()] = mesh.point(*v_it)[0];
			b_y[(*v_it).idx()] = mesh.point(*v_it)[1];
			b_z[(*v_it).idx()] = mesh.point(*v_it)[2];
		}
		else {
			float count = 0;
			// negative number of the sum of the 1-ring coefficient
			for (auto vh_it = mesh.voh_iter(*v_it); vh_it.is_valid(); vh_it++) {
				// calc cot weight
				float w1 = 1.0f / tanf(calc_angle(*vh_it, mesh));
				float w2 = 1.0f / tanf(calc_angle((*vh_it).opp(), mesh));
				float w = w1 + w2;
				Eigen::Triplet<float> temp((*v_it).idx(), (*vh_it).to().idx(), w);
				tv.push_back(temp);
				count += w;
			}
			Eigen::Triplet<float> temp((*v_it).idx(), (*v_it).idx(), -count);
			tv.push_back(temp);
			b_x[(*v_it).idx()] = 0.0f;
			b_y[(*v_it).idx()] = 0.0f;
			b_z[(*v_it).idx()] = 0.0f;
		}
	}

	// solve u, v
	A.setFromTriplets(tv.begin(), tv.end());
	solver.analyzePattern(A);
	solver.factorize(A);
	Eigen::VectorXf x = solver.solve(b_x);
	Eigen::VectorXf y = solver.solve(b_y);
	Eigen::VectorXf z = solver.solve(b_z);
	for (auto v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
		if (!v_it->is_boundary()) {
			int idx = (*v_it).idx();
			MyMesh::Point tp(x(idx), y(idx), z(idx));
			mesh.set_point(*v_it, tp);
		}
	}
	return mesh;
}