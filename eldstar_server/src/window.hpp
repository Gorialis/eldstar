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
        window(std::string window_title = "Eldstar")
            : gl_window(1184, 800, window_title.c_str(), true),
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

            // Set the user pointer of the GLFWwindow to this class
            glfwSetWindowUserPointer(gl_window.get(), this);

            // Set up input handling
            input_state = std::make_shared<input::state_manager>();

            // Set default content scale
            glfwGetWindowContentScale(gl_window.get(), &content_scale.x, &content_scale.y);

            // Set various callbacks
            glfwSetFramebufferSizeCallback(gl_window.get(), [](GLFWwindow* w, int width, int height){
                glfwSetWindowSize(w, width, height);
                glViewport(0, 0, width, height);
            });

            glfwSetWindowContentScaleCallback(gl_window.get(), [](GLFWwindow* w, float xscale, float yscale){
                window* wd = reinterpret_cast<window*>(glfwGetWindowUserPointer(w));

                wd->content_scale = glm::vec2(xscale, yscale);
            });

            glfwSetKeyCallback(gl_window.get(), [](GLFWwindow* w, int key, int scan_code, int action, int mods){
                window* wd = reinterpret_cast<window*>(glfwGetWindowUserPointer(w));

                wd->input_state->keyboard_event(key, action, mods);
            });

            glfwSetMouseButtonCallback(gl_window.get(), [](GLFWwindow* w, int button, int action, int mods){
                window* wd = reinterpret_cast<window*>(glfwGetWindowUserPointer(w));

                wd->input_state->mouse_event(button, action, mods);
            });

            glfwSetCursorPosCallback(gl_window.get(), [](GLFWwindow* w, double x_pos, double y_pos){
                window* wd = reinterpret_cast<window*>(glfwGetWindowUserPointer(w));

                wd->input_state->cursor_position_event(x_pos, y_pos);
            });

            glfwSetScrollCallback(gl_window.get(), [](GLFWwindow* w, double x_pos, double y_pos){
                window* wd = reinterpret_cast<window*>(glfwGetWindowUserPointer(w));

                wd->input_state->scroll_offset(x_pos, y_pos);
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

        glm::vec2 content_scale;

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
