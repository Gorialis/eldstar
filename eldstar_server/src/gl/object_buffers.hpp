#pragma once

#include <vector>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "primitives.hpp"


namespace eldstar {
namespace gl {

const float PI = 3.141592653589793238462643383279f;
const float SIN_45 = sinf(PI * 0.25f);
const float HROOT_2 = sqrtf(2.0f) / 2.0f;


class color {
    public:
        color() : red(1.0f), green(1.0f), blue(1.0f), alpha(1.0f) {};
        color(float magnitude, float alpha = 1.0f) : red(magnitude), green(magnitude), blue(magnitude), alpha(alpha) {};
        color(float red, float green, float blue, float alpha) : red(red), green(green), blue(blue), alpha(alpha) {};
        color(float id, float wrapPoint, float alpha);

        color operator* (const float& x);
        color& operator*= (const float& x);
        color operator* (const color& x);
        color& operator*= (const color& x);

        float red, green, blue, alpha;

        glm::vec4 to_vector();
};


class point {
    public:
        point() : position(glm::vec3(0.0f)), normal(glm::vec3(0.0f, 1.0f, 0.0f)), color() {};
        point(glm::vec3 position, glm::vec3 normal, color color) : position(position), normal(normal), color(color) {};
        point(glm::vec3 position, glm::vec3 normal, float red, float green, float blue, float alpha = 1.0f) : position(position), normal(normal), color(red, green, blue, alpha) {};

        glm::vec3 position, normal;
        gl::color color;

        void to_object_array(float* destination);
};


class triangle {
    public:
        triangle(point points[3]);
        triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, glm::vec3 normal, float red, float green, float blue, float alpha = 1.0f);
        triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, float red, float green, float blue, float alpha = 1.0f);
        triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, glm::vec3 normal, color color = color());
        triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, color color = color());

        point points[3];

        void to_object_array(float* destination);
};


class object_buffers {
    public:
        object_buffers(float* vertices, size_t size);
        object_buffers(std::vector<triangle>& triangles);

        void render();

    private:
        void create_buffers(float* vertices, size_t vertex_amount);

        vertex_array_manager A;
        buffer_manager B;

        size_t vertices;
};

} // gl
} // eldstar
