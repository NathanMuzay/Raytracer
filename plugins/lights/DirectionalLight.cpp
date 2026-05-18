/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** DirectionalLight
*/

#include <sstream>
#include <cmath>

#include "DirectionalLight.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

namespace libconfig {
    class Setting;
}

Math::Vector3D DirectionalLight::getLightDir(const Math::Vector3D &point) const
{
    (void)point;
    return _direction.normalize();
}
 
double DirectionalLight::getLightDist(const Math::Vector3D &point) const
{
    (void)point;
    return std::numeric_limits<double>::max();
}
 
Math::Vector3D DirectionalLight::computeLight(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const
{
    (void)point;
    (void)viewDir;
    Math::Vector3D lightDir = getLightDir(point);
    double cosAngle = normal.dot(lightDir);
    if (cosAngle < 0.0)
        cosAngle = 0.0;
    return _color * (_intensity * cosAngle);
}

Math::Vector3D DirectionalLight::computeSpecular(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const
{
    if (_phong <= 0.0)
        return Math::Vector3D(0, 0, 0);

    Math::Vector3D lightDir = getLightDir(point);
    double diffuse = normal.dot(lightDir);
    if (diffuse <= 0.0)
        return Math::Vector3D(0, 0, 0);

    Math::Vector3D reflected(
        2.0 * diffuse * normal.x - lightDir.x,
        2.0 * diffuse * normal.y - lightDir.y,
        2.0 * diffuse * normal.z - lightDir.z
    );
    reflected = reflected.normalize();

    Math::Vector3D toCamera = viewDir * -1.0;
    double rdotv = reflected.dot(toCamera);
    if (rdotv <= 0.0)
        return Math::Vector3D(0, 0, 0);

    double spec = std::pow(rdotv, _phong) * _intensity;
    return Math::Vector3D(spec, spec, spec);
}
 
std::string DirectionalLight::getDebugInfo() const
{
    std::ostringstream oss;
    oss << "(dir: " << _direction.x << "," << _direction.y << "," << _direction.z
        << " intensity: " << _intensity
        << " phong: " << _phong << ")";
    return oss.str();
}

extern "C" const char *getSectionName()
{
    return "directional";
}

extern "C" ILight *createPlugin(const libconfig::Setting &config)
{
    double x = LibconfigHelper::getNumeric(config, "x");
    double y = LibconfigHelper::getNumeric(config, "y");
    double z = LibconfigHelper::getNumeric(config, "z");

    double r = 1.0, g = 1.0, b = 1.0;
    if (config.exists("color")) {
        r = LibconfigHelper::getNumeric(config["color"], "r") / 255.0;
        g = LibconfigHelper::getNumeric(config["color"], "g") / 255.0;
        b = LibconfigHelper::getNumeric(config["color"], "b") / 255.0;
    }

    double phong = 0.0;
    if (config.exists("phong"))
        phong = LibconfigHelper::getNumeric(config, "phong");

    return new DirectionalLight(Math::Vector3D(x, y, z), 1.0, Math::Vector3D(r, g, b), phong);
}
