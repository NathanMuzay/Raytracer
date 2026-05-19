/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Renderer
*/

#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <thread>
#include <random>
#include <functional>
#include <atomic>
#include <mutex>

#include "Scene.hpp"

#include "../Exception.hpp"

#define BLUE   Math::Vector3D(0, 0, 255)
#define BLACK  Math::Vector3D(0, 0, 0)
#define WHITE  Math::Vector3D(255, 255, 255)
#define RED    Math::Vector3D(255, 0, 0)
#define GREEN  Math::Vector3D(0, 255, 0)
#define YELLOW Math::Vector3D(255, 255, 0)

class Renderer {
    public:
        Renderer() = default;
        ~Renderer() = default;

        void render(const Scene &scene, const std::string &output,
            std::atomic<bool> &stop,
            std::function<void(int, int, const Math::Vector3D&)> onPixel = nullptr);

    private:
        void write_color(std::ofstream &file, const Math::Vector3D &color);
        Math::Vector3D computeColor(const Ray &ray, const Scene &scene, int depth);
        Math::Vector3D clamp(const Math::Vector3D &color);
        static bool isInShadow(const Math::Vector3D &hitPoint, const Math::Vector3D &lightDir, double distToLight, const Scene &scene);
        double getShadowFactor(const Math::Vector3D &hitPoint, ILight *light, const Scene &scene);
        double computeAO(const Math::Vector3D &point, const Math::Vector3D &normal, const Scene &scene) const;
        Math::Vector3D computePixelColor(int x, int y, const Scene &scene, std::mt19937 &rng, std::uniform_real_distribution<double> &dist);
};

#endif /* !RENDERER_HPP_ */
