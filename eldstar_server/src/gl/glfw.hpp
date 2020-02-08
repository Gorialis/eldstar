#pragma once

#include <exception>
#include <string>

#include <GLFW/glfw3.h>


namespace eldstar {
namespace gl {

class glfw {
    public:
        glfw() {
            if (!glfwInit()) {
                const char* description;
                int code = glfwGetError(&description);

                throw load_failure(
                    "GLFW failed to initialize (code " + std::to_string(code) + "): " + std::string(description)
                );
            }

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_SAMPLES, 4);
        }

        ~glfw() {
            glfwTerminate();
        }

        void poll_events() {
            glfwPollEvents();
        }
};

class window {
    public:
        window(int width, int height, const char *title, bool make_context = false, GLFWmonitor* monitor = NULL, GLFWwindow* share = NULL) {
            ptr = glfwCreateWindow(width, height, title, monitor, share);

            if (ptr == NULL) {
                const char* description;
                int code = glfwGetError(&description);

                throw load_failure(
                    "Could not make GLFW window (code " + std::to_string(code) + "): " + std::string(description)
                );
            }

            if (make_context)
                this->make_context();
        }

        ~window() {
            glfwDestroyWindow(ptr);
        }

        GLFWwindow* get() {
            return ptr;
        }

        int get_width() const {
            int width, height;
            glfwGetWindowSize(ptr, &width, &height);
            return width;
        }

        int get_height() const {
            int width, height;
            glfwGetWindowSize(ptr, &width, &height);
            return height;
        }

        void set_size(int width, int height) {
            glfwSetWindowSize(ptr, width, height);
            glViewport(0, 0, width, height);
        }

        float aspect_ratio() const {
            int width, height;
            glfwGetWindowSize(ptr, &width, &height);
            return static_cast<float>(width) / static_cast<float>(height);
        }

        void make_context() {
            glfwMakeContextCurrent(ptr);
        }

        bool should_close() {
            return glfwWindowShouldClose(ptr);
        }

        void swap_buffers() {
            glfwSwapBuffers(ptr);
        }

    private:
        GLFWwindow* ptr;
};

} // gl
} // eldstar
