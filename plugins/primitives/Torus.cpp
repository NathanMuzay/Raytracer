/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Torus primitive plugin - FINAL VERSION
*/

#include <algorithm>
#include <cmath>
#include <libconfig.h++>
#include <limits>
#include <memory>
#include <sstream>

#include "../../src/BVH/AABB.hpp"
#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Math/Math.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Torus : public IPrimitive {
public:
  Math::Point3D _center;
  double _R; // Rayon majeur
  double _r; // Rayon mineur
  Math::Vector3D _rotation;
  std::shared_ptr<IMaterial> _material;

  Torus(const libconfig::Setting &config) {
    _center.x = LibconfigHelper::getNumeric(config, "x");
    _center.y = LibconfigHelper::getNumeric(config, "y");
    _center.z = LibconfigHelper::getNumeric(config, "z");
    _R = LibconfigHelper::getNumeric(config, "R");
    _r = LibconfigHelper::getNumeric(config, "r");
    _rotation =
        Math::Vector3D(LibconfigHelper::getNumeric(config["rotation"], "x"),
                       LibconfigHelper::getNumeric(config["rotation"], "y"),
                       LibconfigHelper::getNumeric(config["rotation"], "z"));
    _material = nullptr;
  }

  double getIntersection(const Ray &ray) const override {
    const double EPS = 1e-4;
    const double MAX_DIST = 1000.0;
    const int MAX_ITER = 128;

    // Ray Marching (Sphere Tracing)
    double t = 0.0;
    for (int i = 0; i < MAX_ITER; ++i) {
      Math::Point3D p = ray.origin + (ray.direction * t);
      Math::Point3D localP = inverseRotateVector(p - _center).toPoint();

      double dist = distanceToTorus(localP);
      if (dist < EPS)
        return t;
      t += dist;
      if (t > MAX_DIST)
        break;
    }

    return -1.0;
  }

  bool hits(const Ray &ray) const override {
    return getIntersection(ray) > 0;
  }

  Math::Vector3D getNormal(const Math::Vector3D &point) const override {
    Math::Point3D localP = inverseRotateVector(point.toPoint() - _center).toPoint();
    
    double xzLen = std::sqrt(localP.x * localP.x + localP.z * localP.z);
    Math::Vector3D localNormal;
    if (xzLen < 1e-6) {
        localNormal = Math::Vector3D(0, localP.y > 0 ? 1 : -1, 0);
    } else {
        Math::Point3D pOnCircle(localP.x / xzLen * _R, 0, localP.z / xzLen * _R);
        localNormal = (localP - pOnCircle).normalize();
    }
    
    return rotateVector(localNormal);
  }

  AABB getAABB() const override {
    double halfSize = _R + _r;
    return AABB(Math::Vector3D(_center.x - halfSize, _center.y - halfSize,
                               _center.z - halfSize),
                Math::Vector3D(_center.x + halfSize, _center.y + halfSize,
                               _center.z + halfSize));
  }

  void setMaterial(std::shared_ptr<IMaterial> material) override {
    _material = material;
  }

  std::shared_ptr<IMaterial> getMaterial() const override { return _material; }

  void applyTranslation(const Math::Vector3D &translation) override {
    _center.x += translation.x;
    _center.y += translation.y;
    _center.z += translation.z;
  }

  void applyRotation(const Math::Vector3D &rotation) override {
    _rotation += rotation;
  }

  std::string getDebugInfo() const override {
    std::ostringstream oss;
    oss << "Torus(center=(" << _center.x << "," << _center.y << "," << _center.z
        << "), R=" << _R << ", r=" << _r << ", rotation=(" << _rotation.x << ","
        << _rotation.y << "," << _rotation.z
        << "), material=" << (_material ? _material->getDebugInfo() : "none")
        << ")";
    return oss.str();
  }

private:
  Math::Vector3D rotateVector(const Math::Vector3D &v) const {
    double rx = _rotation.x * M_PI / 180.0;
    double ry = _rotation.y * M_PI / 180.0;
    double rz = _rotation.z * M_PI / 180.0;

    auto res = v;
    // X axis
    double y = res.y * std::cos(rx) - res.z * std::sin(rx);
    double z = res.y * std::sin(rx) + res.z * std::cos(rx);
    res.y = y; res.z = z;
    // Y axis
    double x = res.x * std::cos(ry) + res.z * std::sin(ry);
    z = -res.x * std::sin(ry) + res.z * std::cos(ry);
    res.x = x; res.z = z;
    // Z axis
    x = res.x * std::cos(rz) - res.y * std::sin(rz);
    y = res.x * std::sin(rz) + res.y * std::cos(rz);
    res.x = x; res.y = y;

    return res;
  }

  Math::Vector3D inverseRotateVector(const Math::Vector3D &v) const {
    double rx = -_rotation.x * M_PI / 180.0;
    double ry = -_rotation.y * M_PI / 180.0;
    double rz = -_rotation.z * M_PI / 180.0;

    auto res = v;
    // Z axis
    double x = res.x * std::cos(rz) - res.y * std::sin(rz);
    double y = res.x * std::sin(rz) + res.y * std::cos(rz);
    res.x = x; res.y = y;
    // Y axis
    x = res.x * std::cos(ry) + res.z * std::sin(ry);
    double z = -res.x * std::sin(ry) + res.z * std::cos(ry);
    res.x = x; res.z = z;
    // X axis
    y = res.y * std::cos(rx) - res.z * std::sin(rx);
    z = res.y * std::sin(rx) + res.z * std::cos(rx);
    res.y = y; res.z = z;

    return res;
  }

  double distanceToTorus(const Math::Point3D &p) const {
    double qx = std::sqrt(p.x * p.x + p.z * p.z) - _R;
    return std::sqrt(qx * qx + p.y * p.y) - _r;
  }
};

extern "C" const char *getSectionName() { return "torus"; }

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config) {
  return new Torus(config);
}
