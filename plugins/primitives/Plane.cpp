/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Plane
*/

#include <libconfig.h++>
#include <sstream>
#include <cmath>
#include <memory>

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class Plane : public IPrimitive {
    public:
        Math::Vector3D _normal;
        Math::Vector3D _point;
        std::shared_ptr<IMaterial> _material;

        Plane(const libconfig::Setting &config)
        {
            std::string axis = (const char *)config["axis"];
            double position  = LibconfigHelper::getNumeric(config, "position");

            if (axis == "X") {
                _normal = Math::Vector3D(1, 0, 0);
                _point  = Math::Vector3D(position, 0, 0);
            } else if (axis == "Y") {
                _normal = Math::Vector3D(0, 1, 0);
                _point  = Math::Vector3D(0, position, 0);
            } else {
                _normal = Math::Vector3D(0, 0, 1);
                _point  = Math::Vector3D(0, 0, position);
            }
            _material = nullptr;
        }

        double getIntersection(const Ray &ray) const override
        {
            double denom = _normal.dot(ray.direction);
            if (std::abs(denom) < 1e-8)
                return -1;

            Math::Vector3D op(
                _point.x - ray.origin.x,
                _point.y - ray.origin.y,
                _point.z - ray.origin.z
            );
            double t = _normal.dot(op) / denom;
            return (t > 1e-4) ? t : -1.0;
        }

        bool hits(const Ray &ray) const override
        {
            return getIntersection(ray) > 0;
        }

        Math::Vector3D getNormal(const Math::Vector3D &point) const override
        {
            (void)point;
            return _normal;
        }

        AABB getAABB() const override
        {
            constexpr double INF = 1e9;
            constexpr double EPS = 1e-3;

            if (std::abs(_normal.x) > 0.9) {
                return AABB(
                    Math::Vector3D(_point.x - EPS, -INF, -INF),
                    Math::Vector3D(_point.x + EPS,  INF,  INF)
                );
            } else if (std::abs(_normal.y) > 0.9) {
                return AABB(
                    Math::Vector3D(-INF, _point.y - EPS, -INF),
                    Math::Vector3D( INF, _point.y + EPS,  INF)
                );
            } else {
                return AABB(
                    Math::Vector3D(-INF, -INF, _point.z - EPS),
                    Math::Vector3D( INF,  INF, _point.z + EPS)
                );
            }
        }

        void setMaterial(std::shared_ptr<IMaterial> material) override
        {
            _material = material;
        }

        std::shared_ptr<IMaterial> getMaterial() const override { return _material; }

        void applyTranslation(const Math::Vector3D &t) override
        {
            _point.x += t.x;
            _point.y += t.y;
            _point.z += t.z;
        }

        void applyRotation(const Math::Vector3D &angles) override
        {
            double rx = angles.x * M_PI / 180.0;
            double ry = angles.y * M_PI / 180.0;
            double rz = angles.z * M_PI / 180.0;

            if (rx != 0.0) {
                double ny = _normal.y * std::cos(rx) - _normal.z * std::sin(rx);
                double nz = _normal.y * std::sin(rx) + _normal.z * std::cos(rx);
                _normal.y = ny; _normal.z = nz;
                double py = _point.y * std::cos(rx) - _point.z * std::sin(rx);
                double pz = _point.y * std::sin(rx) + _point.z * std::cos(rx);
                _point.y = py; _point.z = pz;
            }
            if (ry != 0.0) {
                double nx = _normal.x * std::cos(ry) + _normal.z * std::sin(ry);
                double nz = -_normal.x * std::sin(ry) + _normal.z * std::cos(ry);
                _normal.x = nx; _normal.z = nz;
                double px = _point.x * std::cos(ry) + _point.z * std::sin(ry);
                double pz = -_point.x * std::sin(ry) + _point.z * std::cos(ry);
                _point.x = px; _point.z = pz;
            }
            if (rz != 0.0) {
                double nx = _normal.x * std::cos(rz) - _normal.y * std::sin(rz);
                double ny = _normal.x * std::sin(rz) + _normal.y * std::cos(rz);
                _normal.x = nx; _normal.y = ny;
                double px = _point.x * std::cos(rz) - _point.y * std::sin(rz);
                double py = _point.x * std::sin(rz) + _point.y * std::cos(rz);
                _point.x = px; _point.y = py;
            }
            _normal = _normal.normalize();
        }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "Plane(normal=" << _normal.x << "," << _normal.y << "," << _normal.z
                << " point=" << _point.x << "," << _point.y << "," << _point.z
                << " material=" << (_material ? _material->getDebugInfo() : "none")
                << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "planes";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new Plane(config);
}
