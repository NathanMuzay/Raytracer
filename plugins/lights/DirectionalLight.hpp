/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** DirectionalLight
*/

#ifndef DIRECTIONALLIGHT_HPP_
#define DIRECTIONALLIGHT_HPP_

#include <string>

#include "../../src/Interfaces/ILight.hpp"
#include "../../src/Math/Math.hpp"

class DirectionalLight : public ILight {
    public:
        DirectionalLight(const Math::Vector3D &direction, float intensity, const Math::Vector3D &color = Math::Vector3D(1.0f, 1.0f, 1.0f), double phong = 0.0)
            : _direction(direction.normalize()), _intensity(intensity), _color(color), _phong(phong) {}

        Math::Vector3D getLightDir(const Math::Vector3D &point) const override;
        double getLightDist(const Math::Vector3D &point) const override;
        Math::Vector3D computeLight(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const override;
        Math::Vector3D computeSpecular(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const override;

        std::string getDebugInfo() const override;

    private:
        Math::Vector3D _direction;
        float _intensity;
        Math::Vector3D _color;
        double _phong;
};

#endif /* !DIRECTIONALLIGHT_HPP_ */
