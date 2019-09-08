#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../gl/glad.hpp"
#include "../gl/object_buffers.hpp"
#include "file.hpp"


namespace eldstar {
namespace loaders {

class shader {
    public:
        shader(GLenum shader_type, const file& from) : id(glCreateShader(shader_type)), type(shader_type) {
            // Read data from file
            std::stringstream buffer;
            buffer << from.rdbuf();
            std::string data(buffer.str());

            const char *code_ptr = data.c_str();

            // Compile the shader
            glShaderSource(id, 1, &code_ptr, NULL);
            glCompileShader(id);

            // Check for errors
            GLint code;
            GLchar exception_log[1024];

            glGetShaderiv(id, GL_COMPILE_STATUS, &code);

            if (!code) {
                glGetShaderInfoLog(id, 1024, NULL, exception_log);
                std::string shader_typename;

                switch (shader_type) {
                    case GL_VERTEX_SHADER:
                        shader_typename = "vertex";
                        break;
                    case GL_FRAGMENT_SHADER:
                        shader_typename = "fragment";
                        break;
                    case GL_GEOMETRY_SHADER:
                        shader_typename = "geometry";
                        break;
                    default:
                        shader_typename = "unknown";
                        break;
                }

                std::string exc_info(exception_log);

                glDeleteShader(id);
                throw load_failure(
                    "Failed to load OpenGL " + shader_typename + " shader from '" + from.filename + "' "
                    + "(code " + std::to_string(code) + "):\n" + exc_info
                );
            }
        };

        ~shader() {
            glDeleteShader(id);
        }

        GLenum type;
        unsigned int id;
};

class shader_program {
    public:
        shader_program(const std::vector<shader>& shaders) : id(glCreateProgram()) {
            // Attach shaders
            for (const shader& s : shaders) {
                glAttachShader(id, s.id);
            }

            // Link program
            glLinkProgram(id);

            // Check for errors
            GLint code;
            GLchar exception_log[1024];

            glGetProgramiv(id, GL_LINK_STATUS, &code);

            if (!code) {
                glGetProgramInfoLog(id, 1024, NULL, exception_log);
                std::string exc_info(exception_log);

                glDeleteProgram(id);
                throw load_failure(
                    "Failed to link OpenGL shader program (code " + std::to_string(code) + "):\n" + exc_info
                );
            }
        };

        ~shader_program() {
            glDeleteProgram(id);
        }

        void use() {
            glUseProgram(id);
        }

        GLint get_location(const char *name) const {
            return glGetUniformLocation(id, name);
        }

        GLint get_location(const std::string &name) const {
            return get_location(name.c_str());
        }

        GLint operator[](const char *name) const {
            return get_location(name);
        }

        GLint operator[](const std::string &name) const {
            return get_location(name.c_str());
        }

        void set(const GLint &location, bool value) const {
            glUniform1i(location, static_cast<int>(value));
        }

        void set(const GLint &location, int value) const {
            glUniform1i(location, value);
        }

        void set(const GLint &location, float value) const {
            glUniform1f(location, value);
        }

        void set(const GLint &location, const glm::vec2 &value) const {
            glUniform2fv(location, 1, &value[0]);
        }

        void set(const GLint &location, const glm::vec3 &value) const {
            glUniform3fv(location, 1, &value[0]);
        }

        void set(const GLint &location, const glm::vec4 &value) const {
            glUniform4fv(location, 1, &value[0]);
        }

        void set(const GLint &location, gl::color value) const {
            glm::vec4 x = value.to_vector();
            glUniform4fv(location, 1, &x[0]);
        }

        void set(const GLint &location, const glm::mat2 &value) const {
            glUniformMatrix2fv(location, 1, GL_FALSE, &value[0][0]);
        }

        void set(const GLint &location, const glm::mat3 &value) const {
            glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
        }

        void set(const GLint &location, const glm::mat4 &value) const {
            glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
        }

        unsigned int id;
};

} // loaders
} // eldstar
