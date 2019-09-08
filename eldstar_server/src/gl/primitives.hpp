#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace eldstar {
namespace gl {

class vertex_array_manager {
    public:
        vertex_array_manager(GLsizei amount = 1) : Amount(amount), IDs(amount) {
            glGenVertexArrays(amount, IDs.data());
        }

        ~vertex_array_manager() {
            glDeleteVertexArrays(Amount, IDs.data());
        }

        const unsigned int operator[](GLsizei index) const {
            return IDs[index];
        }


    private:
        vertex_array_manager(const vertex_array_manager&) = delete;
        vertex_array_manager& operator=(const vertex_array_manager&) = delete;

        GLsizei Amount;
        std::vector<unsigned int> IDs;
};


class vertex_array {
    public:
        vertex_array(GLsizei amount = 1) : Buffer(std::make_shared<vertex_array_manager>(amount)) {};

        const unsigned int operator[](GLsizei index) const {
            return (*Buffer)[index];
        }

    private:
        std::shared_ptr<vertex_array_manager> Buffer;
};


class vertex_array_binder {
    public:
        vertex_array_binder() {};
        vertex_array_binder(vertex_array_manager& v, GLsizei index = 0) {
            bind(v, index);
        }
        vertex_array_binder(vertex_array& v, GLsizei index = 0) {
            bind(v, index);
        }
        vertex_array_binder(unsigned int x) {
            bind(x);
        }

        ~vertex_array_binder() {
            bind();
        };

        void bind(vertex_array_manager& v, GLsizei index = 0) {
            glBindVertexArray(v[index]);
        }

        void bind(vertex_array& v, GLsizei index = 0) {
            glBindVertexArray(v[index]);
        }

        void bind(unsigned int x) {
            glBindVertexArray(x);
        }

        void bind() {
            glBindVertexArray(0);
        }
};



class buffer_manager {
    public:
        buffer_manager(GLsizei amount = 1) : Amount(amount), IDs(amount) {
            glGenBuffers(amount, IDs.data());
        }

        ~buffer_manager() {
            glDeleteBuffers(Amount, IDs.data());
        }

        const unsigned int operator[](GLsizei index) const {
            return IDs[index];
        }


    private:
        buffer_manager(const buffer_manager&) = delete;
        buffer_manager& operator=(const buffer_manager&) = delete;

        GLsizei Amount;
        std::vector<unsigned int> IDs;
};


class buffer {
    public:
        buffer(GLsizei amount = 1) : Buffer(std::make_shared<buffer_manager>(amount)) {};

        const unsigned int operator[](GLsizei index) const {
            return (*Buffer)[index];
        }

    private:
        std::shared_ptr<buffer_manager> Buffer;
};


class buffer_binder {
    public:
        buffer_binder() {};
        buffer_binder(buffer_manager& v, GLsizei index = 0) {
            bind(v, index);
        }
        buffer_binder(buffer& v, GLsizei index = 0) {
            bind(v, index);
        }
        buffer_binder(unsigned int x) {
            bind(x);
        }

        ~buffer_binder() {
            bind();
        };

        void bind(buffer_manager& v, GLsizei index = 0) {
            glBindBuffer(GL_ARRAY_BUFFER, v[index]);
        }

        void bind(buffer& v, GLsizei index = 0) {
            glBindBuffer(GL_ARRAY_BUFFER, v[index]);
        }

        void bind(unsigned int x) {
            glBindBuffer(GL_ARRAY_BUFFER, x);
        }

        void bind() {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
};



class texture_manager {
    public:
        texture_manager(GLsizei amount = 1) : Amount(amount), IDs(amount) {
            glGenTextures(amount, IDs.data());
        }

        ~texture_manager() {
            glDeleteTextures(Amount, IDs.data());
        }

        const unsigned int operator[](GLsizei index) const {
            return IDs[index];
        }


    private:
        texture_manager(const texture_manager&) = delete;
        texture_manager& operator=(const texture_manager&) = delete;

        GLsizei Amount;
        std::vector<unsigned int> IDs;
};


class texture {
    public:
        texture(GLsizei amount = 1) : Buffer(std::make_shared<texture_manager>(amount)) {};

        const unsigned int operator[](GLsizei index) const {
            return (*Buffer)[index];
        }

    private:
        std::shared_ptr<texture_manager> Buffer;
};


class texture_binder {
    public:
        texture_binder() {};
        texture_binder(texture_manager& v, GLsizei index = 0){
            bind(v, index);
        }
        texture_binder(texture& v, GLsizei index = 0){
            bind(v, index);
        }
        texture_binder(unsigned int x) {
            bind(x);
        }

        ~texture_binder() {
            bind();
        };

        void bind(texture_manager& v, GLsizei index = 0) {
            glBindTexture(GL_TEXTURE_2D, v[index]);
        }

        void bind(texture& v, GLsizei index = 0) {
            glBindTexture(GL_TEXTURE_2D, v[index]);
        }

        void bind(unsigned int x) {
            glBindTexture(GL_TEXTURE_2D, x);
        }

        void bind() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
};

} // gl
} // eldstar
