#pragma once

#include "../gl/object_buffers.hpp"

namespace eldstar {
namespace game {

class world_object {
    public:
        world_object() : matrix(0.0f) {};
        world_object(float x, float y, float z, float x_rotation, float y_rotation, float z_rotation, float width, float height, float depth)
            : matrix(
                glm::scale(
                    glm::rotate(
                        glm::rotate(
                            glm::rotate(
                                glm::translate(
                                    glm::mat4(1.0f),
                                    glm::vec3(x, y, z)
                                ),
                                glm::radians(y_rotation),
                                glm::vec3(0.0f, 1.0f, 0.0f)
                            ),
                            glm::radians(x_rotation),
                            glm::vec3(1.0f, 0.0f, 0.0f)
                        ),
                        glm::radians(z_rotation),
                        glm::vec3(0.0f, 0.0f, 1.0f)
                    ),
                    glm::vec3(width, height, depth)
                )
            ) {};

        glm::mat4 matrix;
};

} // game
} // eldstar
