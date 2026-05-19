#include "EmissiveLight.hpp"

#include <cmath>
#include <sstream>

Math::Vector3D EmissiveLight::getLightDir(const Math::Vector3D &point) const
{
    Math::Vector3D dir(
        _position.x - point.x,
        _position.y - point.y,
        _position.z - point.z
    );
    return dir.normalize();
}

double EmissiveLight::getLightDist(const Math::Vector3D &point) const
{
    Math::Vector3D diff(
        _position.x - point.x,
        _position.y - point.y,
        _position.z - point.z
    );
    return diff.length();
}

Math::Vector3D EmissiveLight::computeLight(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const
{
    (void)viewDir;
    Math::Vector3D lightDir = getLightDir(point);
    double cosAngle = normal.dot(lightDir);
    if (cosAngle < 0.0)
        cosAngle = 0.0;
    return _color * (_intensity * cosAngle);
}

Math::Vector3D EmissiveLight::computeSpecular(const Math::Vector3D &point, const Math::Vector3D &normal, const Math::Vector3D &viewDir) const
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

std::string EmissiveLight::getDebugInfo() const
{
    std::ostringstream oss;
    oss << "(pos: " << _position.x << "," << _position.y << "," << _position.z
        << " intensity: " << _intensity
        << " phong: " << _phong
        << " surfaceRadius: " << _surfaceRadius << ")";
    return oss.str();
}
