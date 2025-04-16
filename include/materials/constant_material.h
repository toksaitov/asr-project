#ifndef CONSTANT_MATERIAL_H
#define CONSTANT_MATERIAL_H

#include "materials/material.h"
#include "textures/texture.h"

#include <glm/glm.hpp>

namespace asr
{
    class ConstantMaterial : public Material
    {
    public:
        [[nodiscard]] const glm::vec4 &get_emission_color() const
        {
            return _emission_color;
        }

        void set_emission_color(const glm::vec4 &emission_color)
        {
            _emission_color = emission_color;
        }

        [[nodiscard]] const std::shared_ptr<Texture> &get_texture_1() const
        {
            return _texture1;
        }

        void set_texture_1(const std::shared_ptr<Texture> &texture_1)
        {
            _texture1 = texture_1;
        }

        [[nodiscard]] const std::shared_ptr<Texture> &get_texture_2() const
        {
            return _texture2;
        }

        void set_texture_2(const std::shared_ptr<Texture> &texture_2)
        {
            _texture2 = texture_2;
        }

    protected:
        glm::vec4 _emission_color{1.0f};

        std::shared_ptr<Texture> _texture1;
        std::shared_ptr<Texture> _texture2;
    };
}

#endif
