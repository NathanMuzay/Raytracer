/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** SceneBuilder
*/

#ifndef SCENEBUILDER_HPP_
#define SCENEBUILDER_HPP_

#include "../Parser/LibconfigHelper.hpp"
#include "Scene.hpp"
#include "../Parser/Parser.hpp"
#include "../Camera/Camera.hpp"
#include "../Factory/PrimitiveFactory.hpp"
#include "../Factory/MaterialFactory.hpp"
#include "../Factory/LightFactory.hpp"

class SceneBuilder {
    public:
        SceneBuilder(const std::string &configFile) : _parser(configFile) {};
        ~SceneBuilder() = default;

        SceneBuilder &buildCamera()
        {
            const libconfig::Setting &cam = _parser.getCamera();

            double x = LibconfigHelper::getNumeric(cam["position"], "x");
            double y = LibconfigHelper::getNumeric(cam["position"], "y");
            double z = LibconfigHelper::getNumeric(cam["position"], "z");
            double fov = LibconfigHelper::getNumeric(cam, "fieldOfView");
            int width  = LibconfigHelper::getInt(cam["resolution"], "width");
            int height = LibconfigHelper::getInt(cam["resolution"], "height");

            if (fov == 0.0)
                fov = 90.0;

            double aspectRatio = (double)width / height;
            double halfHeight  = std::tan(fov * M_PI / 360.0);
            double halfWidth   = halfHeight * aspectRatio;

            Rectangle3D screen(
                Math::Point3D(x - halfWidth, y + 1.0, z - halfHeight),
                Math::Vector3D(2 * halfWidth, 0, 0),
                Math::Vector3D(0, 0, 2 * halfHeight)
            );
            _scene.camera = Camera(Math::Point3D(x, y, z), screen);
            _scene.width = width;
            _scene.height = height;

            if (_parser.getCamera().exists("fog")) {
                const libconfig::Setting &fog = _parser.getCamera()["fog"];
                _scene.fog.enabled = true;
                if (fog.exists("density"))
                    _scene.fog.density = LibconfigHelper::getNumeric(fog, "density");
                if (fog.exists("start"))
                    _scene.fog.start = LibconfigHelper::getNumeric(fog, "start");
                if (fog.exists("color")) {
                    _scene.fog.color.x = LibconfigHelper::getNumeric(fog["color"], "r");
                    _scene.fog.color.y = LibconfigHelper::getNumeric(fog["color"], "g");
                    _scene.fog.color.z = LibconfigHelper::getNumeric(fog["color"], "b");
                }
            }

            if (_parser.getCamera().exists("antialiasing")) {
                _scene.antialiasingSamples = static_cast<int>(LibconfigHelper::getNumeric(_parser.getCamera(), "antialiasing"));
            }

            return *this;
        }

        SceneBuilder &buildPrimitives(PrimitiveFactory &pFactory, MaterialFactory &mFactory)
        {
            _scene.primitives = pFactory.loadPlugins("plugins/primitives", _parser.getPrimitives(), mFactory, "plugins/materials", &_scene.lights);
            return *this;
        }

        SceneBuilder &buildLights(LightFactory &factory)
        {
            const libconfig::Setting &lights = _parser.getLights();
            _scene.ambientLight = LibconfigHelper::getNumeric(lights, "ambient");
            _scene.diffuseLight = LibconfigHelper::getNumeric(lights, "diffuse");

            if (lights.exists("ao")) {
                const libconfig::Setting &ao = lights["ao"];
                _scene.ao.enabled = true;
                if (ao.exists("samples"))
                    _scene.ao.samples = static_cast<int>(LibconfigHelper::getNumeric(ao, "samples"));
                if (ao.exists("radius"))
                    _scene.ao.radius = LibconfigHelper::getNumeric(ao, "radius");
                if (ao.exists("intensity"))
                    _scene.ao.intensity = LibconfigHelper::getNumeric(ao, "intensity");
            }

            auto newLights = factory.loadPlugins("plugins/lights", _parser.getLights());
            _scene.lights.insert(_scene.lights.end(), newLights.begin(), newLights.end());
            return *this;
        }

        Scene build()
        {
            if (_built)
                throw std::runtime_error("SceneBuilder::build() called more than once");
            _built = true;
            const libconfig::Setting &root = _parser.getConfig().getRoot();
            if (root.exists("background")) {
                const libconfig::Setting &background = root["background"];
                _scene.backgroundColor = Math::Vector3D(
                    LibconfigHelper::getNumeric(background, "r"),
                    LibconfigHelper::getNumeric(background, "g"),
                    LibconfigHelper::getNumeric(background, "b")
                );
            }
            _scene.buildBVH();

            std::cout << "[Scene] Camera: origin=" << _scene.camera.origin.x << ","
                      << _scene.camera.origin.y << "," << _scene.camera.origin.z
                      << " resolution=" << _scene.width << "x" << _scene.height << std::endl;
            std::cout << "[Scene] Light multipliers: ambient=" << _scene.ambientLight
                      << " diffuse=" << _scene.diffuseLight << std::endl;
            std::cout << "[Scene] Background: rgb(" << _scene.backgroundColor.x << ", "
                      << _scene.backgroundColor.y << ", " << _scene.backgroundColor.z << ")" << std::endl;
            std::cout << "[Scene] Ambient Occlusion: " << (_scene.ao.enabled
                      ? "enabled (samples=" + std::to_string(_scene.ao.samples)
                        + " radius=" + std::to_string((int)_scene.ao.radius)
                        + " intensity=" + std::to_string(_scene.ao.intensity) + ")"
                      : "disabled") << std::endl;
            std::cout << "[Scene] Antialiasing: " << (_scene.antialiasingSamples > 1
                      ? std::to_string(_scene.antialiasingSamples * _scene.antialiasingSamples) + " samples per pixel"
                      : "disabled") << std::endl;
            std::cout << "[Scene] Fog: " << (_scene.fog.enabled
                      ? "enabled (density=" + std::to_string(_scene.fog.density) + ")"
                      : "disabled") << std::endl;
            std::cout << "[Scene] " << _scene.primitives.size() << " primitive(s) loaded" << std::endl;
            std::cout << "[Scene] " << _scene.lights.size() << " light(s) loaded" << std::endl;
            std::cout << "[Scene] BVH built" << std::endl;
            return std::move(_scene);
        }

    private:
        Parser _parser;
        Scene _scene;
        bool _built = false;
};

#endif /* !SCENEBUILDER_HPP_ */
