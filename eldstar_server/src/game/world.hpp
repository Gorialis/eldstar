#pragma once

#include <map>

#include "dynamic_object.hpp"
#include "geometry.hpp"
#include "item_object.hpp"
#include "world_object.hpp"
#include "../resources.hpp"
#include "../gl/object_buffers.hpp"


namespace eldstar {
namespace game {

class world {
    public:
        world() {};

        void add_triangle(long id, gl::triangle tri) {
            std::pair<std::map<long, geometry>::iterator, bool> inserted = zones.insert(std::pair<long, geometry>(id, geometry(id)));
            inserted.first->second.add_triangle(tri);
        }

        void add(long id, dynamic_object obj) {
            dynamic_objects.insert(std::pair<long, dynamic_object>(id, obj));
        }

        void add(long id, item_object obj) {
            item_objects.insert(std::pair<long, item_object>(id, obj));
        }

        void add(long id, world_object obj) {
            world_objects.insert(std::pair<long, world_object>(id, obj));
        }

        glm::mat4 get_camera_view_matrix() {
            return glm::translate(
                glm::lookAt(
                    camera_position,
                    camera_target,
                    glm::vec3(0.0f, 1.0f, 0.0f)
                ),
                glm::vec3(-camera_side_pan, 0.0f, 0.0f)
            );
        }

        void render(resource_manager& m, int color_mode, GLint color_id, GLint matrix_id, GLint mode_id) {
            // Render Mario
            m.mesh.set(color_id, gl::color(1.0f, 0.0f, 0.0f, 1.0f));
            m.mesh.set(matrix_id, glm::rotate(mario.matrix, -glm::radians(camera_yaw), glm::vec3(0.0f, 1.0f, 0.0f)));
            m.mesh.set(mode_id, 0);

            m.cylinder_pointer.render();
            m.cylinder_caps.render();

            m.mesh.set(color_id, gl::color(1.0f, 0.0f, 0.0f, 0.4f));
            m.cylinder_perimeter.render();

            // Render game camera
            m.mesh.set(matrix_id, glm::inverse(get_camera_view_matrix()));
            m.mesh.set(color_id, gl::color(0.4f, 0.4f, 0.4f, 1.0f));

            m.camera.render();

            // Render dynamic objects
            for (auto& pair : dynamic_objects) {
                auto color = gl::color(static_cast<float>(pair.first), 65.0f, 1.0f);
                m.mesh.set(color_id, color);
                m.mesh.set(matrix_id, pair.second.matrix);

                m.cylinder_pointer.render();
                m.cylinder_caps.render();

                color.alpha *= 0.4f;

                m.mesh.set(color_id, color);
                m.cylinder_perimeter.render();
            }

            // Render item objects
            for (auto& pair : item_objects) {
                m.mesh.set(color_id, gl::color(static_cast<float>(pair.first), 65.0f, 1.0f));
                m.mesh.set(matrix_id, pair.second.matrix);

                m.cylinder_caps.render();
                m.cylinder_perimeter.render();
            }

            m.mesh.set(mode_id, color_mode);

            // Render world objects
            for (auto& pair : world_objects) {
                m.mesh.set(color_id, gl::color(static_cast<float>(pair.first), 32.0f, 1.0f));
                m.mesh.set(matrix_id, pair.second.matrix);

                m.world_object.render();
            }

            // Render world geometry
            m.mesh.set(matrix_id, glm::mat4(1.0f));

            for (auto& pair : zones) {
                m.mesh.set(color_id, gl::color(static_cast<float>(pair.first), 75.0f, 1.0f));

                pair.second.render_buffer();
            }
        }

        std::map<long, geometry> zones;
        std::map<long, dynamic_object> dynamic_objects;
        std::map<long, item_object> item_objects;
        std::map<long, world_object> world_objects;
        dynamic_object mario;

        float camera_side_pan;
        float camera_yaw;
        glm::vec3 camera_position;
        glm::vec3 camera_target;

        long frame;
        std::string description;
};

} // game
} // eldstar
