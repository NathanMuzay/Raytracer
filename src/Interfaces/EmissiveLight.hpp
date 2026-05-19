/*
** EPITECH PROJECT, 2026
** EmissiveLight
*/

#ifndef EMISSIVELIGHT_HPP_
#define EMISSIVELIGHT_HPP_

#include "ILight.hpp"

class EmissiveLight : public ILight {
    public:
        EmissiveLight(const Math::Vector3D &position, float intensity, const Math::Vector3D &color = Math::Vector3D(1.0f,1.0f,1.0f), double phong = 0.0, double surfaceRadius = 0.0)
            : _position(position), _intensity(intensity), _color(color), _phong(phong), _surfaceRadius(surfaceRadius) {}

        Math::Vector3D getLightDir(const Math::Vector3D &point) const override;
        double getLightDist(const Math::Vector3D &point) const override;
        Math::Vector3D computeLight(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const override;
        Math::Vector3D computeSpecular(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const override;

        std::string getDebugInfo() const override;

        double getSurfaceRadius() const { return _surfaceRadius; }
        const Math::Vector3D& getPosition() const { return _position; }
        const Math::Vector3D& getColor() const { return _color; }
        float getIntensity() const { return _intensity; }
        double getPhong() const { return _phong; }

    private:
        Math::Vector3D _position;
        float _intensity;
        Math::Vector3D _color;
        double _phong;
        double _surfaceRadius;
};

#endif /* !EMISSIVELIGHT_HPP_ */
