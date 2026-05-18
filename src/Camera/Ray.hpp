/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Ray
*/

#ifndef RAY_HPP_
#define RAY_HPP_

#include "../Math/Math.hpp"

class Ray {
    public:
        Math::Point3D origin;
        Math::Vector3D direction;

        Ray() = default;
        Ray(Math::Point3D origin, Math::Vector3D direction) : origin(origin), direction(direction) {}
};

#endif /* !RAY_HPP_ */
