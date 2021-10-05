#include "base.hpp"
#include "topics.hpp"
#include <OpenMesh/Core/Utils/PropertyManager.hh>
#include <Eigen/Sparse>
#include <queue>

void simple_mesh_smoother(MyMesh& mesh) {
	auto cog = OpenMesh::VProp<MyMesh::Point>(mesh);
	for (const auto& vh : mesh.vertices()) {
		cog[vh] = { 0,0,0 };
		int valence = 0;
		for (const auto& vvh : mesh.vv_range(vh)) {
			cog[vh] += mesh.point(vvh);
			++valence;
		}
		cog[vh] /= valence;
	}
	for (const auto& vh : mesh.vertices()) {
		mesh.point(vh) = cog[vh];
	}
}

void half_angle(MyMesh& mesh, OpenMesh::HProp<double>& h_angle) {
	for (auto& h : mesh.halfedges()) {
		if (h.is_boundary()) continue;
		MyMesh::Point p = mesh.point(h.next().to()), a = mesh.point(h.from()), b = mesh.point(h.to());
		OpenMesh::Vec3d pa = a - p, pb = b - p;
		h_angle[h] = std::acos(pa.dot(pb) / (pa.norm() * pb.norm()));
	}
}
void laplace_coordinate(MyMesh& mesh, OpenMesh::VProp<OpenMesh::Vec3d>& hn) {
	auto h_angle = OpenMesh::HProp<double>(mesh);
	half_angle(mesh, h_angle);
	for (auto& v : mesh.vertices()) {
		if (v.is_boundary()) continue;
		OpenMesh::Vec3d temp(0.0, 0.0, 0.0);
		double A = 0.0;
		for (auto ve : v.outgoing_halfedges()) {
			double w = 1.0 / std::tan(h_angle[ve]) + 1.0 / std::tan(h_angle[ve.opp()]); w = std::max(w, 0.0); w = std::min(w, 10.0);
			OpenMesh::Vec3d vo = mesh.point(ve.to()) - mesh.point(v);
			temp += w * vo;
			A += w * vo.sqrnorm();
		}
		A /= 8.0; temp /= (4 * A);
		hn[v] = temp;
	}
}
void local_laplace_smoother(MyMesh& mesh, double lambda, int in_num) {
	auto hn = OpenMesh::VProp<OpenMesh::Vec3d>(mesh);
	for (int i = 0; i < in_num; i++) {
		laplace_coordinate(mesh, hn);
		for (auto& v : mesh.vertices())
			if (!v.is_boundary())
				mesh.point(v) += hn[v] * lambda;
	}
}

void global_laplace_smoother(MyMesh& mesh) {
	Eigen::SparseMatrix<double> A(mesh.n_vertices(), mesh.n_vertices());
	Eigen::VectorXd b_x(mesh.n_vertices());
	Eigen::VectorXd b_y(mesh.n_vertices());
	Eigen::VectorXd b_z(mesh.n_vertices());
	std::vector<Eigen::Triplet<double>> tv;

	auto h_angle = OpenMesh::HProp<double>(mesh);
	half_angle(mesh, h_angle);
	// fill the coefficient matrix
	for (auto& v : mesh.vertices()) {
		// if if boundary, a_i = 1;
		if (v.is_boundary()) {
			Eigen::Triplet<double> temp(v.idx(), v.idx(), 1.0f);
			tv.push_back(temp);
			b_x[v.idx()] = mesh.point(v)[0];
			b_y[v.idx()] = mesh.point(v)[1];
			b_z[v.idx()] = mesh.point(v)[2];
		}
		else {
			double count = 0.0;
			// negative number of the sum of the 1-ring coefficient
			for (auto ve : v.outgoing_halfedges()) {
				double w = 1.0 / std::tan(h_angle[ve]) + 1.0 / std::tan(h_angle[ve.opp()]);
				Eigen::Triplet<double> temp(v.idx(), ve.to().idx(), w);
				tv.push_back(temp);
				count += w;
			}
			Eigen::Triplet<double> temp(v.idx(), v.idx(), -count);
			tv.push_back(temp);
			b_x[v.idx()] = 0.0;
			b_y[v.idx()] = 0.0;
			b_z[v.idx()] = 0.0;
		}
	}

	// solve u, v
	A.setFromTriplets(tv.begin(), tv.end());
	Eigen::SparseLU<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> solver;
	solver.analyzePattern(A);
	solver.factorize(A);
	Eigen::VectorXd x = solver.solve(b_x);
	Eigen::VectorXd y = solver.solve(b_y);
	Eigen::VectorXd z = solver.solve(b_z);
	for (auto& v : mesh.vertices())
		mesh.point(v) = MyMesh::Point(x(v.idx()), y(v.idx()), z(v.idx()));
}

