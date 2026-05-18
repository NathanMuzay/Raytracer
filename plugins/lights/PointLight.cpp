/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** PointLight
*/

#include <sstream>
#include <cmath>

#include "PointLight.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

namespace libconfig {
    class Setting;
}

Math::Vector3D PointLight::getLightDir(const Math::Vector3D &point) const
{
    Math::Vector3D dir(
        _position.x - point.x,
        _position.y - point.y,
        _position.z - point.z
    );
    return dir.normalize();
}
 
double PointLight::getLightDist(const Math::Vector3D &point) const
{
    Math::Vector3D diff(
        _position.x - point.x,
        _position.y - point.y,
        _position.z - point.z
    );
    return diff.length();
}

Math::Vector3D PointLight::computeLight(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const
{
    (void)viewDir;
    Math::Vector3D lightDir = getLightDir(point);
    double cosAngle = normal.dot(lightDir);
    if (cosAngle < 0.0)
        cosAngle = 0.0;
    return _color * (_intensity * cosAngle);
}

Math::Vector3D PointLight::computeSpecular(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const
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

    // viewDir va caméra → hit, on veut hit → caméra
    Math::Vector3D toCamera = viewDir * -1.0;
    double rdotv = reflected.dot(toCamera);
    if (rdotv <= 0.0)
        return Math::Vector3D(0, 0, 0);

    double spec = std::pow(rdotv, _phong) * _intensity;
    return Math::Vector3D(spec, spec, spec);
}

std::string PointLight::getDebugInfo() const
{
    std::ostringstream oss;
    oss << "(pos: " << _position.x << "," << _position.y << "," << _position.z
        << " intensity: " << _intensity
        << " phong: " << _phong << ")";
    return oss.str();
}

extern "C" const char *getSectionName()
{
    return "point";
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

    return new PointLight(Math::Vector3D(x, y, z), 1.0, Math::Vector3D(r, g, b), phong);
}
