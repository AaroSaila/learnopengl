#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Shader.hpp"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices,
        std::vector<Texture>& textures);

    void draw(Shader& shader) const;
private:
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    void setup_mesh();
};
