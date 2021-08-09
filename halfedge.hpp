#pragma once

#include <vector>
#include <glm/glm.hpp>

struct HE_edge;
struct HE_vert;
struct HE_face;

struct Vertex {
	// position
	glm::vec3 Position = { 0.0, 0.0, 0.0 };
	// normal
	glm::vec3 Normal = { 0.0, 0.0, 0.0 };
	// texCoords
	glm::vec2 TexCoords = { 0.0, 0.0 };
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

struct HE_vert
{
	Vertex vertex;
	HE_edge* edge;
	Eigen::Matrix4f Q = Eigen::Matrix4f::Zero();
};

struct HE_edge
{
	HE_vert* vert0;
	HE_vert* vert;
	HE_edge* pair;
	HE_face* face;
	HE_edge* next;
	Eigen::Matrix4f	Q;
	Eigen::Vector4f p;
	float cost;
	bool flag = false;
};
bool cmp(const HE_edge* a, const HE_edge* b) {
	if (a->flag) return false;
	if (b->flag) return true;
	return a->cost < b->cost;
}

struct HE_face
{
	HE_edge* edge;
	bool flag = false;
	Eigen::Matrix4f Kp;
};

class HalfEdge
{
public:
	std::vector<HE_edge*> edges;
	std::vector<HE_vert*> verts;
	std::vector<HE_face*> faces;
};

void getQ(HE_vert* vert) {
	auto edge = vert->edge;
	do
	{
		vert->Q += edge->face->Kp;
		edge = edge->pair->next;
	} while (edge != vert->edge);
}

void getQpcost(HE_edge* edge) {
	edge->Q = edge->vert->Q + edge->vert0->Q;
	Eigen::Matrix4f tMatrix;
	tMatrix << edge->Q(0, 0), edge->Q(0, 1), edge->Q(0, 2), edge->Q(0, 3),
		edge->Q(0, 1), edge->Q(1, 1), edge->Q(1, 2), edge->Q(1, 3),
		edge->Q(0, 2), edge->Q(1, 2), edge->Q(2, 2), edge->Q(2, 3),
		0, 0, 0, 1;
	if (tMatrix.fullPivLu().isInvertible()) {
		Eigen::Vector4f tVector(0, 0, 0, 1);
		edge->p = tMatrix.inverse() * tVector;
		edge->cost = edge->p.transpose() * edge->Q * edge->p;
	}
	else {
		Eigen::Vector4f tVector1(edge->vert->vertex.Position.x, edge->vert->vertex.Position.y, edge->vert->vertex.Position.z, 1);
		Eigen::Vector4f tVector2(edge->vert0->vertex.Position.x, edge->vert0->vertex.Position.y, edge->vert0->vertex.Position.z, 1);
		Eigen::Vector4f tVector3 = (tVector1 + tVector2) / 2;
		float cost1 = tVector1.transpose() * edge->Q * tVector1;
		float cost2 = tVector2.transpose() * edge->Q * tVector2;
		float cost3 = tVector3.transpose() * edge->Q * tVector3;
		if (cost1 <= cost2 && cost1 <= cost3) { edge->p = tVector1; edge->cost = cost1; }
		if (cost2 <= cost1 && cost2 <= cost3) { edge->p = tVector2; edge->cost = cost2; }
		else { edge->p = tVector3; edge->cost = cost3; }
	}
}
void dismiss(HE_edge* edge) {
	edge->flag = true;
	edge->pair->flag = true;
}
void collapse(HE_edge* edge) {
	edge->vert->Q = edge->Q;
	edge->vert->vertex.Position.x = edge->p[0];
	edge->vert->vertex.Position.y = edge->p[1];
	edge->vert->vertex.Position.z = edge->p[2];
	auto tempEdge = edge->pair->next;
	do
	{
		tempEdge->vert0 = edge->vert;
		getQpcost(tempEdge);
		tempEdge = tempEdge->pair;
		tempEdge->vert = edge->vert;
		getQpcost(tempEdge);
		tempEdge = tempEdge->next;
	} while (tempEdge != edge);
	edge->face->flag = true;
	edge->pair->flag = true;
	dismiss(edge);
	dismiss(edge->pair->next);
	dismiss(edge->next->next);
}

void getKp(HE_face* face) {
	auto first = face->edge->vert->vertex.Position;
	auto second = face->edge->next->vert->vertex.Position;
	auto third = face->edge->vert0->vertex.Position;
	auto fs = second - first;
	auto st = third - second;
	Eigen::Vector3f A(fs.x, fs.y, fs.z);
	Eigen::Vector3f B(st.x, st.y, st.z);
	auto temp = A.cross(B).normalized();
	float a = temp[0], b = temp[1], c = temp[3];
	float d = .0f - (a * second.x + b * second.y + c * second.z);
	Eigen::Vector4f p(a, b, c, d);
	face->Kp = p * p.transpose();
}