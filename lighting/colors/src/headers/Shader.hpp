#pragma once

#include <string_view>

#include <glm/glm.hpp>

class Shader {
    public:
        Shader(const char* vertex_path, const char* fragment_path);

        unsigned int id() const;

        void use() const;
        void set_bool(const std::string_view &name, const bool value) const;
        void set_int(const std::string_view &name, const int value) const;
        void set_float(const std::string_view &name, const float value) const;
        void set_vec3(const std::string_view &name, const glm::vec3& value) const;
        void set_mat4(const std::string_view& name, const glm::mat4& value) const;

    private:
        unsigned int _id;
};

