#pragma once

struct HE_Vert;
struct HE_Edge;
struct HE_Face;

struct Vertex {
	glm::vec3 Position = { 0.0,0.0,0.0 };
	glm::vec2 TexCoords = { 0.0,0.0 };
	glm::vec3 Normal = { 0.0,0.0,0.0 };
};

struct HE_Vert {
	Vertex vertex;
	HE_Edge* edge;
};

struct HE_Edge {
	HE_Vert* vert;
	HE_Edge* pair;
	HE_Face* face;
	HE_Edge* next;
};

struct HE_Face {
	HE_Edge* edge;
};

class HalfEdge {
public:
	std::vector<HE_Vert*> verts;
	std::vector<HE_Edge*> edges;
	std::vector<HE_Face*> faces;
};