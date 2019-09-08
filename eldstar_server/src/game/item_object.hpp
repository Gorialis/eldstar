#pragma once

#include "../gl/object_buffers.hpp"

namespace eldstar {
namespace game {

class item_object {
    public:
        item_object() : matrix(1.0f) {};
        item_object(float x, float y, float z)
            : matrix(
                glm::scale(
                    glm::translate(
                        glm::mat4(1.0f),
                        glm::vec3(x, y, z)
                    ),
                    glm::vec3(6.75f, 27.0f, 6.75f)
                )
            ) {};

        glm::mat4 matrix;
};

} // game
} // eldstar
