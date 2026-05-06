#include <string>
#include <string_view>

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

void Mesh::draw(Shader& shader) const {
    (void) shader;
    for (std::size_t i { 0 }; i < this->textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        const std::string_view name { this->textures[i].type };
        if (name == "texture_diffuse") {
            shader.set_int("diffuse_map", i);
        } else if (name == "texture_normal") {
            shader.set_int("normal_map", i);
        } else {
            continue;
        }

        glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
    }

    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
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
        this->vertices.size() * sizeof(Vertex),
        &this->vertices[0],
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        this->indices.size() * sizeof(unsigned int),
        &this->indices[0],
        GL_STATIC_DRAW);

    constexpr std::size_t stride { sizeof(Vertex) };
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*) offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*) offsetof(Vertex, tex_coords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*) offsetof(Vertex, tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*) offsetof(Vertex, bitangent));

    glBindVertexArray(0);
}
