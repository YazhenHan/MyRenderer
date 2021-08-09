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
};

struct HE_vert
{
	Vertex vertex;
	HE_edge* edge;
	Eigen::Matrix4f Q = Eigen::Matrix4f::Zero();
};

struct HE_face
{
	HE_edge* edge;
	Eigen::Matrix4f Kp;
};

class HalfEdge
{
public:
	std::vector<HE_edge*> edges;
	std::vector<HE_vert*> verts;
	std::vector<HE_face*> faces;
};

Eigen::Matrix4f getKp(HE_face* face) {
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
	return p * p.transpose();
}