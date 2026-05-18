/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Core
*/

#ifndef CORE_HPP_
#define CORE_HPP_

#include <thread>
#include <atomic>

#include "../Factory/PrimitiveFactory.hpp"
#include "../Factory/MaterialFactory.hpp"
#include "../Factory/LightFactory.hpp"

#include "SceneBuilder.hpp"
#include "Renderer.hpp"
#include "../UI/Window.hpp"

class Core {
    public:
        Core(const std::string &configFile, bool display);
        ~Core();

    private:
        std::string _configFile;
        bool _display;

        PrimitiveFactory _primitiveFactory;
        MaterialFactory _materialFactory;
        LightFactory _lightFactory;

        Scene _scene;
        Renderer _renderer;
        std::unique_ptr<Window> _window;

        std::thread _renderThread;
        std::atomic<bool> _stopRender{false};

        void buildScene();
        void launchRender();
        void launchPreview();
        void stopRender();
        void moveCamera(double dx, double dy, double dz);
        void rotateCamera(double dyaw, double dpitch);
        void run();
};

#endif /* !CORE_HPP_ */
