#include <fstream>
#include <sstream>
#include <string_view>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"

#include "Shader.hpp"
#include "error_handling.hpp"
#include "quit.hpp"

Shader::Shader(const char* vertex_path, const char* fragment_path) {
    std::string vertex_code;
    std::string fragment_code;
    std::ifstream vertex_shader_file;
    std::ifstream fragment_shader_file;
    vertex_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragment_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vertex_shader_file.open(vertex_path);
        std::stringstream vertex_shader_stream;
        vertex_shader_stream << vertex_shader_file.rdbuf();
        vertex_shader_file.close();
        vertex_code = vertex_shader_stream.str();

        fragment_shader_file.open(fragment_path);
        std::stringstream fragment_shader_stream;
        fragment_shader_stream << fragment_shader_file.rdbuf();
        fragment_shader_file.close();
        fragment_code = fragment_shader_stream.str();
    } catch (std::ifstream::failure e) {
        log_error(std::format("Failed to read shaders: {}", e.what()).c_str());
        quit(1);
    }

    const char* vertex_code_c_str { vertex_code.c_str() };
    const char* fragment_code_c_str { fragment_code.c_str() };

    // Vertex shader
    unsigned int vertex_shader { glCreateShader(GL_VERTEX_SHADER) };
    if (vertex_shader == 0) {
        log_error("Failed to create vertex shader.");
        quit(1);
    }

    glShaderSource(vertex_shader, 1, &vertex_code_c_str, nullptr);
    glCompileShader(vertex_shader);
    check_shader_compile_error(vertex_shader);

    // Fragment shader
    unsigned int fragment_shader { glCreateShader(GL_FRAGMENT_SHADER) };
    if (fragment_shader == 0) {
        log_error("Failed to create fragment shader.");
        quit(1);
    }

    glShaderSource(fragment_shader, 1, &fragment_code_c_str, nullptr);
    glCompileShader(fragment_shader);
    check_shader_compile_error(fragment_shader);

    // Shader program
    this->_id = glCreateProgram();
    if (this->_id == 0) {
        log_error("Failed to create program.");
        quit(1);
    }

    glAttachShader(this->_id, vertex_shader);
    glAttachShader(this->_id, fragment_shader);

    glLinkProgram(this->_id);
    check_shader_program_link_error(this->_id);

    // Delete linked shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

unsigned int Shader::id() const {
    return this->_id;
}

void Shader::use() const {
    glUseProgram(this->_id);
}

void Shader::setBool(const std::string_view& name, const bool value) const {
    glUniform1i(glGetUniformLocation(this->_id, name.data()), static_cast<int>(value));
}

void Shader::setInt(const std::string_view& name, const int value) const {
    const int uniform { glGetUniformLocation(this->_id, name.data()) };
    if (uniform == -1) {
        log_error(std::format("Could not find uniform '{}'", name).data());
        quit(1);
    }
    glUniform1i(uniform, value);
}

void Shader::setFloat(const std::string_view& name, const float value) const {
    const int uniform { glGetUniformLocation(this->_id, name.data()) };
    if (uniform == -1) {
        log_error(std::format("Could not find uniform '{}'", name).data());
        quit(1);
    }
    glUniform1f(uniform, value);
}

void Shader::setMat4(const std::string_view& name, const glm::mat4& value) {
    const int uniform { glGetUniformLocation(this->_id, name.data()) };
    if (uniform == -1) {
        log_error(std::format("Could not find uniform '{}'", name).data());
        quit(1);
    }
    glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(value));
}
