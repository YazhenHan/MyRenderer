#pragma once

#include <glm/glm.hpp>
#include <utility>
#include <vector>
#include <Eigen/Dense>

struct HE_Vert;
struct HE_Edge;
struct HE_Face;

struct Vertex {
	// position
	glm::vec3 Position = { 0.0, 0.0, 0.0 };
	// texCoords
	glm::vec2 TexCoords = { 0.0, 0.0 };
	// normal
	glm::vec3 Normal = { 0.0, 0.0, 0.0 };
	// tangent
	glm::vec3 Tangent = { 0.0, 0.0, 0.0 };
	// bitangent
	glm::vec3 Bitangent = { 0.0, 0.0, 0.0 };
};

glm::vec3 operator*(const glm::vec3& v, float k) {
	return glm::vec3(v[0] * k, v[1] * k, v[2] * k);
}

glm::vec2 operator*(const glm::vec2& v, float k) {
	return glm::vec2(v[0] * k, v[1] * k);
}

Vertex operator+(const Vertex& v1, const Vertex& v2) {
	Vertex v;
	v.Position = v1.Position + v2.Position;
	v.Normal = v1.Normal + v2.Normal;
	v.TexCoords = v1.TexCoords + v2.TexCoords;
	v.Tangent = v1.Tangent + v2.Tangent;
	v.Bitangent = v1.Bitangent + v2.Bitangent;
	return v;
}

Vertex operator*(const Vertex& v1, float k) {
	Vertex v;
	v.Position = v1.Position * k;
	v.Normal = v1.Normal * k;
	v.TexCoords = v1.TexCoords * k;
	v.Tangent = v1.Tangent * k;
	v.Bitangent = v1.Bitangent * k;
	return v;
}

struct hashVertex {
	std::size_t operator () (const Vertex& v) const {
		return std::hash<float>()(v.Position.x) ^ std::hash<float>()(v.Position.y) ^ std::hash<float>()(v.Position.z);
	}
};
struct equalVertex {
	bool operator () (const Vertex& v1, const Vertex& v2) const {
		return v1.Position == v2.Position;
	}
};
struct hashPair {
	std::size_t operator () (const std::pair<Vertex, Vertex>& v) const {
		return std::hash<float>()(v.first.Position.x) ^ std::hash<float>()(v.first.Position.y) ^ std::hash<float>()(v.first.Position.z)
			^ std::hash<float>()(v.second.Position.x) ^ std::hash<float>()(v.second.Position.y) ^ std::hash<float>()(v.second.Position.z);
	}
};
struct equalPair {
	bool operator () (const std::pair<Vertex, Vertex>& v1, const std::pair<Vertex, Vertex>& v2) const {
		return v1.first.Position == v2.first.Position && v1.second.Position == v2.second.Position;
	}
};

struct HE_Vert {
	Vertex vertex;
	HE_Edge* edge = nullptr;

	Eigen::Matrix4f Q = Eigen::Matrix4f::Zero();
	inline void getQ();
};

struct HE_Edge {
	HE_Vert* vert = nullptr;
	HE_Edge* pair = nullptr;
	HE_Face* face = nullptr;
	HE_Edge* next = nullptr;

	Eigen::Matrix4f	Q;
	Eigen::Vector4f p;
	float cost;
	bool flag = false;
	inline void getQpcost();
	inline void dismiss();
	inline int collapse();
};

struct HE_Face {
	HE_Edge* edge = nullptr;

	bool flag = false;
	Eigen::Matrix4f Kp;
	inline void getKp();
};

