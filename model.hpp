#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <unordered_map>

#include "halfedge.hpp"
#include "shader.hpp"

class Model {
public:
	HalfEdge halfEdge;
	unsigned int VAO;
	Model();
	Model(const std::string& path);
	int vSize() const;
	int eSize() const;
	int fSize() const;
	void Draw();

private:
	std::vector<Vertex>		  vertices;
	std::vector<unsigned int> indices;
	unsigned int VBO, EBO;
	void setupMesh();
	void toHalfEdge();
	void toGLData();
};

inline Model::Model() {}

inline Model::Model(const std::string& path) {
	std::ifstream in(path);
	if (in.fail()) return;
	std::string line;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::vec3> normals;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			glm::vec3 p;
			for (int i = 0; i < 3; ++i) iss >> p[i];
			Vertex v; v.Position = p;
			vertices.push_back(v);
		} 
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			glm::vec2 uv;
			for (int i = 0; i < 2; ++i) iss >> uv[i];
			texcoords.push_back(uv);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			glm::vec3 n;
			for (int i = 0; i < 3; ++i) iss >> n[i];
			normals.push_back(n);
		}
		else if (!line.compare(0, 2, "f ")) {
			int f, t, n;
			iss >> trash;
			int count = 0;
			if (texcoords.size() != 0 && normals.size() != 0)
				while (iss >> f >> trash >> t >> trash >> n) {
					indices.push_back(--f);
					vertices[f].TexCoords = texcoords[--t];
					vertices[f].Normal = normals[--n];
					++count;
				}
			else if (texcoords.size() == 0 && normals.size() != 0) 
				while (iss >> f >> trash >>  trash >> n) { indices.push_back(--f); vertices[f].Normal = normals[--n]; ++count; }
			else if (texcoords.size() != 0 && normals.size() == 0)
				while (iss >> f >> trash >> t) { indices.push_back(--f); vertices[f].TexCoords = texcoords[--t]; ++count; }
			if (4 == count) {
				indices.push_back(indices[indices.size() - 4]);
				indices.push_back(indices[indices.size() - 3]);
				continue;
			}
			if (3 != count) {
				std::cerr << "Error: the obj file is supposed to be triangulated " << count << std::endl;
				in.close();
				exit(-1);
			}
		}
	}
	in.close();
	std::clog << vertices.size() << " " << indices.size() / 3 <<  std::endl;
	toHalfEdge(); toGLData(); setupMesh();
}

inline int Model::vSize() const { return halfEdge.verts.size(); }
inline int Model::eSize() const { return halfEdge.edges.size(); }
inline int Model::fSize() const { return halfEdge.faces.size(); }

inline void Model::setupMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glBindVertexArray(0);
}

inline void Model::Draw() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

inline void Model::toHalfEdge() {
	halfEdge.clear();
	std::unordered_map<Vertex, HE_Vert*, hashVertex, equalVertex> umv;
	std::unordered_map<std::pair<Vertex, Vertex>, HE_Edge*, hashPair, equalPair> ume;
	for (unsigned int i = 0; i < indices.size(); i += 3) {
		unsigned int first = indices[i], second = indices[i + 1], third = indices[i + 2];
		HE_Vert* vert1 = nullptr, * vert2 = nullptr, * vert3 = nullptr;
		HE_Edge* edge1 = new HE_Edge(), * edge2 = new HE_Edge(), * edge3 = new HE_Edge();
		HE_Face* face = new HE_Face();
		if (umv.find(vertices[first]) != umv.end()) vert1 = umv[vertices[first]];
		else { vert1 = new HE_Vert(); vert1->vertex = vertices[first]; vert1->edge = edge1; halfEdge.verts.push_back(vert1); umv.insert({ vert1->vertex, vert1 }); }
		if (umv.find(vertices[second]) != umv.end()) vert2 = umv[vertices[second]];
		else { vert2 = new HE_Vert(); vert2->vertex = vertices[second]; vert2->edge = edge2; halfEdge.verts.push_back(vert2); umv.insert({ vert2->vertex, vert2 }); }
		if (umv.find(vertices[third]) != umv.end()) vert3 = umv[vertices[third]];
		else { vert3 = new HE_Vert(); vert3->vertex = vertices[third]; vert3->edge = edge3; halfEdge.verts.push_back(vert3); umv.insert({ vert3->vertex, vert3 }); }
		edge1->vert = vert2; edge1->face = face; edge1->next = edge2;
		edge2->vert = vert3; edge2->face = face; edge2->next = edge3;
		edge3->vert = vert1; edge3->face = face; edge3->next = edge1;
		face->edge = edge1;
		if (ume.find({ vert2->vertex, vert1->vertex }) != ume.end()) { edge1->pair = ume[{vert2->vertex, vert1->vertex}]; ume[{vert2->vertex, vert1->vertex}]->pair = edge1; }
		else ume.insert({ {vert1->vertex, vert2->vertex}, edge1 });
		if (ume.find({ vert3->vertex, vert2->vertex }) != ume.end()) { edge1->pair = ume[{vert3->vertex, vert2->vertex}]; ume[{vert3->vertex, vert2->vertex}]->pair = edge2; }
		else ume.insert({ {vert2->vertex, vert3->vertex}, edge2 });
		if (ume.find({ vert1->vertex, vert3->vertex }) != ume.end()) { edge1->pair = ume[{vert1->vertex, vert3->vertex}]; ume[{vert1->vertex, vert3->vertex}]->pair = edge3; }
		else ume.insert({ {vert3->vertex, vert1->vertex}, edge3 });
		halfEdge.edges.push_back(edge1); halfEdge.edges.push_back(edge2); halfEdge.edges.push_back(edge3);
		halfEdge.faces.push_back(face);
	}
}

inline void Model::toGLData() {
	vertices.clear();
	indices.clear();
	for (auto& face : halfEdge.faces) {
		auto edge = face->edge;
		do
		{
			vertices.push_back(edge->vert->vertex);
			indices.push_back(vertices.size() - 1);
			edge = edge->next;
		} while (edge != face->edge);
	}
}