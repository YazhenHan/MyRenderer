#pragma once

struct HE_Vert;
struct HE_Edge;
struct HE_Face;

struct Vertex {
	glm::vec3 Position = { 0.0,0.0,0.0 };
	glm::vec2 TexCoords = { 0.0,0.0 };
	glm::vec3 Normal = { 0.0,0.0,0.0 };
};

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
};

struct HE_Edge {
	HE_Vert* vert = nullptr;
	HE_Edge* pair = nullptr;
	HE_Face* face = nullptr;
	HE_Edge* next = nullptr;
};

struct HE_Face {
	HE_Edge* edge = nullptr;
};

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