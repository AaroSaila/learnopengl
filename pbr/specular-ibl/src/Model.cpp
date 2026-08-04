#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "glad/glad.h"
#include "stb_image.h"

#include "Model.hpp"
#include "error_handling.hpp"
#include "quit.hpp"

std::vector<Texture> Model::textures_loaded { };

Model::Model(const std::filesystem::path& path) {
    this->load_model(path);
}

// public

void Model::draw(Shader& shader) const {
    for (const auto& mesh : this->meshes) {
        mesh.draw(shader);
    }
}

// private

void Model::load_model(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        log_error(
            "Failed to load model. Given path does not exist. Path: {}",
            path.c_str()
        );
        quit(1);
    }

    Assimp::Importer importer { };
    const aiScene* scene { importer.ReadFile(
        path.c_str(),
        aiProcess_Triangulate
            | aiProcess_FlipUVs
            | aiProcess_CalcTangentSpace
    ) };

    if (
        scene == nullptr
        || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
        || scene->mRootNode == nullptr
    ) {

        log_error("Failed to load model. Assimp error: {}", importer.GetErrorString());
        return;
    }

    this->directory = std::filesystem::path { path }.remove_filename();

    this->process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode* node, const aiScene* scene) {
    for (std::size_t i { 0 }; i < node->mNumMeshes; i++) {
        aiMesh* mesh { scene->mMeshes[node->mMeshes[i]] };
        this->meshes.push_back(process_mesh(mesh, scene));
    }

    for (std::size_t i { 0 }; i < node->mNumChildren; i++) {
        this->process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices { };
    vertices.reserve(mesh->mNumVertices);
    std::vector<unsigned int> indices { };
    std::vector<Texture> textures { };

    for (std::size_t i { 0 }; i < mesh->mNumVertices; i++) {
        Vertex vertex { };

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0] != nullptr) {
            vertex.tex_coords.x = mesh->mTextureCoords[0][i].x;
            vertex.tex_coords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.tex_coords.x = 0.0f;
            vertex.tex_coords.y = 0.0f;
        }

        if (mesh->mTangents != nullptr) {
            vertex.tangent = glm::vec3 {
                mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z
            };
        }

        if (mesh->mBitangents != nullptr) {
            vertex.bitangent = glm::vec3 {
                mesh->mBitangents[i].x,
                mesh->mBitangents[i].y,
                mesh->mBitangents[i].z
            };
        }

        vertices.emplace_back(vertex);
    }

    // indices
    for (std::size_t i { 0 }; i < mesh->mNumFaces; i++) {
        const aiFace& face { mesh->mFaces[i] };
        for (std::size_t j { 0 }; j < face.mNumIndices; j++) {
            indices.emplace_back(face.mIndices[j]);
        }
    }

    // material
    aiMaterial* material { scene->mMaterials[mesh->mMaterialIndex] };

    const std::vector<Texture> diffuse_maps { this->load_material_textures(
        material,
        aiTextureType_DIFFUSE,
        "texture_diffuse"
    ) };
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    const std::vector<Texture> specular_maps { this->load_material_textures(
        material,
        aiTextureType_SPECULAR,
        "texture_specular"
    ) };
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

    const std::vector<Texture> normal_maps { this->load_material_textures(
        material,
        aiTextureType_HEIGHT,
        "texture_normal"
    ) };
    textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

    return Mesh { vertices, indices, textures };
}

std::vector<Texture> Model::load_material_textures(
    aiMaterial* material,
    aiTextureType type,
    std::string type_name
) {

    std::vector<Texture> textures { };

    for (std::size_t i { 0 }; i < material->GetTextureCount(type); i++) {
        aiString path { };
        material->GetTexture(type, i, &path);

        const auto existing_texture { std::find_if(
            Model::textures_loaded.begin(),
            Model::textures_loaded.end(),
            [path](Texture& tex) -> bool {
                return tex.path.compare(path.C_Str()) == 0;
            }
        ) };

        if (existing_texture == Model::textures_loaded.end()) {
            Texture texture { };
            texture.id = this->texture_from_file(std::filesystem::path { this->directory } / path.C_Str());
            texture.type = type_name;
            texture.path = std::string { path.C_Str() };
            textures.emplace_back(texture);
            Model::textures_loaded.emplace_back(texture);
        } else {
            textures.emplace_back(*existing_texture);
        }
    }

    return textures;
}

unsigned int Model::texture_from_file(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        log_error("The given image file '{}' does not exist.", path.c_str());
        quit(1);
    }

    int img_w { };
    int img_h { };
    int img_nr_channels { };
    unsigned char* img_data {
        stbi_load(path.c_str(), &img_w, &img_h, &img_nr_channels, 0)
    };
    if (img_data == nullptr) {
        log_error("Failed to load image.");
        quit(1);
    }

    GLenum format { };
    switch (img_nr_channels) {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        log_error("Unhandled amount of channels: {}", img_nr_channels);
    }

    unsigned int texture { };
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_w, img_h, 0, format,
        GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(img_data);
    img_data = nullptr;

    return texture;
}
