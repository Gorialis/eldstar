#pragma once

#include "../exceptions.hpp"
#include "glfw.hpp"
#include "primitives.hpp"


namespace eldstar {
namespace gl {

class camera {
    public:
        camera() {}

        virtual glm::mat4 matrix(const window& w) {
            throw not_implemented();
        }
};

class perspective_camera : public camera {
    public:
        perspective_camera(
                glm::vec3 position,
                glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f),
                float fov = 25.0f,
                float close_plane = 0.1f,
                float far_plane = 10000.0f
            ) :
            camera(),
            position(position),
            target(target),
            fov(fov),
            close_plane(close_plane),
            far_plane(far_plane)
        {}

        virtual glm::mat4 matrix(const window& w) {
            glm::mat4 view = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 projection = glm::perspective(glm::radians(fov), w.aspect_ratio(), close_plane, far_plane);

            return projection * view;
        }

        glm::vec3 position;
        glm::vec3 target;
        float fov;
        float close_plane;
        float far_plane;
};

class ortho_camera : public camera {
    public:
        ortho_camera() : camera() {};

        virtual glm::mat4 matrix(const window& w) {
            return glm::ortho(0.0f, static_cast<float>(w.get_width()), 0.0f, static_cast<float>(w.get_height()));
        }
};

} // gl
} // eldstar
