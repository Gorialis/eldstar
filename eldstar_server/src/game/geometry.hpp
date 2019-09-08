#pragma once

#include "../gl/object_buffers.hpp"

namespace eldstar {
namespace game {

class geometry {
    public:
        geometry(long id) : id(id), needs_update(true) {};

        void add_triangle(gl::triangle& tri) {
            triangles.push_back(tri);
            needs_update = true;
        }

        void render_buffer() {
            if (!buffer || needs_update) {
                buffer = std::make_shared<gl::object_buffers>(triangles);
                needs_update = false;
            }

            buffer->render();
        }

        long id;

    private:
        std::vector<gl::triangle> triangles;

        std::shared_ptr<gl::object_buffers> buffer;
        bool needs_update;
};

} // game
} // eldstar
