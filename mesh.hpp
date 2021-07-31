#pragma once
#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"

#include <string>
#include <vector>
#include <unordered_set>

#include "halfedge.hpp"

using namespace std;

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;

    HalfEdge halfEdge;

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    void toHalfEdge() {
        for (unsigned int i = 0; i < indices.size(); i += 3) {
            unsigned int first = indices[i], second = indices[i + 1], third = indices[i + 2];
            HE_edge* edge1 = new HE_edge(), * edge2 = new HE_edge(), * edge3 = new HE_edge();
            HE_vert* vert1 = nullptr, * vert2 = nullptr, * vert3 = nullptr;
            HE_face* face = new HE_face();

            bool flag1 = false, flag2 = false, flag3 = false;
            for (auto& vert : halfEdge.verts) {
                if (first == vert->indice) { vert1 = vert; flag1 = true; }
                if (second == vert->indice) { vert2 = vert; flag2 = true; }
                if (third == vert->indice) { vert3 = vert; flag3 = true; }
            }
            if (!flag1) { vert1 = new HE_vert(); vert1->indice = first; vert1->vertex = vertices[first]; vert1->edge = edge1; halfEdge.verts.push_back(vert1); }
            if (!flag2) { vert2 = new HE_vert(); vert2->indice = second; vert2->vertex = vertices[second]; vert2->edge = edge2; halfEdge.verts.push_back(vert2); }
            if (!flag3) { vert3 = new HE_vert(); vert3->indice = third; vert3->vertex = vertices[third]; vert3->edge = edge3; halfEdge.verts.push_back(vert3); }
            
            edge1->vert0 = vert1; edge1->vert = vert2; edge1->pair = nullptr; edge1->face = face; edge1->next = edge2;
            edge2->vert0 = vert2; edge2->vert = vert3; edge2->pair = nullptr; edge2->face = face; edge2->next = edge3;
            edge3->vert0 = vert3; edge3->vert = vert1; edge3->pair = nullptr; edge3->face = face; edge3->next = edge1;
            face->edge = edge1;
            for (auto& edge : halfEdge.edges) {
                if (edge1->vert0->indice == edge->vert->indice && edge1->vert->indice == edge->vert0->indice) {
                    edge1->pair = edge; edge->pair = edge1;
                }
                if (edge2->vert0->indice == edge->vert->indice && edge2->vert->indice == edge->vert0->indice) {
                    edge2->pair = edge; edge->pair = edge2;
                }
                if (edge3->vert0->indice == edge->vert->indice && edge3->vert->indice == edge->vert0->indice) {
                    edge3->pair = edge; edge->pair = edge3;
                }
            }
            halfEdge.edges.push_back(edge1); halfEdge.edges.push_back(edge2); halfEdge.edges.push_back(edge3);
            halfEdge.faces.push_back(face);
        }
    }

    void toGLMesh() {
        vertices.clear();
        indices.clear();
        /*for (auto& face : halfEdge.faces) {
            auto edge = face->edge;
            do
            {
                vertices.push_back(edge->vert->vertex);
                indices.push_back(vertices.size());
                edge = edge->next;
            } while (edge != face->edge);
        }*/
        for (unsigned int i = 0; i < halfEdge.faces.size() / 2; ++i) {
            auto face = halfEdge.faces[i];
            auto edge = face->edge;
            do
            {
                vertices.push_back(edge->vert->vertex);
                indices.push_back(vertices.size());
                edge = edge->next;
            } while (edge != face->edge);
        }
    }

    void loopSub() {
        std::vector<Vertex> vns;
        for (auto& face : halfEdge.faces) {
            auto edge = face->edge;
            do
            {
                Vertex va = edge->vert0->vertex, vb = edge->vert->vertex;
                Vertex vc = edge->next->vert->vertex, vd;
                if (edge->pair != nullptr) vd = edge->pair->next->vert->vertex;
                Vertex v = (va + vb) * (3.0 / 8.0) + (vc + vd) * (1.0 / 8.0);
                vns.push_back(v);
                edge = edge->next;
            } while (edge != face->edge);
        }
        std::vector<Vertex> vos;
        for (auto* vert : halfEdge.verts) {
            Vertex vo = vert->vertex;
            auto edge = vert->edge;
            Vertex vn;
            int n = 0;
            do
            {
                vn = vn + edge->vert->vertex;
                edge = edge->pair->next;
            } while (edge != vert->edge);
            float u = n == 3 ? 3.0 / 16.0 : 3.0 / (8.0 * n);
            Vertex v = vo * (1.0 - n * u) + vn * u;
            vos.push_back(v);
        }
        for (auto& face : halfEdge.faces) {
            auto edge = face->edge;
            do
            {

            } while (true);
        }
    }
    // render the mesh
    void Draw(Shader& shader)
    {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to stream
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to stream
            else if (name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to stream

            // now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};
#endif