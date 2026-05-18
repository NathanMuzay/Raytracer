/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** ILight
*/

#ifndef ILIGHT_HPP_
#define ILIGHT_HPP_

#include <string>

#include "../Math/Math.hpp"

class ILight {
    public:
        virtual ~ILight() = default;

        virtual Math::Vector3D getLightDir(const Math::Vector3D &point) const = 0;
        virtual double getLightDist(const Math::Vector3D &point) const = 0;
        virtual Math::Vector3D computeLight(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const = 0;
        virtual Math::Vector3D computeSpecular(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const { (void)point; (void)normal; (void)viewDir; return Math::Vector3D(0, 0, 0); }

        virtual std::string getDebugInfo() const = 0;
};

#endif /* !ILIGHT_HPP_ */
