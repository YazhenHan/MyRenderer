#pragma once
#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"

#include <string>
#include <vector>

#include "halfedge.hpp"
#include <unordered_map>
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

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        //setupMesh();
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

    HalfEdge halfEdge;
    int vSize() const { return halfEdge.verts.size(); }
    int eSize() const { return halfEdge.edges.size(); }
    int fSize() const { return halfEdge.faces.size(); }
    float xmin, xmax, ymin, ymax, zmin, zmax;
    void toHalfEdge() {
        halfEdge.clear();
        std::unordered_map<Vertex, HE_Vert*, hashVertex, equalVertex> umv;
        std::unordered_map<std::pair<Vertex, Vertex>, HE_Edge*, hashPair, equalPair> ume;
        xmin = vertices.front().Position.x, xmax = vertices.front().Position.x,
        ymin = vertices.front().Position.y, ymax = vertices.front().Position.y,
        zmin = vertices.front().Position.z, zmax = vertices.front().Position.z;
        for (unsigned int i = 0; i < indices.size(); i += 3) {
            unsigned int first = indices[i], second = indices[i + 1], third = indices[i + 2];
            xmin = std::min({ xmin, vertices[first].Position.x, vertices[second].Position.x, vertices[third].Position.x });
            xmax = std::max({ xmax, vertices[first].Position.x, vertices[second].Position.x, vertices[third].Position.x });
            ymin = std::min({ ymin, vertices[first].Position.y, vertices[second].Position.y, vertices[third].Position.y });
            ymax = std::max({ ymax, vertices[first].Position.y, vertices[second].Position.y, vertices[third].Position.y });
            zmin = std::min({ zmin, vertices[first].Position.z, vertices[second].Position.z, vertices[third].Position.z });
            zmax = std::max({ zmax, vertices[first].Position.z, vertices[second].Position.z, vertices[third].Position.z });
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
            if (ume.find({ vert3->vertex, vert2->vertex }) != ume.end()) { edge2->pair = ume[{vert3->vertex, vert2->vertex}]; ume[{vert3->vertex, vert2->vertex}]->pair = edge2; }
            else ume.insert({ {vert2->vertex, vert3->vertex}, edge2 });
            if (ume.find({ vert1->vertex, vert3->vertex }) != ume.end()) { edge3->pair = ume[{vert1->vertex, vert3->vertex}]; ume[{vert1->vertex, vert3->vertex}]->pair = edge3; }
            else ume.insert({ {vert3->vertex, vert1->vertex}, edge3 });
            halfEdge.edges.push_back(edge1); halfEdge.edges.push_back(edge2); halfEdge.edges.push_back(edge3);
            halfEdge.faces.push_back(face);
        }
    }
    void toGLData() {
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
        setupMesh();
    }
    void loopSub() {
        std::vector<Vertex> vns;
        for (auto& face : halfEdge.faces) {
            auto edge = face->edge;
            do
            {
                Vertex va = edge->next->next->vert->vertex, vb = edge->vert->vertex;
                Vertex vc = edge->next->vert->vertex, vd, v;
                if (edge->pair != nullptr) { vd = edge->pair->next->vert->vertex; v = (va + vb) * (3.0 / 8.0) + (vc + vd) * (1.0 / 8.0); }
                else { v = (va + vb) * (4.0 / 8.0); }
                vns.push_back(v);
                edge = edge->next;
            } while (edge != face->edge);
        }
        std::vector<Vertex> vos;
        for (auto& vert : halfEdge.verts) {
            Vertex vo = vert->vertex;
            auto edge = vert->edge;
            Vertex vn;
            int n = 0;
            do
            {
                n++;
                vn = vn + edge->vert->vertex;
                if (edge->pair == nullptr) break;
                edge = edge->pair->next;
            } while (edge != vert->edge);
            float u = n == 3 ? 3.0 / 16.0 : 3.0 / (8.0 * n);
            Vertex v = vo * (1.0 - n * u) + vn * u;
            float a = 5.0 / 8.0 - (3.0 / 8.0 + 1.0 / 4.0 * cos(2 * 3.1415926 / n)) * (3.0 / 8.0 + 1.0 / 4.0 * cos(2 * 3.1415926 / n));
            v = vo * (1 - a) + vn * (a / n);
            vos.push_back(v);
        }
        for (int i = 0; i < vos.size(); ++i)
            halfEdge.verts[i]->vertex = vos[i];
        vertices.clear();
        indices.clear();
        unsigned int i = 0;
        for (auto& face : halfEdge.faces) {
            vertices.push_back(vns[i]); indices.push_back(vertices.size() - 1);
            vertices.push_back(vns[i + 1]); indices.push_back(vertices.size() - 1);
            vertices.push_back(vns[i + 2]); indices.push_back(vertices.size() - 1);
            auto edge = face->edge;
            vertices.push_back(vns[i]); indices.push_back(vertices.size() - 1);
            vertices.push_back(edge->vert->vertex); indices.push_back(vertices.size() - 1);
            vertices.push_back(vns[i + 1]); indices.push_back(vertices.size() - 1);
            edge = edge->next;
            vertices.push_back(vns[i + 1]); indices.push_back(vertices.size() - 1);
            vertices.push_back(edge->vert->vertex); indices.push_back(vertices.size() - 1);
            vertices.push_back(vns[i + 2]); indices.push_back(vertices.size() - 1);
            edge = edge->next;
            vertices.push_back(vns[i + 2]); indices.push_back(vertices.size() - 1);
            vertices.push_back(edge->vert->vertex); indices.push_back(vertices.size() - 1);
            vertices.push_back(vns[i]); indices.push_back(vertices.size() - 1);
            i = i + 3;
        }
    }
    void qemSim(int faceNum) {
        for (auto& face : halfEdge.faces) face->getKp();
        for (auto& vert : halfEdge.verts) vert->getQ();
        for (auto& edge : halfEdge.edges) edge->getQpcost();
        int n = halfEdge.faces.size();
        while (n > faceNum)
        {
            sort(halfEdge.edges.begin(), halfEdge.edges.end(), cmp);
            n -= halfEdge.edges.front()->collapse();
        }
        vertices.clear();
        indices.clear();
        for (auto& face : halfEdge.faces) {
            if (face->flag) continue;
            auto edge = face->edge;
            vertices.push_back(edge->vert->vertex); indices.push_back(vertices.size() - 1);
            edge = edge->next;
            vertices.push_back(edge->vert->vertex); indices.push_back(vertices.size() - 1);
            edge = edge->next;
            vertices.push_back(edge->vert->vertex); indices.push_back(vertices.size() - 1);
        }
    }
    void miniSur(float stepsize) {
        for (auto& vert : halfEdge.verts) {
            auto edge = vert->edge;

            bool flag = false;
            do
            {
                if (edge->pair == nullptr) { flag = true; break; }
                edge = edge->pair->next;
            } while (edge != vert->edge);
            if (flag) continue;

            float voronoi = 0.0f;
            edge = vert->edge;
            auto vert0 = vert->vertex.Position;
            do
            {
                auto vert1 = edge->vert->vertex.Position;
                auto vert2 = edge->next->vert->vertex.Position; auto vert3 = edge->pair->next->vert->vertex.Position;

                auto vert20 = vert0 - vert2, vert21 = vert1 - vert2;
                auto costheta2 = (vert20.x * vert21.x + vert20.y * vert21.y + vert20.z * vert21.z) / (sqrt(vert20.x * vert20.x +
                    vert20.y * vert20.y + vert20.z * vert20.z) * sqrt(vert21.x * vert21.x + vert21.y * vert21.y + vert21.z * vert21.z));
                auto cottheta2 = costheta2 / sqrt(1 - costheta2 * costheta2);
                cottheta2 = max(cottheta2, 0.0f);

                auto vert30 = vert0 - vert3, vert31 = vert1 - vert3;
                auto costheta3 = (vert30.x * vert31.x + vert30.y * vert31.y + vert30.z * vert31.z) / (sqrt(vert30.x * vert30.x +
                    vert30.y * vert30.y + vert30.z * vert30.z) * sqrt(vert31.x * vert31.x + vert31.y * vert31.y + vert31.z * vert31.z));
                auto cottheta3 = costheta3 / sqrt(1 - costheta3 * costheta3);
                cottheta3 = max(cottheta3, 0.0f);

                auto vert10 = vert0 - vert1;
                auto d01 = vert10.x * vert10.x + vert10.y * vert10.y + vert10.z * vert10.z;

                voronoi = voronoi + (cottheta2 + cottheta3) * d01 / 8.0f;

                edge = edge->pair->next;
            } while (edge != vert->edge);

            glm::vec3 hn(0.0, 0.0, 0.0);
            do
            {
                auto vert1 = edge->vert->vertex.Position;
                auto vert2 = edge->next->vert->vertex.Position; auto vert3 = edge->pair->next->vert->vertex.Position;

                auto vert20 = vert0 - vert2, vert21 = vert1 - vert2;
                auto costheta2 = (vert20.x * vert21.x + vert20.y * vert21.y + vert20.z * vert21.z) / (sqrt(vert20.x * vert20.x +
                    vert20.y * vert20.y + vert20.z * vert20.z) * sqrt(vert21.x * vert21.x + vert21.y * vert21.y + vert21.z * vert21.z));
                auto cottheta2 = costheta2 / sqrt(1 - costheta2 * costheta2);
                cottheta2 = max(cottheta2, 0.0f);

                auto vert30 = vert0 - vert3, vert31 = vert1 - vert3;
                auto costheta3 = (vert30.x * vert31.x + vert30.y * vert31.y + vert30.z * vert31.z) / (sqrt(vert30.x * vert30.x +
                    vert30.y * vert30.y + vert30.z * vert30.z) * sqrt(vert31.x * vert31.x + vert31.y * vert31.y + vert31.z * vert31.z));
                auto cottheta3 = costheta3 / sqrt(1 - costheta3 * costheta3);
                cottheta3 = max(cottheta3, 0.0f);

                auto vert10 = vert0 - vert1;

                hn = hn + (cottheta2 + cottheta3) * vert10 / (4 * voronoi);

                edge = edge->pair->next;
            } while (edge != vert->edge);

            vert->hn = hn;
        }

        for (auto& vert : halfEdge.verts)
            vert->vertex.Position = vert->vertex.Position - vert->hn * stepsize;
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