#pragma once

#include <fstream>
#include <string>

#include "../exceptions.hpp"


namespace eldstar {
namespace loaders {

class file {
    public:
        file(const std::string &filename) : filename(filename) {
            internal.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try {
                internal.open(filename, std::ios::in | std::ios::binary);
            } catch (std::ifstream::failure& e) {
                throw load_failure(
                    "Failed to open resource file '" + filename + "' (code " + std::to_string(e.code().value()) + "): " + std::string(e.what())
                );
            }
        };

        ~file() {
            internal.close();
        }

        std::filebuf* rdbuf() const {
            return internal.rdbuf();
        };

        std::istream& read(char *s, std::streamsize n) {
            return internal.read(s, n);
        };

        std::istream& seekg(std::streamoff off, std::ios_base::seekdir way) {
            return internal.seekg(off, way);
        }

        std::streampos tellg() {
            return internal.tellg();
        }

        std::ifstream internal;
        std::string filename;
};

} // loaders
} // eldstar
