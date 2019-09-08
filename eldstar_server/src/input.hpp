#pragma once

#include <map>

#include "gl/primitives.hpp"


namespace eldstar {
namespace input {

enum action_type {
    down,
    held,
    up,
    released
};

struct state {
    state() : action(released), change_time(0.0), mods(0) {};
    state(action_type action, double change_time, int mods)
        : action(action), change_time(change_time), mods(mods) {};

    action_type action;
    double change_time;
    int mods;
};

class state_manager {
    public:
        state_manager() {};

        void keyboard_event(int key, int action, int mods) {
            switch (action) {
                case GLFW_PRESS:
                keyboard[key] = state(down, glfwGetTime(), mods);
                break;

                case GLFW_RELEASE:
                keyboard[key] = state(up, glfwGetTime(), mods);
                break;

                default: break;
            }
        }

        void mouse_event(int button, int action, int mods) {
            switch (action) {
                case GLFW_PRESS:
                mouse[button] = state(down, glfwGetTime(), mods);
                break;

                case GLFW_RELEASE:
                mouse[button] = state(up, glfwGetTime(), mods);
                break;

                default: break;
            }
        }

        void cursor_position_event(double x_pos, double y_pos) {
            cursor_position = glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos));
        }

        void scroll_offset(double x_pos, double y_pos) {
            scroll_delta += glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos));
        }

        void update() {
            for (auto& k : keyboard) {
                switch (k.second.action) {
                    case down:
                    k.second.action = held;
                    break;

                    case up:
                    k.second.action = released;
                    break;

                    default: break;
                }
            }

            for (auto& b : mouse) {
                switch (b.second.action) {
                    case down:
                    b.second.action = held;
                    break;

                    case up:
                    b.second.action = released;
                    break;

                    default: break;
                }
            }

            cursor_delta = cursor_position - last_cursor_position;
            scroll_delta = glm::vec2(0.0f);

            last_cursor_position = cursor_position;
        }

        std::map<int, state> keyboard;
        std::map<int, state> mouse;

        glm::vec2 cursor_position;
        glm::vec2 last_cursor_position;

        glm::vec2 cursor_delta;
        glm::vec2 scroll_delta;

};

} // input
} // eldstar