inline void HE_Vert::getQ() {
	auto tempEdge = edge;
	do
	{
		Q = Q + tempEdge->face->Kp;
		if (tempEdge->pair == nullptr) break;
		tempEdge = tempEdge->pair->next;
	} while (tempEdge != edge);
}
inline void HE_Edge::getQpcost() {
	Q = vert->Q + next->next->vert->Q;
	Eigen::Matrix4f tMatrix;
	tMatrix << Q(0, 0), Q(0, 1), Q(0, 2), Q(0, 3),
		Q(0, 1), Q(1, 1), Q(1, 2), Q(1, 3),
		Q(0, 2), Q(1, 2), Q(2, 2), Q(2, 3),
		0, 0, 0, 1;
	if (tMatrix.fullPivLu().isInvertible()) {
		Eigen::Vector4f tVector(0, 0, 0, 1);
		p = tMatrix.inverse() * tVector;
		cost = p.transpose() * Q * p;
	}
	else {
		Eigen::Vector4f tVector1(vert->vertex.Position.x, vert->vertex.Position.y, vert->vertex.Position.z, 1);
		Eigen::Vector4f tVector2(next->next->vert->vertex.Position.x, next->next->vert->vertex.Position.y, next->next->vert->vertex.Position.z, 1);
		Eigen::Vector4f tVector3 = (tVector1 + tVector2) / 2;
		float cost1 = tVector1.transpose() * Q * tVector1;
		float cost2 = tVector2.transpose() * Q * tVector2;
		float cost3 = tVector3.transpose() * Q * tVector3;
		if (cost1 <= cost2 && cost1 <= cost3) { p = tVector1; cost = cost1; }
		if (cost2 <= cost1 && cost2 <= cost3) { p = tVector2; cost = cost2; }
		else { p = tVector3; cost = cost3; }
	}
}
inline void HE_Edge::dismiss() {
	flag = true; if (pair != nullptr) pair->flag = true;
}
inline int HE_Edge::collapse() {
	int n = 0;
	dismiss();
	if (pair == nullptr) { return n; }
	vert->Q = Q; pair->vert->Q = Q;
	vert->vertex.Position.x = p[0];
	vert->vertex.Position.y = p[1];
	vert->vertex.Position.z = p[2];
	pair->vert->vertex.Position.x = p[0];
	pair->vert->vertex.Position.y = p[1];
	pair->vert->vertex.Position.z = p[2];
	auto tempEdge = this;
	do
	{
		tempEdge = tempEdge->next;
		tempEdge->getQpcost();
		if (tempEdge->pair == nullptr) break;
		tempEdge = tempEdge->pair;
		tempEdge->getQpcost();
	} while (tempEdge != this);
	tempEdge = pair;
	do
	{
		tempEdge = tempEdge->next;
		tempEdge->getQpcost();
		if (tempEdge->pair == nullptr) break;
		tempEdge = tempEdge->pair;
		tempEdge->vert = vert;
		tempEdge->getQpcost();
	} while (tempEdge->next != this);
	if (!face->flag) { ++n; face->flag = true; }
	if (!pair->face->flag) { ++n; pair->face->flag = true; }
	/*pair->next->dismiss();
	next->next->dismiss();*/
	return n;
}
bool cmp(const HE_Edge* a, const HE_Edge* b) {
	if (a->flag) return false;
	if (b->flag) return true;
	return a->cost < b->cost;
}
inline void HE_Face::getKp() {
	auto first = edge->vert->vertex.Position;
	auto second = edge->next->vert->vertex.Position;
	auto third = edge->next->next->vert->vertex.Position;
	auto fs = second - first;
	auto st = third - second;
	Eigen::Vector3f A(fs.x, fs.y, fs.z);
	Eigen::Vector3f B(st.x, st.y, st.z);
	auto temp = A.cross(B).normalized();
	float a = temp[0], b = temp[1], c = temp[3];
	float d = .0f - (a * second.x + b * second.y + c * second.z);
	Eigen::Vector4f p(a, b, c, d);
	Kp = p * p.transpose();
}

class HalfEdge {
public:
	std::vector<HE_Vert*> verts;
	std::vector<HE_Edge*> edges;
	std::vector<HE_Face*> faces;
	void clear();
};

inline void HalfEdge::clear() {
	verts.clear(); edges.clear(); faces.clear();
}