#pragma once

#include <functional>
#include <memory>
#include <string>

#include "resources.hpp"


namespace eldstar {

struct menu_option {
    std::string name;
    std::function<void(void*)> callback;

    menu_option() : name("<unset>"), callback([](void* p){}) {};
    menu_option(std::string name, std::function<void(void*)> callback) : name(name), callback(callback) {};
    void operator()(void* ptr) { callback(ptr); };
};

struct status_bar {
    std::string text;
    float countdown;

    status_bar& operator-=(float value) { countdown -= value; return *this; };
    status_bar& operator-=(double value) { countdown -= static_cast<float>(value); return *this; };

    operator bool() { return countdown >= 0.0f; };
    operator std::string() { return text; };

    status_bar() : text(), countdown(0.0f) {};
    status_bar(std::string message, float timeout = 2.0f) : text(message), countdown(timeout) {};
    void operator()(std::string message, float timeout = 2.0f) { text = message; countdown = timeout; };
};

struct menu {
    menu(std::string title, std::vector<menu_option> options = {}, std::function<void()> back = []() {})
        : title(title), options(options), back(back), index(0), submenu(nullptr) {};

    std::string title;
    int index;

    std::vector<menu_option> options;
    std::function<void()> back;

    std::unique_ptr<menu> submenu;

    // Operators
    menu& operator++() {
        size_t max = options.size();

        if (max == 0) {
            index = 0;
            return *this;
        }

        index++;
        if (index >= max) index = 0;

        return *this;
    }

    menu& operator--() {
        size_t max = options.size();

        if (max == 0) {
            index = 0;
            return *this;
        }

        index--;
        if (index < 0) index = static_cast<int>(max) - 1;

        return *this;
    }

    // Functions
    menu* get_head() {
        menu* current = this;

        while (current->submenu)
            current = current->submenu.get();

        return current;
    }

    void set_submenu(std::string sub_title, std::vector<menu_option> sub_options) {
        submenu.reset(new menu(
            sub_title,
            sub_options,
            [this](){ submenu.reset(); }
        ));
    }

    void select() {
        size_t max = options.size();

        if (max == 0 || index >= max || index < 0)
            return;

        options[index](this);
    }

    void render(float x, float y, resource_manager& r, GLint color_id) {
        glm::vec3 top(1.0f, 1.0f, 1.0f);
        glm::vec3 selected(0.8f, 0.8f, 1.0f);
        glm::vec3 other(0.7f, 0.7f, 0.7f);

        glm::vec3 defocused(0.5f, 0.5f, 0.5f);

        // If a submenu is active, defocus this menu element
        if (submenu) {
            top *= defocused;
            selected *= defocused;
            other *= defocused;
        }

        float line_spacing = static_cast<float>(r.opensans.size) * 1.2f;
        float right = x;
        float needle = y;

        // Draw title
        r.text.set(color_id, top);
        right = fmaxf(right, r.opensans.render_utf8_bordered(title, glm::vec2(x, needle)).x);
        needle -= line_spacing;

        // Draw options
        for (size_t i = 0; i < options.size(); i++) {
            if (i == index)
                r.text.set(color_id, selected);
            else
                r.text.set(color_id, other);

            right = fmaxf(right, r.opensans.render_utf8_bordered(options[i].name, glm::vec2(x, needle)).x);
            needle -= line_spacing;
        }

        // Draw submenu if present
        if (submenu)
            submenu->render(right + line_spacing, y, r, color_id);
    }
};

} // eldstar
