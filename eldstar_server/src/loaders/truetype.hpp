#pragma once

#include <map>
#include <string>
#include <sstream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "../external/unicode.hpp"
#include "../exceptions.hpp"
#include "../gl/primitives.hpp"
#include "file.hpp"


namespace eldstar {
namespace loaders {

std::vector<UChar32>* utf8_to_utf32(const std::string& utf8_text) {
    UErrorCode error = 0;

    size_t utf16_buffer_size = utf8_text.length() + 1;  // +1 for null terminator
    std::unique_ptr<UChar[]> utf16_text(new UChar[utf16_buffer_size]);

    int32_t utf16_length;
    u_strFromUTF8(utf16_text.get(), static_cast<int32_t>(utf16_buffer_size), &utf16_length, utf8_text.c_str(), static_cast<int32_t>(utf8_text.length()), &error);

    int32_t utf32_buffer_size = utf16_length + 1;  // +1 for null terminator
    std::vector<UChar32>* utf32_text = new std::vector<UChar32>(utf32_buffer_size);

    int32_t utf32_length;
    u_strToUTF32(utf32_text->data(), utf32_buffer_size, &utf32_length, utf16_text.get(), utf16_length, &error);

    utf32_text->resize(static_cast<size_t>(utf32_length));

    return utf32_text;
}

class text_manager {
    public:
        text_manager() {
            FT_Error code = 0;

            if (code = FT_Init_FreeType(&ft)) {
                throw load_failure("The FreeType library was not initialized successfully. (error code " + std::to_string(code) + ")");
            }

            if (code = FT_Stroker_New(ft, &st)) {
                throw load_failure("The FreeType library was loaded but could not initialize the stroke drawer. (error code " + std::to_string(code) + ")");
            }
        }

        ~text_manager() {
            FT_Stroker_Done(st);
            FT_Done_FreeType(ft);
        }

        FT_Library ft;
        FT_Stroker st;
};

class glyph {
    public:
        glyph(FT_Face face, UChar32 c, FT_Stroker st) : character(c) {
            FT_Error code = 0;
            glyph_index = FT_Get_Char_Index(face, c);

            // Load the glyph data and bitmap
            if (code = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) {
                throw load_failure("Could not load a FreeType glyph for rendering. (character " + std::to_string(c) + ", glyph index " + std::to_string(glyph_index) + ", error code " + std::to_string(code) + ")");
            }

            // Bind this glyph's texture
            gl::texture_binder binder;
            binder.bind(fill_bitmap);

            // Copy the FreeType bitmap to our GL texture.
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            // Store information to compensate for the single-channel texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Store render information for the fill
            fill_size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            fill_bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            advance = glm::ivec2(face->glyph->advance.x, face->glyph->advance.y);

            // Switch to rendering stroke
            binder.bind(stroke_bitmap);

            FT_Stroker_Set(st, 2 * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

            // Load the glyph fresh, in non-rendered mode
            if (code = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
                throw load_failure("Could not load a FreeType glyph for stroking. (character " + std::to_string(c) + ", glyph index " + std::to_string(glyph_index) + ", error code " + std::to_string(code) + ")");
            }

            FT_Glyph glyph;
            FT_Get_Glyph(face->glyph, &glyph);

            FT_Glyph_StrokeBorder(&glyph, st, false, true);
            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);

            FT_BitmapGlyph stroke_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

            // Copy the FreeType bitmap to our GL texture.
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                stroke_glyph->bitmap.width,
                stroke_glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                stroke_glyph->bitmap.buffer
            );

            // Store information to compensate for the single-channel texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stroke_size = glm::ivec2(stroke_glyph->bitmap.width, stroke_glyph->bitmap.rows);
            stroke_bearing = glm::ivec2(stroke_glyph->left, stroke_glyph->top);
        }

        UChar32 character;
        FT_UInt glyph_index;

        gl::texture fill_bitmap;
        glm::ivec2 fill_size;
        glm::ivec2 fill_bearing;

        gl::texture stroke_bitmap;
        glm::ivec2 stroke_size;
        glm::ivec2 stroke_bearing;

        glm::ivec2 advance;
};

class truetype {
    public:
        truetype(text_manager* man, const file& from, FT_UInt size) : man(man), size(size) {
            std::stringstream buffer;
            buffer << from.rdbuf();
            font_data = buffer.str();

            FT_Error code = 0;

            if (code = FT_New_Memory_Face(man->ft, reinterpret_cast<const FT_Byte*>(font_data.c_str()), static_cast<FT_Long>(font_data.length()), 0, &face)) {
                throw load_failure("Failed to load TrueType font from '" + from.filename + "' (error code " + std::to_string(code) + ")");
            }

            if (code = FT_Set_Pixel_Sizes(face, 0, size)) {
                throw load_failure("Failed to set the font size for '" + from.filename + "' (error code " + std::to_string(code) + ")");
            }

            gl::vertex_array_binder a_binder(vao);
            gl::buffer_binder b_binder(vbo);

            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        }

        ~truetype() {
            FT_Done_Face(face);
        }

        glyph* get_glyph(UChar32 c) {
            if (glyphs.count(c) == 0) {
                glyphs.insert(std::pair<UChar32, glyph>(
                    c, glyph(face, c, man->st)
                ));
            }

            return &glyphs.find(c)->second;
        }

        glyph* operator[](UChar32 c) {
            return get_glyph(c);
        }

