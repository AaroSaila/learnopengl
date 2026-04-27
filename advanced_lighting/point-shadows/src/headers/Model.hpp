#pragma once

#include <string>
#include <vector>

#include <assimp/scene.h>

#include "Mesh.hpp"

class Model {
public:
    std::vector<Mesh> meshes;

    Model(const char* path);

    void draw(Shader& shader) const;

private:
    static std::vector<Texture> textures_loaded;

    std::string directory;

    void load_model(std::string path);
    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> load_material_textures(
        aiMaterial* material,
        aiTextureType type,
        std::string type_name);
    unsigned int texture_from_file(const std::filesystem::path& path);
};
