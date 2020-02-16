
#include <iostream>
#include <memory>

#include "parser.hpp"
#include "exceptions.hpp"
#include "resources.hpp"
#include "window.hpp"
#include "controls.hpp"
#include "tracking.hpp"
#include "game/world.hpp"
#include "gl/camera.hpp"
#include "gl/dump.hpp"


int main(int argc, char** argv) {
    bool dry_run = argc >= 2 && strcmp(argv[1], "dryrun") == 0;

    std::unique_ptr<eldstar::window> window;
    std::unique_ptr<eldstar::resource_manager> resources;
    std::unique_ptr<eldstar::socket_parser> parser;

    try {
        window = std::unique_ptr<eldstar::window>(new eldstar::window());
        resources = std::unique_ptr<eldstar::resource_manager>(new eldstar::resource_manager());
        parser = std::unique_ptr<eldstar::socket_parser>(new eldstar::socket_parser());
    } catch (eldstar::load_failure& e) {
        e.report();
        return -1;
    }

    const GLFWimage icons[4] = {
        {128, 128, const_cast<unsigned char*>(resources->icon_128.get())},
        {48, 48, const_cast<unsigned char*>(resources->icon_48.get())},
        {32, 32, const_cast<unsigned char*>(resources->icon_32.get())},
        {16, 16, const_cast<unsigned char*>(resources->icon_16.get())}
    };

    glfwSetWindowIcon(window->gl_window.get(), 4, icons);

    // Store perspective scene IDs
    resources->mesh.use();
    GLint camera_matrix = resources->mesh["camera"];
    GLint camera_pos = resources->mesh["camPos"];
    GLint light_pos = resources->mesh["lightPos"];
    GLint model_matrix = resources->mesh["model"];
    GLint model_color = resources->mesh["globalColor"];
    GLint color_mode = resources->mesh["colorMode"];
    eldstar::gl::perspective_camera camera(glm::vec3(0.0f, 650.0f, 1125.0f));

    // Store orthographic (text) scene IDs
    resources->text.use();
    GLint color = resources->text["text_color"];
    GLint projection = resources->text["projection"];
    eldstar::gl::ortho_camera ortho_camera;

    // Set up game world storage for swapping
    std::shared_ptr<eldstar::game::world> world;
    bool swapped = false;

    // Run main event loop
    while (window->update()) {
        // Check if a new world is ready, if so, load it
        std::shared_ptr<eldstar::game::world> interim = parser->get_world();
        if (interim) {
            world = interim;
            swapped = true;
        } else {
            swapped = false;
        }

        // Clear the screen
        glEnable(GL_DEPTH_TEST);
        if (window->recording)
            glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        else
            glClearColor(0.05f, 0.1f, 0.1f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle input
        if (!eldstar::controls::menu_control(*window, *resources, camera, color))
            eldstar::controls::orbiting_camera_control(*window, camera);
        eldstar::controls::display_control(*window);

        // Track target if set
        if (world)
            camera_track_target(camera, *world, window->track_target);

        // Enable the mesh shader
        resources->mesh.use();

        // Set up the camera and default modes
        if (window->mirror_game_camera && world) {
            glm::mat4 view = world->get_camera_view_matrix();
            glm::mat4 inverse_view = glm::inverse(view);

            glm::vec3 center = glm::vec3(inverse_view * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            glm::vec3 track = glm::vec3(inverse_view * glm::vec4(0.0f, 0.0f, -glm::length(
                world->camera_position - world->camera_target
            ), 1.0f));

            camera.position = center;
            camera.target = track;

            glm::mat4 projection = glm::perspective(glm::radians(25.0f), window->gl_window.aspect_ratio(), 90.0f, 4096.0f);

            resources->mesh.set(camera_matrix, projection * view);
            resources->mesh.set(camera_pos, center);
        } else {
            resources->mesh.set(camera_matrix, camera.matrix(window->gl_window));
            resources->mesh.set(camera_pos, camera.position);
        }
        resources->mesh.set(light_pos, glm::vec3(500.0f, 1000.0f, 1000.0f));
        resources->mesh.set(model_matrix, glm::mat4(1.0f));
        resources->mesh.set(model_color, glm::vec4(1.0f));
        resources->mesh.set(color_mode, window->color_mode);

        parser->patchback_deferred = camera.position;
        parser->patchback_target = camera.target;

        if (world) {
            if (window->wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                world->render(*resources, window->color_mode, model_color, model_matrix, color_mode);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                // Prime depth buffer
                glDisable(GL_CULL_FACE);
                glDepthFunc(GL_LESS);
                world->render(*resources, window->color_mode, model_color, model_matrix, color_mode);

                // Render with alpha = f*alpha
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                glDepthFunc(GL_ALWAYS);
                world->render(*resources, window->color_mode, model_color, model_matrix, color_mode);

                // Render with alpha = (alpha - f*alpha)/(1.0 - f*alpha)
                glDepthFunc(GL_LEQUAL);
                world->render(*resources, window->color_mode, model_color, model_matrix, color_mode);

                // Render with alpha = f*alpha
                glCullFace(GL_BACK);
                glDepthFunc(GL_ALWAYS);
                world->render(*resources, window->color_mode, model_color, model_matrix, color_mode);

                // Render with alpha = (alpha - f*alpha)/(1.0 - f*alpha)
                glDisable(GL_CULL_FACE);
                glDepthFunc(GL_LEQUAL);
                world->render(*resources, window->color_mode, model_color, model_matrix, color_mode);
            }

            if (swapped && window->recording && !window->recording_has_ui)
                eldstar::gl::dump_png("framedump/" + std::to_string(world->frame) + ".png", window->gl_window);

        } else {
            resources->mesh.set(model_color, eldstar::gl::color(0.4f, 0.4f, 0.4f, 1.0f));
            resources->camera.render();
        }


        // Enable the orthographic (text) shader
        resources->text.use();

        // Disable depth test
        glDisable(GL_DEPTH_TEST);

        // Set the projection matrix and text color
        resources->text.set(projection, ortho_camera.matrix(window->gl_window));
        resources->text.set(color, glm::vec3(1.0f, 1.0f, 1.0f));

        // Render Eldstar version
        glm::vec2 version_end = resources->opensans.render_utf8_bordered("Eldstar v1.0." + std::to_string(GIT_REV_COUNT) + "b", glm::vec2(20.0f, 45.0f));

        if (window->show_fps) {
            float delta = static_cast<float>(window->delta_time);
            if (delta != 0.0f) {
                float fps = 1.0f / delta;
                std::stringstream buffer;
                buffer.precision(4);
                buffer << "Window FPS: " << fps;
                resources->opensans.render_utf8_bordered(buffer.str(), glm::vec2(20.0f, 70.0f));
            }
            else
                resources->opensans.render_utf8_bordered("Window FPS: inf", glm::vec2(20.0f, 70.0f));
        }

        if (window->recording) {
            resources->text.set(color, glm::vec3(1.0f, 0.5f, 0.5f));
            resources->opensans.render_utf8_bordered(" (recording)", version_end);
        }

        // If a status message is present, show it
        if (window->status) {
            resources->text.set(color, glm::vec3(1.0f, 1.0f, 0.2f));
            resources->opensans.render_utf8_bordered(window->status, glm::vec2(20.0f, 20.0f));
        } else {
            // Else show useragent + frame
            resources->text.set(color, glm::vec3(1.0f, 1.0f, 1.0f));
            if (world)
                resources->opensans.render_utf8_bordered(world->description, glm::vec2(20.0f, 20.0f));
            else
                resources->opensans.render_utf8_bordered("No client connected", glm::vec2(20.0f, 20.0f));
        }

        resources->text.set(color, glm::vec3(1.0f, 1.0f, 1.0f));
        window->status -= window->delta_time;

        float window_top = static_cast<float>(window->gl_window.get_height()) - static_cast<float>(resources->opensans.size) - 20.0f;

        if (window->active_menu)
            window->active_menu->render(20.0f, window_top, *resources, color);
        else
            resources->opensans.render_utf8_bordered("Press [M] to open the menu", glm::vec2(20.0f, window_top));

        if (world && swapped && window->recording && window->recording_has_ui)
            eldstar::gl::dump_png("framedump/" + std::to_string(world->frame) + ".png", window->gl_window);

        // If this was a dry run, exit successfully.
        if (dry_run) {
            std::cout << "dry run completed successfully" << std::endl;
            window->close();
        }
    }
}