        glm::vec2 render_utf8(const std::string& utf8_text, glm::vec2 position, glm::vec2 scale = glm::vec2(1.0f, 1.0f)) {
            std::unique_ptr<std::vector<UChar32>> utf32_text(utf8_to_utf32(utf8_text));

            glActiveTexture(GL_TEXTURE0);

            gl::vertex_array_binder a_binder(vao);
            gl::buffer_binder b_binder(vbo);
            gl::texture_binder t_binder;

            for (UChar32 c : *utf32_text) {
                glyph* g = get_glyph(c);

                glm::vec2 draw_position = position + (glm::vec2(g->fill_bearing.x, g->fill_bearing.y - g->fill_size.y) * scale);
                glm::vec2 draw_size = glm::vec2(g->fill_size.x, g->fill_size.y) * scale;

                float vertices[6][5] = {
                    {draw_position.x, draw_position.y + draw_size.y, 0.0f, 0.0f, 1.0f},
                    {draw_position.x, draw_position.y, 0.0f, 1.0f, 1.0f},
                    {draw_position.x + draw_size.x, draw_position.y, 1.0f, 1.0f, 1.0f},

                    {draw_position.x, draw_position.y + draw_size.y, 0.0f, 0.0f, 1.0f},
                    {draw_position.x + draw_size.x, draw_position.y, 1.0f, 1.0f, 1.0f},
                    {draw_position.x + draw_size.x, draw_position.y + draw_size.y, 1.0f, 0.0f, 1.0f}
                };

                t_binder.bind(g->fill_bitmap);

                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                position += glm::vec2(
                    scale.x * static_cast<float>(g->advance.x >> 6),
                    scale.y * static_cast<float>(g->advance.y >> 6)
                );
            }

            return position;
        }

        glm::vec2 render_utf8_bordered(const std::string& utf8_text, glm::vec2 position, glm::vec2 scale = glm::vec2(1.0f, 1.0f)) {
            std::unique_ptr<std::vector<UChar32>> utf32_text(utf8_to_utf32(utf8_text));

            glActiveTexture(GL_TEXTURE0);

            gl::vertex_array_binder a_binder(vao);
            gl::buffer_binder b_binder(vbo);
            gl::texture_binder t_binder;

            size_t glyph_count = utf32_text->size();
            std::vector<glyph*> glyphs(glyph_count);

            for (size_t i = 0; i < glyph_count; i++)
                glyphs[i] = get_glyph((*utf32_text)[i]);

            glm::vec2 stroke_needle(position);

            for (glyph* g : glyphs) {
                // Draw stroke
                glm::vec2 stroke_position = stroke_needle + (glm::vec2(g->stroke_bearing.x, g->stroke_bearing.y - g->stroke_size.y) * scale);
                glm::vec2 stroke_size = glm::vec2(g->stroke_size.x, g->stroke_size.y) * scale;

                float stroke_vertices[6][5] = {
                    {stroke_position.x, stroke_position.y + stroke_size.y, 0.0f, 0.0f, 0.0f},
                    {stroke_position.x, stroke_position.y, 0.0f, 1.0f, 0.0f},
                    {stroke_position.x + stroke_size.x, stroke_position.y, 1.0f, 1.0f, 0.0f},

                    {stroke_position.x, stroke_position.y + stroke_size.y, 0.0f, 0.0f, 0.0f},
                    {stroke_position.x + stroke_size.x, stroke_position.y, 1.0f, 1.0f, 0.0f},
                    {stroke_position.x + stroke_size.x, stroke_position.y + stroke_size.y, 1.0f, 0.0f, 0.0f}
                };

                t_binder.bind(g->stroke_bitmap);

                glBufferData(GL_ARRAY_BUFFER, sizeof(stroke_vertices), stroke_vertices, GL_DYNAMIC_DRAW);

                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                stroke_needle += glm::vec2(
                    scale.x * static_cast<float>(g->advance.x >> 6),
                    scale.y * static_cast<float>(g->advance.y >> 6)
                );
            }

            for (glyph* g : glyphs) {
                // Draw fill
                glm::vec2 fill_position = position + (glm::vec2(g->fill_bearing.x, g->fill_bearing.y - g->fill_size.y) * scale);
                glm::vec2 fill_size = glm::vec2(g->fill_size.x, g->fill_size.y) * scale;

                float fill_vertices[6][5] = {
                    {fill_position.x, fill_position.y + fill_size.y, 0.0f, 0.0f, 1.0f},
                    {fill_position.x, fill_position.y, 0.0f, 1.0f, 1.0f},
                    {fill_position.x + fill_size.x, fill_position.y, 1.0f, 1.0f, 1.0f},

                    {fill_position.x, fill_position.y + fill_size.y, 0.0f, 0.0f, 1.0f},
                    {fill_position.x + fill_size.x, fill_position.y, 1.0f, 1.0f, 1.0f},
                    {fill_position.x + fill_size.x, fill_position.y + fill_size.y, 1.0f, 0.0f, 1.0f}
                };

                t_binder.bind(g->fill_bitmap);

                glBufferData(GL_ARRAY_BUFFER, sizeof(fill_vertices), fill_vertices, GL_DYNAMIC_DRAW);

                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                position += glm::vec2(
                    scale.x * static_cast<float>(g->advance.x >> 6),
                    scale.y * static_cast<float>(g->advance.y >> 6)
                );
            }

            return position;
        }

        text_manager* man;

        FT_Face face;
        FT_UInt size;

        std::map<UChar32, glyph> glyphs;

    private:
        gl::vertex_array vao;
        gl::buffer vbo;

        std::string font_data;
};

} // loaders
} // eldstar
