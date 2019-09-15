#pragma once

#include <algorithm>
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
        : title(title), options(options), back(back), index(0), view_index(0), submenu(nullptr) {};

    std::string title;
    int index;
    int view_index;

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

        if (view_index >= index) view_index = std::max(index - 1, 0);
        if (index - 8 > view_index) view_index = index - 8;

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

        if (view_index >= index) view_index = std::max(index - 1, 0);
        if (index - 8 > view_index) view_index = index - 8;

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

        // Calculate menu bounds
        size_t start = static_cast<size_t>(view_index);
        size_t end = start + 10;
        size_t max = static_cast<size_t>(options.size());
        float top_needle = needle;

        bool skip_first, skip_last;

        if (skip_first = start > 0) {
            needle -= line_spacing;
            start++;
        }

        if (skip_last = max > end) {
            end--;
        }


        // Draw options
        for (size_t i = start; i < std::min(end, max); i++) {
            if (i == index)
                r.text.set(color_id, selected);
            else
                r.text.set(color_id, other);

            right = fmaxf(right, r.opensans.render_utf8_bordered(options[i].name, glm::vec2(x, needle)).x);
            needle -= line_spacing;
        }

        // Draw menu extensions if present
        float center = ((x + right) / 2.0f) - (line_spacing * 0.4f);

        r.text.set(color_id, top);
        if (skip_first)
            r.opensans.render_utf8_bordered(u8"\u2bc5", glm::vec2(center, top_needle));
        if (skip_last)
            r.opensans.render_utf8_bordered(u8"\u2bc6", glm::vec2(center, needle));

        // Draw submenu if present
        if (submenu)
            submenu->render(right + line_spacing, y, r, color_id);
    }
};

} // eldstar
