#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>

namespace asr
{
    class Sphere
    {
    public:
        Sphere(const glm::vec3 &center, float radius)
            : _center(center), _radius(radius)
        {}

        [[nodiscard]] const glm::vec3 &get_center() const
        {
            return _center;
        }

        [[nodiscard]] float get_radius() const
        {
            return _radius;
        }

        void set_center(const glm::vec3 &center)
        {
            _center = center;
        }

        void set_radius(float radius)
        {
            _radius = radius;
        }

        void transform(glm::mat4 transformation_matrix)
        {
            _center = glm::vec3(transformation_matrix * glm::vec4(_center, 1.0f));

            float squared_scale_x =
                transformation_matrix[0][0] * transformation_matrix[0][0] +
                transformation_matrix[0][1] * transformation_matrix[0][1] +
                transformation_matrix[0][2] * transformation_matrix[0][2];

            float squared_scale_y =
                transformation_matrix[1][0] * transformation_matrix[1][0] +
                transformation_matrix[1][1] * transformation_matrix[1][1] +
                transformation_matrix[1][2] * transformation_matrix[1][2];

            float squared_scale_z =
                transformation_matrix[2][0] * transformation_matrix[2][0] +
                transformation_matrix[2][1] * transformation_matrix[2][1] +
                transformation_matrix[2][2] * transformation_matrix[2][2];

            float scale = sqrtf(fmaxf(squared_scale_x, fmaxf(squared_scale_y, squared_scale_z)));
            _radius *= scale;
        }

        [[nodiscard]] bool intersects_with_sphere(const Sphere& sphere) const {
			float difference_between_x = _center.x - sphere.get_center().x;
			float difference_between_y = _center.y - sphere.get_center().y;
			float difference_between_z = _center.z - sphere.get_center().z;
			float difference_between_centers =	difference_between_x * difference_between_x +
												difference_between_y * difference_between_y  +
												difference_between_z * difference_between_z;
			return difference_between_centers < (_radius + sphere.get_radius()) * (_radius + sphere.get_radius());

		}

    private:
        glm::vec3 _center;
        float _radius;
    };
}

#endif
