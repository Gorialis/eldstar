#pragma once

#include <cstdint>
#include <sstream>
#include <string>

#include "../gl/object_buffers.hpp"
#include "file.hpp"


namespace eldstar {
namespace loaders {

class stl {
    public:
        stl(const file& from) {
            // Read data from file
            std::stringstream buffer;
            buffer << from.rdbuf();
            buffer.seekg(0, std::ios::beg);

            // Read in the header and triangle count
            char stl_header[80] = "";
            char tri_count_encoded[4];

            buffer.read(stl_header, 80);
            buffer.read(tri_count_encoded, 4);

            // Convert the triangle count to a uint32
            uint32_t tri_count = *reinterpret_cast<uint32_t*>(&tri_count_encoded[0]);

            // Make a triangle list
            std::vector<gl::triangle> triangles;
            triangles.reserve(tri_count);

            for (uint32_t i = 0; i < tri_count; i++) {
                char triangle_buffer[50];
                buffer.read(triangle_buffer, 50);

                float* triangle_coords = reinterpret_cast<float*>(&triangle_buffer[0]);
                triangles.push_back(gl::triangle(
                    glm::vec3(triangle_coords[3], triangle_coords[4], triangle_coords[5]),
                    glm::vec3(triangle_coords[6], triangle_coords[7], triangle_coords[8]),
                    glm::vec3(triangle_coords[9], triangle_coords[10], triangle_coords[11]),
                    glm::vec3(triangle_coords[0], triangle_coords[1], triangle_coords[2])
                ));
            }

            this->buffer = std::make_shared<gl::object_buffers>(triangles);
        };

        void render() {
            buffer->render();
        }

    private:
        std::shared_ptr<gl::object_buffers> buffer;
};

} // loaders
} // eldstar
