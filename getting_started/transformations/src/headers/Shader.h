#ifndef SHADER_H_
#define SHADER_H_

#include <string>

#include <glm/glm.hpp>

class Shader {
    public:
        Shader(const char* vertex_path, const char* fragment_path);

        unsigned int id() const;

        void use() const;
        void setBool(const std::string &name, const bool value) const;
        void setInt(const std::string &name, const int value) const;
        void setFloat(const std::string &name, const float value) const;
        void setMat4(const std::string& name, const glm::mat4& value);

    private:
        unsigned int _id;
};

#endif // SHADER_H_
