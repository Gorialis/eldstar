#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <png.h>
#include <zlib.h>

#include "file.hpp"


namespace eldstar {
namespace loaders {

void png_handle_read(png_structp png_ptr, png_bytep out, png_size_t length) {
    png_voidp io_ptr = png_get_io_ptr(png_ptr);

    if (io_ptr == NULL)
        throw load_failure("IO pointer missing in png handler");

    std::stringstream *ss = reinterpret_cast<std::stringstream*>(io_ptr);
    ss->read(reinterpret_cast<char*>(out), length);
}

class png {
    public:
        png(const file& from) {
            // Read data from file
            std::stringstream buffer;
            buffer << from.rdbuf();
            buffer.seekg(0, std::ios::beg);

            png_byte header[8];
            buffer.read(reinterpret_cast<char*>(header), 8);

            if (!png_check_sig(header, 8)) {
                throw load_failure("Failed to load PNG from '" + from.filename + "': bad header");
            }

            png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (png_ptr == NULL) {
                throw load_failure("Failed to make PNG reader struct from '" + from.filename + "' (out of memory?)");
            }

            png_infop info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr == NULL) {
                png_destroy_read_struct(&png_ptr, NULL, NULL);
                throw load_failure("Failed to make PNG info struct from '" + from.filename + "' (out of memory?)");
            }

            if (setjmp(png_jmpbuf(png_ptr))) {
                png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                throw load_failure("An unknown error occured while parsing a PNG from '" + from.filename + "'");
            }

            png_set_read_fn(png_ptr, &buffer, png_handle_read);
            png_set_sig_bytes(png_ptr, 8);
            png_read_info(png_ptr, info_ptr);

            png_get_IHDR(png_ptr, info_ptr,
                &width,
                &height,
                &bit_depth,
                &color_type,
                &interlace_type,
                &compression_type,
                &filter_method
            );

            size_t channel_size;

            switch (color_type) {
                case PNG_COLOR_TYPE_RGB_ALPHA:
                    channel_size = 4;
                    break;
                case PNG_COLOR_TYPE_RGB:
                    channel_size = 3;
                    break;
                case PNG_COLOR_TYPE_GRAY_ALPHA:
                    channel_size = 2;
                    break;
                case PNG_COLOR_TYPE_GRAY:
                default:
                    channel_size = 1;
                    break;
            }

            size_t row_width = channel_size * width;
            data.resize(row_width * height);

            for (size_t row = 0; row < height; row++) {
                png_read_row(png_ptr, &data[row_width * row], NULL);
            }

            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        };

        const unsigned char* get() const {
            return reinterpret_cast<const unsigned char*>(data.data());
        }

        png_uint_32 width;
        png_uint_32 height;
        int bit_depth;
        int color_type;
        int filter_method;
        int compression_type;
        int interlace_type;

        std::vector<png_byte> data;
};

} // loaders
} // eldstar
