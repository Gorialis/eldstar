
#include "object_buffers.hpp"

namespace eldstar {
namespace gl {

color::color(float ID, float wrapPoint, float alpha) : alpha(alpha) {
    float h = fmod((ID / wrapPoint), 1.0f);
    float s = 0.6f;
    float v = 1.0f;

    int i = static_cast<int>(floorf(h * 6.0f));
    float f = (h * 6.0f) - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - (s * f));
    float t = v * (1.0f - (s * (1.0f - f)));
    i = i % 6;

    switch (i) {
        case 0:
            red = v;
            green = t;
            blue = p;
            return;
        case 1:
            red = q;
            green = v;
            blue = p;
            return;
        case 2:
            red = p;
            green = v;
            blue = t;
            return;
        case 3:
            red = p;
            green = q;
            blue = v;
            return;
        case 4:
            red = t;
            green = p;
            blue = v;
            return;
        case 5:
        default:
            red = v;
            green = p;
            blue = q;
            return;
    }
}

color color::operator* (const float& x) {
    return color(red, green, blue, alpha * x);
}

color& color::operator*= (const float& x) {
    alpha *= x;

    return *this;
}

color color::operator* (const color& x) {
    return color(red * x.red, green * x.green, blue * x.blue, alpha * x.alpha);
}

color& color::operator*= (const color& x) {
    red *= x.red;
    green *= x.green;
    blue *= x.blue;
    alpha *= x.alpha;

    return *this;
}

glm::vec4 color::to_vector() {
    return glm::vec4(red, green, blue, alpha);
}


void point::to_object_array(float* destination) {
    destination[0] = position.x;
    destination[1] = position.y;
    destination[2] = position.z;
    destination[3] = normal.x;
    destination[4] = normal.y;
    destination[5] = normal.z;
    destination[6] = color.red;
    destination[7] = color.green;
    destination[8] = color.blue;
    destination[9] = color.alpha;
}



triangle::triangle(point points[3]) {
    this->points[0] = points[0];
    this->points[1] = points[1];
    this->points[2] = points[2];
}

triangle::triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, glm::vec3 normal, float red, float green, float blue, float alpha) {
    this->points[0] = point(one, normal, red, green, blue, alpha);
    this->points[1] = point(two, normal, red, green, blue, alpha);
    this->points[2] = point(three, normal, red, green, blue, alpha);
}

triangle::triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, float red, float green, float blue, float alpha) {
    glm::vec3 normal = glm::normalize(glm::cross(two - one, three - one));

    this->points[0] = point(one, normal, red, green, blue, alpha);
    this->points[1] = point(two, normal, red, green, blue, alpha);
    this->points[2] = point(three, normal, red, green, blue, alpha);
}

triangle::triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, glm::vec3 normal, color color) {
    this->points[0] = point(one, normal, color);
    this->points[1] = point(two, normal, color);
    this->points[2] = point(three, normal, color);
}

triangle::triangle(glm::vec3 one, glm::vec3 two, glm::vec3 three, color color) {
    glm::vec3 normal = glm::normalize(glm::cross(two - one, three - one));

    this->points[0] = point(one, normal, color);
    this->points[1] = point(two, normal, color);
    this->points[2] = point(three, normal, color);
}

void triangle::to_object_array(float* destination) {
    for (char i = 0; i < 3; i++) {
        this->points[i].to_object_array(&destination[10 * i]);
    }
}


object_buffers::object_buffers(float* vertices, size_t vertex_amount) {
    create_buffers(vertices, vertex_amount);
}

object_buffers::object_buffers(std::vector<triangle>& triangles) {
    size_t tri_count = triangles.size();
    std::vector<float> point_array;

    point_array.resize(30 * tri_count);
    float* pt_ptr = point_array.data();

    for (size_t i = 0; i < tri_count; i++) {
        triangles[i].to_object_array(&pt_ptr[30 * i]);
    }

    create_buffers(pt_ptr, tri_count * 3);
}

void object_buffers::render() {
    if (vertices == 0) return;

    vertex_array_binder aBinder(A);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices));
}

void object_buffers::create_buffers(float* vertices, size_t vertex_amount) {
    this->vertices = vertex_amount;

    if (vertex_amount == 0) return;

    vertex_array_binder aBinder(A);
    buffer_binder bBinder(B);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_amount * 10, vertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

} // gl
} // eldstar
