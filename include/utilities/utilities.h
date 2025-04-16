#ifndef UTILITIES_H
#define UTILITIES_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <tuple>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace asr::file_utilities
{
    typedef std::tuple<std::vector<uint8_t>, unsigned int, unsigned int, unsigned int> image_data_type;

    static std::string read_text_file(const std::string &path)
    {
        std::ifstream file_stream{path};
        if (!file_stream.is_open()) {
            std::cerr << "Failed to open the file: '" << path << "'" << std::endl;
            std::exit(-1);
        }

        std::stringstream string_stream;
        string_stream << file_stream.rdbuf();

        return string_stream.str();
    }

    static image_data_type read_image_file(const std::string &path)
    {
        int image_width, image_height;
        int bytes_per_pixel;

        auto image_data = static_cast<uint8_t *>(stbi_load(path.c_str(), &image_width, &image_height, &bytes_per_pixel, 0));
        if (!image_data) {
            std::cerr << "Failed to open the file: '" << path << "'" << std::endl;
            std::exit(-1);
        }
        if (!(bytes_per_pixel == 3 || bytes_per_pixel == 4)) {
            std::cerr << "Invalid image file format (only RGB and RGBA files are supported): '" << path << "'"
                      << std::endl;
            std::exit(-1);
        }

        std::vector<uint8_t> result{image_data, image_data + image_height * image_width * bytes_per_pixel};
        stbi_image_free(image_data);

        return std::make_tuple(result, image_width, image_height, bytes_per_pixel);
    }
}

#endif
