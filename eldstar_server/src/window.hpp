#pragma once

#include "gl/glad.hpp"
#include "gl/glfw.hpp"
#include "input.hpp"
#include "menu.hpp"
#include "tracking.hpp"

#ifndef GIT_REV_COUNT
#define GIT_REV_COUNT 0
#endif


namespace eldstar {

std::weak_ptr<input::state_manager> global_state_manager;

class window {
    /**
     * This class covers all the startup mechanics of the Eldstar server window.
     *
     * Instantiating this class will load all of the requirements (GL, etc) and either produce a fully drawable GL window
     * or throw an exception.
     *
     * You should not instantiate more than one of this class in a given runtime.
     */

    public:
        window()
            : gl_window(1184, 800, "Eldstar", true),
              start_time(glfwGetTime()), last_time(start_time),
              wireframe(false), mirror_game_camera(false), recording(false), recording_has_ui(false), color_mode(0), show_fps(false)
        {
            // Enable depth-testing and set the blend function
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Enable single-channel images for masking
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            // Set the wireframe line width to 2.0f
            glLineWidth(2.0f);

            // Enable anti-aliasing
            glEnable(GL_MULTISAMPLE);

            // Set up input handling
            input_state = std::make_shared<input::state_manager>();
            if (!global_state_manager.lock())
                global_state_manager = input_state;

            glfwSetFramebufferSizeCallback(gl_window.get(), [](GLFWwindow* w, int width, int height){
                glfwSetWindowSize(w, width, height);
                glViewport(0, 0, width, height);
            });

            glfwSetKeyCallback(gl_window.get(), [](GLFWwindow* w, int key, int scan_code, int action, int mods){
                if (auto spt = global_state_manager.lock()) {
                    spt->keyboard_event(key, action, mods);
                }
            });

            glfwSetMouseButtonCallback(gl_window.get(), [](GLFWwindow* w, int button, int action, int mods){
                if (auto spt = global_state_manager.lock()) {
                    spt->mouse_event(button, action, mods);
                }
            });

            glfwSetCursorPosCallback(gl_window.get(), [](GLFWwindow* w, double x_pos, double y_pos){
                if (auto spt = global_state_manager.lock()) {
                    spt->cursor_position_event(x_pos, y_pos);
                }
            });

            glfwSetScrollCallback(gl_window.get(), [](GLFWwindow* w, double x_pos, double y_pos){
                if (auto spt = global_state_manager.lock()) {
                    spt->scroll_offset(x_pos, y_pos);
                }
            });
        }

        bool update() {
            if (gl_window.should_close()) return false;

            double now = glfwGetTime();
            delta_time = now - last_time;
            last_time = now;

            gl_window.swap_buffers();

            if(input_state)
                input_state->update();

            glfw.poll_events();

            return true;
        }

        double runtime() const {
            return glfwGetTime() - start_time;
        }

        void close() {
            glfwSetWindowShouldClose(gl_window.get(), 1);
        }

        gl::glfw glfw;
        gl::window gl_window;
        gl::glad glad;

        std::shared_ptr<input::state_manager> input_state;
        bool wireframe;
        bool mirror_game_camera;
        bool recording;
        bool recording_has_ui;
        bool show_fps;
        int color_mode;

        std::unique_ptr<menu> active_menu;
        status_bar status;

        tracking_target track_target;

        double delta_time;

    private:
        double start_time;
        double last_time;
};

} // eldstar
