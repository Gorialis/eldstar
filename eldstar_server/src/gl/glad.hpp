#pragma once

#include <glad/glad.h>

#include "glfw.hpp"
#include "../exceptions.hpp"


namespace eldstar {
namespace gl {

class glad {
    public:
        glad() {
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                throw load_failure("glad was unable to load correctly (unsupported platform?)");
            }
        }
};

} // gl
} // eldstar
