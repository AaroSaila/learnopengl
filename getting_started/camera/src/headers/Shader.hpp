#pragma once

#include <string_view>

#include <glm/glm.hpp>

class Shader {
    public:
        Shader(const char* vertex_path, const char* fragment_path);

        unsigned int id() const;

        void use() const;
        void setBool(const std::string_view &name, const bool value) const;
        void setInt(const std::string_view &name, const int value) const;
        void setFloat(const std::string_view &name, const float value) const;
        void setMat4(const std::string_view& name, const glm::mat4& value);

    private:
        unsigned int _id;
};

