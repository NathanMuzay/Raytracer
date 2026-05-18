/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** PointLight
*/

#ifndef POINTLIGHT_HPP_
#define POINTLIGHT_HPP_

#include <string>

#include "../../src/Interfaces/ILight.hpp"
#include "../../src/Math/Math.hpp"

class PointLight : public ILight {
    public:
        PointLight(const Math::Vector3D &position, float intensity, const Math::Vector3D &color = Math::Vector3D(1.0f, 1.0f, 1.0f), double phong = 0.0)
            : _position(position), _intensity(intensity), _color(color), _phong(phong) {}
    
        Math::Vector3D getLightDir(const Math::Vector3D &point) const override;
        double getLightDist(const Math::Vector3D &point) const override;
        Math::Vector3D computeLight(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const override;
        Math::Vector3D computeSpecular(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const override;

        std::string getDebugInfo() const override;

    private:
        Math::Vector3D _position;
        float _intensity;
        Math::Vector3D _color;
        double _phong;
};

#endif /* !POINTLIGHT_HPP_ */
