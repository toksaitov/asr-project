#ifndef ASR_H
#define ASR_H

#include "objects/object.h"
#include "objects/mesh.h"
#include "objects/camera.h"
#include "lights/light.h"
#include "lights/ambient_light.h"
#include "lights/directional_light.h"
#include "lights/point_light.h"
#include "lights/spot_light.h"
#include "geometries/vertex.h"
#include "geometries/geometry.h"
#include "geometries/es2_geometry.h"
#include "geometries/geometry_generators.h"
#include "textures/texture.h"
#include "textures/es2_texture.h"
#include "materials/material.h"
#include "materials/constant_material.h"
#include "materials/es2_constant_material.h"
#include "materials/phong_material.h"
#include "materials/es2_phong_material.h"
#include "scene/scene.h"
#include "window/window.h"
#include "window/es2_sdl_window.h"
#include "renderer/shader.h"
#include "renderer/es2_shader.h"
#include "renderer/renderer.h"
#include "renderer/es2_renderer.h"
#include "math/ray.h"
#include "math/plane.h"
#include "math/aabb.h"
#include "math/sphere.h"
#include "utilities/utilities.h"

#include <imgui.h>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#endif
