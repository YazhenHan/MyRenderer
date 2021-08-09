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
};

struct HE_vert
{
	Vertex vertex;
	HE_edge* edge;
};

struct HE_face
{
	HE_edge* edge;
};

class HalfEdge
{
public:
	std::vector<HE_edge*> edges;
	std::vector<HE_vert*> verts;
	std::vector<HE_face*> faces;
};
