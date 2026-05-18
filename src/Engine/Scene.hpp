/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Scene
*/

#ifndef SCENE_HPP_
#define SCENE_HPP_

#include <vector>
#include <memory>

#include "../Camera/Camera.hpp"
#include "../Interfaces/IPrimitive.hpp"
#include "../Interfaces/ILight.hpp"
#include "../BVH/BVH.hpp"

struct AOSettings {
    bool enabled = false;
    int samples = 16;
    double radius = 50.0;
    double intensity = 0.8;
};

struct FogSettings {
    bool enabled = false;
    double density = 0.003;
    double start = 100.0;
    Math::Vector3D color = Math::Vector3D(10, 10, 10);
};

class Scene {
    public:
        bool fastMode = false;

        Camera camera;
        int width;
        int height;

        double ambientLight;
        double diffuseLight;
        Math::Vector3D backgroundColor = Math::Vector3D(0, 0, 0);

        AOSettings ao;
        FogSettings fog;

        int antialiasingSamples = 1;

        std::vector<std::shared_ptr<IPrimitive>> primitives;
        std::vector<std::shared_ptr<ILight>> lights;

        BVH bvh;

        void buildBVH()
        {
            bvh.build(primitives);
        }
};

#endif /* !SCENE_HPP_ */