void loop_subdivision(MyMesh& mesh) {
	MyMesh ans;

	{
		auto old_vertex_vhandle = OpenMesh::VProp<MyMesh::VertexHandle>(mesh);
		auto edge_flag = OpenMesh::EProp<bool>(mesh);
		auto edge_vhandle = OpenMesh::EProp<MyMesh::VertexHandle>(mesh);
		for (auto& v_it : mesh.vertices()) {
			MyMesh::Point temp_point = { 0, 0, 0 };
			if (v_it.is_boundary()) {
				for (auto vv_it : v_it.vertices())
					if (vv_it.is_boundary())
						temp_point += mesh.point(vv_it);
				temp_point = (1.0 / 8.0) * temp_point + (3.0 / 4.0) * mesh.point(v_it);
			}
			else {
				float u = v_it.valence() == 3 ? 3.0 / 16.0 : 3.0 / (8.0 * v_it.valence());
				for (auto vv_it : v_it.vertices())
					temp_point += mesh.point(vv_it);
				temp_point = (1 - v_it.valence() * u) * mesh.point(v_it) + u * temp_point;
			}
			old_vertex_vhandle[v_it] = ans.add_vertex(temp_point);
		}
		for (auto& e_it : mesh.edges()) edge_flag[e_it] = false;
		for (auto& f_it : mesh.faces()) {
			std::vector<MyMesh::VertexHandle> face;
			auto temp = f_it.halfedge();
			do {
				if (edge_flag[temp.edge()])
					face.push_back(edge_vhandle[temp.edge()]);
				else {
					if (temp.is_boundary())
						face.push_back(ans.add_vertex(0.5 * (mesh.point(temp.from()) + mesh.point(temp.to()))));
					else
						face.push_back(ans.add_vertex((3.0 / 8.0) * (mesh.point(temp.from()) + mesh.point(temp.to())) +
							(1.0 / 8.0) * (mesh.point(temp.next().to()) + mesh.point(temp.opp().next().to()))));
					edge_flag[temp.edge()] = true;
					edge_vhandle[temp.edge()] = face.back();
				}
				temp = temp.next();
			} while (temp != f_it.halfedge());
			int i = 0;
			do {
				ans.add_face({ face[i], old_vertex_vhandle[temp.to()], face[(i + 1) % face.size()] });
				i = (i + 1) % face.size();
				temp = temp.next();
			} while (temp != f_it.halfedge());
			ans.add_face(face);
		}
	}

	mesh = ans;
}

struct edge_Collapse_structure
{
	MyMesh::HalfedgeHandle hf;
	MyMesh::Point np;
	MyMesh::VertexHandle vfrom;
	MyMesh::VertexHandle vto;

	// 判断该点对是否已被更新过的点对取代
	int vto_flag = 0;
	int vfrom_flag = 0;
	Eigen::Matrix4f Q_new;
	float cost;
	bool operator < (const edge_Collapse_structure& a) const {
		return cost > a.cost;
	}
};
void qem_simplification(MyMesh& mesh) {
	int it_num = mesh.n_vertices() / 2;
	//1. Compute the Q matrices for all the initial vertices
	auto Q = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Matrix4f>(mesh);
	auto v = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Vector4f>(mesh);
	auto flag = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, int>(mesh); //与vto_flag vfrom_flag配合判断点对是否还有效
	auto p = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, Eigen::Vector4f>(mesh);
	mesh.request_face_normals();
	mesh.update_face_normals();
	for (MyMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
	{
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
	for (MyMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
	{
		Eigen::Matrix4f mat;
		mat.setZero();
		for (MyMesh::VertexFaceIter vf_it = mesh.vf_iter(*vit); vf_it.is_valid(); ++vf_it)
		{
			mat += p[*vf_it] * p[*vf_it].transpose();
		}
		Q[*vit] = mat;
		v[*vit][0] = mesh.point(*vit)[0];
		v[*vit][1] = mesh.point(*vit)[1];
		v[*vit][2] = mesh.point(*vit)[2];
		v[*vit][3] = 1.0f;
		flag[*vit] = 0;
	}
	// 2. Select all valid pairs (only vertices in an edge are considered)
	// 3. Compute the optimal contraction target
	std::priority_queue <edge_Collapse_structure, std::vector<edge_Collapse_structure>, std::less<edge_Collapse_structure>> q;
	for (MyMesh::EdgeIter eit = mesh.edges_begin(); eit != mesh.edges_end(); ++eit)
	{
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
		{
			vnew = tQ.inverse() * b;
		}
		// else take the midpoint
		else
		{
			vnew = (v[eit->v0()] + v[eit->v1()]) / 2.0f;
		}
		//std::cout << vnew << std::endl;
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
		if (mesh.is_collapse_ok(s.hf))
		{
			mesh.collapse(s.hf);
			tvh = s.vto;
			flag[s.vto] ++;
			flag[s.vfrom] ++;
		}

		else if (mesh.is_collapse_ok(mesh.opposite_halfedge_handle(s.hf)))
		{
			mesh.collapse(mesh.opposite_halfedge_handle(s.hf));
			tvh = s.vfrom;
			flag[s.vto] ++;
			flag[s.vfrom] ++;
		}
		else
		{
			continue;
		}

		mesh.set_point(tvh, s.np);
		Q[tvh] = s.Q_new;
		v[tvh][0] = s.np[0];
		v[tvh][1] = s.np[1];
		v[tvh][2] = s.np[2];
		v[tvh][3] = 1.0f;
		for (MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(tvh); vh_it.is_valid(); ++vh_it)
		{
			MyMesh::VertexHandle tt = vh_it->to();
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
			if (lu.isInvertible())
			{
				vnew = tQ.inverse() * b;
			}
			// else take the midpoint
			else
			{
				vnew = (v[tvh] + v[tt]) / 2.0f;
			}
			//std::cout << vnew << std::endl;
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

void laplace_editing(MyMesh& mesh) {}