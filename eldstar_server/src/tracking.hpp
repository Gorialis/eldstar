#pragma once

#include "game/world.hpp"
#include "gl/camera.hpp"
#include "exceptions.hpp"


namespace eldstar {

// Types of objects the camera can lock tracking to
enum tracking_type {
    track_none,
    track_mario,
    track_dynamic_object,
    track_item_object,
    track_world_object
};

struct tracking_target {
    tracking_type type;
    long id;

    tracking_target() : type(track_none), id(0) {};
    tracking_target(tracking_type type, long id) : type(type), id(id) {};
    operator bool() const { return type != track_none; };
};

void camera_track_target(gl::perspective_camera& camera, game::world& world, const tracking_target& target) {
    if (!target) return;

    glm::mat4 matrix;

    switch (target.type) {
        case track_mario:
            matrix = world.mario.matrix;
            break;
        case track_dynamic_object:
            matrix = world.dynamic_objects[target.id].matrix;
            break;
        case track_item_object:
            matrix = world.item_objects[target.id].matrix;
            break;
        case track_world_object:
            matrix = world.item_objects[target.id].matrix;
            break;
        default: return;  // Unknown tracking type?
    }

    glm::vec4 track_location = matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    if (track_location.w == 0.0f){
        // If the homogenous component is 0, this is probably a null matrix.
        // This indicates an uninitialized object, or in other words, one that cannot actually be tracked right now.
        return;
    }

    // Move the target to the new tracked location and move the position accordingly
    glm::vec3 delta = camera.position - camera.target;
    camera.target = glm::vec3(track_location);
    camera.position = camera.target + delta;
}

} // eldstar
