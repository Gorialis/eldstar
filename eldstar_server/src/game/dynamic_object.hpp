#pragma once

#include "../gl/object_buffers.hpp"

namespace eldstar {
namespace game {

class dynamic_object {
    public:
        dynamic_object() : matrix(1.0f) {};
        dynamic_object(float x, float y, float z, float rotation)
            : matrix(
                glm::scale(
                    glm::rotate(
                        glm::translate(
                            glm::mat4(1.0f),
                            glm::vec3(x, y, z)
                        ),
                        glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f)
                    ),
                    glm::vec3(13.0f, 18.5f, 13.0f)
                )
            ) {};
        dynamic_object(float x, float y, float z, float diameter, float height, float rotation)
            : matrix(
                glm::scale(
                    glm::rotate(
                        glm::translate(
                            glm::mat4(1.0f),
                            glm::vec3(x, y, z)
                        ),
                        glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f)
                    ),
                    glm::vec3(diameter / 2.0f, height, diameter / 2.0f)
                )
            ) {};

        glm::mat4 matrix;
};

} // game
} // eldstar
