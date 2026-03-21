#include <string>

#include "Mesh.hpp"

#include "glad/glad.h"

// Constructor

Mesh::Mesh(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    std::vector<Texture>& textures)

    : vertices { vertices }
    , indices { indices }
    , textures { textures } {

    this->setup_mesh();
}

// public

void Mesh::draw(Shader& shader) {
    unsigned int diffuse_nr { 1 };
    unsigned int specular_nr { 1 };
    for (std::size_t i { 0 }; i < this->indices.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number {};
        const std::string name { this->textures[i].type };
        if (name == "texture_diffuse") {
            number = std::to_string(diffuse_nr);
            diffuse_nr++;
        } else if (name == "texture_specular") {
            number = std::to_string(specular_nr);
            specular_nr++;
        }

        glBindTexture(GL_TEXTURE_2D, textures[i].id);
        shader.set_int(("material" + name + number).c_str(), i);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// private

void Mesh::setup_mesh() {
    glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
    glGenBuffers(1, &this->ebo);

    glBindVertexArray(this->vao);

    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(this->vertices),
        this->vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(this->indices),
        this->indices.data(),
        GL_STATIC_DRAW);

    constexpr std::size_t stride { sizeof(Vertex) };
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)offsetof(Vertex, tex_coords));

    glBindVertexArray(0);
}
