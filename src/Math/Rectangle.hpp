/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Rectangle
*/

#ifndef RECTANGLE_HPP_
#define RECTANGLE_HPP_

#include "Math.hpp"

class Rectangle3D {
    public:
        Math::Point3D origin;
        Math::Vector3D bottom_side;
        Math::Vector3D left_side;

        Rectangle3D() = default;
        Rectangle3D(Math::Point3D origin, Math::Vector3D bottom_side, Math::Vector3D left_side) 
            : origin(origin), bottom_side(bottom_side), left_side(left_side) {}

        Math::Point3D pointAt(double u, double v) const { return origin + bottom_side * u + left_side * v; }
};

#endif /* !RECTANGLE_HPP_ */
