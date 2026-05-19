/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Core
*/

#include "Core.hpp"

namespace {
    const std::string OUTPUT_FILE = "output.ppm";
    constexpr double MOVE_SPEED = 10.0;
}

Core::Core(const std::string &configFile, bool display) : _configFile(configFile), _display(display)
{
    buildScene();
    if (_display){
        _window = std::make_unique<Window>(_scene.width, _scene.height, "Raytracer - " + configFile);
        launchRender();
        run();
    } else {
        _renderer.render(_scene, OUTPUT_FILE, _stopRender);
    }
}

Core::~Core()
{
    stopRender();
}

void Core::buildScene()
{
    SceneBuilder builder(_configFile);
    _scene = builder
        .buildCamera()
        .buildLights(_lightFactory)
        .buildPrimitives(_primitiveFactory, _materialFactory)
        .build();
}

void Core::launchRender()
{
    _stopRender = false;
    _renderThread = std::thread([this]() {
        launchPreview();
        if (!_stopRender){
            _renderer.render(_scene, OUTPUT_FILE, _stopRender,
                [this](int x, int y, const Math::Vector3D &color) {
                    _window->setPixel(x, y, color);
                }
            );
        }
    });
}

void Core::launchPreview()
{
    Scene preview;
    preview.camera           = _scene.camera;
    preview.width            = _scene.width  / 8;
    preview.height           = _scene.height / 8;
    preview.ambientLight     = _scene.ambientLight;
    preview.diffuseLight     = _scene.diffuseLight;
    preview.backgroundColor  = _scene.backgroundColor;
    preview.ao               = _scene.ao;
    preview.fog              = _scene.fog;
    preview.antialiasingSamples = _scene.antialiasingSamples;
    preview.primitives       = _scene.primitives;
    preview.lights           = _scene.lights;
    preview.fastMode         = true;
    preview.ao.enabled       = false;
    preview.fog.enabled      = false;
    preview.bvh              = _scene.bvh;

    std::atomic<bool> dummy{false};
    _renderer.render(preview, "", dummy,
        [this](int x, int y, const Math::Vector3D &color) {
            for (int dy = 0; dy < 8; dy++)
                for (int dx = 0; dx < 8; dx++)
                    _window->setPixel(x * 8 + dx, y * 8 + dy, color);
        }
    );
}

void Core::stopRender()
{
    _stopRender = true;
    if (_renderThread.joinable())
        _renderThread.join();
}

void Core::moveCamera(double dx, double dy, double dz)
{
    stopRender();
    _scene.camera.translate(dx, dy, dz);
    std::cout << "[Core] Camera moved to ("
              << _scene.camera.origin.x << ", "
              << _scene.camera.origin.y << ", "
              << _scene.camera.origin.z << ")" << std::endl;
    launchRender();
}

void Core::rotateCamera(double dyaw, double dpitch)
{
    stopRender();
    _scene.camera.rotate(dyaw, dpitch);
    std::cout << "[Core] Camera rotated (yaw=" << dyaw << ", pitch=" << dpitch << ")" << std::endl;
    launchRender();
}

void Core::run()
{
    while (_window->isOpen()) {
        Window::Event event = _window->pollEvents();
        switch (event){
            case Window::Event::Close:
                _window->close();
                break;
            case Window::Event::Reload:
                std::cout << "[Core] Scene reloaded" << std::endl;
                stopRender();
                buildScene();
                launchRender();
                break;
            case Window::Event::MoveForward:  moveCamera( 0,  MOVE_SPEED, 0); break;
            case Window::Event::MoveBack:     moveCamera( 0, -MOVE_SPEED, 0); break;
            case Window::Event::MoveLeft:     moveCamera(-MOVE_SPEED, 0, 0); break;
            case Window::Event::MoveRight:    moveCamera( MOVE_SPEED, 0, 0); break;
            case Window::Event::MoveUp:       moveCamera(0, 0,  MOVE_SPEED); break;
            case Window::Event::MoveDown:     moveCamera(0, 0, -MOVE_SPEED); break;
            case Window::Event::RotateLeft:  rotateCamera(5.0, 0.0); break;
            case Window::Event::RotateRight: rotateCamera(-5.0, 0.0); break;
            case Window::Event::RotateUp:    rotateCamera(0.0,  5.0); break;
            case Window::Event::RotateDown:  rotateCamera(0.0, -5.0); break;
            default: break;
        }
        _window->display();
    }
    stopRender();
}
