#ifndef RENDERER_H
#define RENDERER_H

#include <utility>

#include "scene/scene.h"
#include "window/window.h"

namespace asr
{
    class Renderer
    {
    public:
        Renderer(std::shared_ptr<Scene> scene, std::shared_ptr<Window> window)
            : scene(std::move(scene)), window(std::move(window))
        {}

        virtual ~Renderer() = default;

        virtual void render() = 0;

    protected:
        std::shared_ptr<Scene> scene;
        std::shared_ptr<Window> window;
    };
}

#endif
