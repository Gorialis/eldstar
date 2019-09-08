#pragma once

#include <png.h>
#include <zlib.h>

#include "../exceptions.hpp"
#include "glfw.hpp"
#include "primitives.hpp"


namespace eldstar {
namespace gl {

void dump_png(const std::string& filename, window& w) {
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL)
        throw std::bad_alloc();

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        throw std::bad_alloc();
    }

    FILE* fp;

    #ifdef _WIN32
    
    if (fopen_s(&fp, filename.c_str(), "wb") != 0) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        throw std::bad_alloc();
    }

    #elif __linux__

    fp = fopen(filename.c_str(), "wb");

    if (fp == NULL) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        throw std::bad_alloc();
    }

    #endif

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        throw load_failure("Failed to dump frame for " + filename);
    }

    int width = w.get_width();
    int height = w.get_height();

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
    png_set_compression_level(png_ptr, Z_BEST_SPEED);

    png_write_info(png_ptr, info_ptr);

    GLubyte* pixels = new GLubyte[width * height * 4];

    if (setjmp(png_jmpbuf(png_ptr))) {
        delete[] pixels;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        throw load_failure("Failed to dump frame for " + filename);
    }

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    for (int i = height - 1; i >= 0; i--) {
        png_write_row(png_ptr, &pixels[i * width * 4]);
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    delete[] pixels;
}

} // gl
} // eldstar
