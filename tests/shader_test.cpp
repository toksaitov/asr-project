#include "asr.h"

#include <string>
#include <vector>

// You can get any other shaders from http://glslsandbox.com/ to try here.
static const std::string Fragment_Shader_Source{R"( // NOLINT(cert-err58-cpp)
/*
 * Original shader from: https://www.shadertoy.com/view/3tyfWG
 */

#ifdef GL_ES
precision mediump float;
#endif

// glslsandbox 
uniform float time;
uniform vec2 resolution;

// shadertoy emulation
#define iTime time
#define iResolution resolution

// --------[ Original ShaderToy begins here ]---------- //
////////////////////////////////////////////////////////////////////////////////
//
// yonatan's 3D fractal - A twitter user named yonatan/@zozuar posted a really
// compelling animation and GLSL-snippet of a 3D-fractal the other day. I had to
// get that into a fully working shader to see how much for a working shader was
// still missing. The character-limit on twitter is hard :)
//
// Furthermore some people in the comments on the original post were missing
// code-context and this my attempt to provide that. I provide a bit of
// commentary in case people a bit unfamiliar with raymarching give it a read.
//
// See the function yonatansFractal() for all the details and references.
//
// Author(s):
//   Mirco "MacSlow" Müller <macslow@gmail.com>
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 3, as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranties of
// MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

// basic 2D-rotation matrix
mat2 rotate2D (float rad)
{
    float c = cos (rad);
    float s = sin (rad);
    return mat2 (c, s, -s, c);
}

// convert hue/saturation/value to RGB-color
vec3 hsv(float h, float s, float v)
{
    vec3 c = vec3 (h, s, v);
    vec4 k = vec4(1., 2./3., 1./3., 3.);
    vec3 p = abs(fract(c.xxx + k.xyz) * 6. - k.www);

    return c.z * mix(k.xxx, clamp(p - k.xxx, .0, 1.), c.y);
}

void yonatansFractal (float t, vec2 FC, vec2 r, inout vec3 o)
{
    // yonatan/@zozuar's GLSL-snippet as posted on twitter...
    // just formatted a bit nicer, but otherwise untouched, see
    // https://twitter.com/zozuar/status/1367243732764876800
    // for reference
    float g=0., e, s, k = t*.1;
    for(float i=0.; i < 99.;++i) {
        vec3 p = vec3(g*(FC.xy - .5*r)/r.y + .5,g - 1.);
        p.xz *= rotate2D(k);
        s = 3.;
        for(int i=0; i < 9; ++i ) {
            s *= e = max(1.,(8.-8.*cos(k))/dot(p,p));
            p = vec3(2,4,2) - abs(abs(p)*e - vec3(4,4,2) );
        }
        g += min(length(p.xz), p.y)/s;
        s = log(s);
        o.rgb += hsv(s/15. + .5, .3, s/1e3);
    }
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float t = iTime + 20.;   // offset the time thus the thumbnail looks not empty
    vec2 FC = fragCoord;     // pixel-coordinates
    vec3 o = vec3 (.0);      // rgb-output color
    vec2 r = iResolution.xy; // resolution for point/ray-generation

    yonatansFractal(t, FC, r, o);

    // finally show it on screen
    fragColor = vec4(o, 1.);
}
// --------[ Original ShaderToy ends here ]---------- //

void main(void)
{
    mainImage(gl_FragColor, gl_FragCoord.xy);
}
)"};

static const std::vector<asr::Vertex> Quad_Geometry_Vertices = {
    //           Position           Normal            Color (RGBA)            Texture Coordinates (UV)
    asr::Vertex{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
    asr::Vertex{ 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    asr::Vertex{-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f},
    asr::Vertex{ 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f}
};
static const std::vector<unsigned int> Quad_Geometry_Indices = { 0, 1, 2, 1, 3, 2 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(400U, 400U, "Shader Test on ASR Version 1.3");

    auto material = create_material(
        R"(
            #version 110

            attribute vec4 position; void main() { gl_Position = position; }
        )",
        Fragment_Shader_Source
    );
    auto geometry = create_geometry(Triangles, Quad_Geometry_Vertices, Quad_Geometry_Indices);

    prepare_for_rendering();

    set_material_current(&material);

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

        set_geometry_current(&geometry);
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_geometry(geometry);
    destroy_material(material);

    destroy_window();

    return 0;
}
