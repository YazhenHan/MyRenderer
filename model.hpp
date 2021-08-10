#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

#include "halfedge.hpp"
#include "shader.hpp"

class Model {
public:
	HalfEdge halfEdge;
	unsigned int VAO;
	Model();
	Model(const std::string& path);
	int vSize() const;
	int fSize() const;
	void Draw();

private:
	std::vector<Vertex>		  vertices;
	std::vector<unsigned int> indices;
	unsigned int VBO, EBO;
	void setupMesh();
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
			while (iss >> f >> trash >> t >> trash >> n) {
				indices.push_back(--f);
				vertices[f].TexCoords = texcoords[--t];
				vertices[f].Normal = normals[--n];
				++count;
			}
			if (3 != count) {
				std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
				in.close();
				return;
			}
		}
	}
	in.close();
	setupMesh();
}

inline int Model::vSize() const { return vertices.size(); }
inline int Model::fSize() const { return indices.size() / 3; }

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