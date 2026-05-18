/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Cube
*/

#include <libconfig.h++>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <memory>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class Cube : public IPrimitive {
    public:
        Math::Point3D _center;
        Math::Vector3D _size;
        std::shared_ptr<IMaterial> _material;
        Math::Vector3D _rotation;

        Cube(const libconfig::Setting &config)
        {
            double x = LibconfigHelper::getNumeric(config, "x");
            double y = LibconfigHelper::getNumeric(config, "y");
            double z = LibconfigHelper::getNumeric(config, "z");
            double sizeX = LibconfigHelper::getNumeric(config, "sizeX");
            double sizeY = LibconfigHelper::getNumeric(config, "sizeY");
            double sizeZ = LibconfigHelper::getNumeric(config, "sizeZ");

            _center = Math::Point3D(x, y, z);
            _size = Math::Vector3D(sizeX, sizeY, sizeZ);
            _material = nullptr;
            _rotation = Math::Vector3D(0, 0, 0);
        }

        Math::Vector3D rotateVector(const Math::Vector3D &v) const
        {
            double rx = _rotation.x * M_PI / 180.0;
            double ry = _rotation.y * M_PI / 180.0;
            double rz = _rotation.z * M_PI / 180.0;

            // Rotation around X axis
            double cosRx = std::cos(rx);
            double sinRx = std::sin(rx);
            Math::Vector3D v1(v.x, v.y * cosRx - v.z * sinRx, v.y * sinRx + v.z * cosRx);

            // Rotation around Y axis
            double cosRy = std::cos(ry);
            double sinRy = std::sin(ry);
            Math::Vector3D v2(v1.x * cosRy + v1.z * sinRy, v1.y, -v1.x * sinRy + v1.z * cosRy);

            // Rotation around Z axis
            double cosRz = std::cos(rz);
            double sinRz = std::sin(rz);
            Math::Vector3D v3(v2.x * cosRz - v2.y * sinRz, v2.x * sinRz + v2.y * cosRz, v2.z);

            return v3;
        }

        Math::Vector3D inverseRotateVector(const Math::Vector3D &v) const
        {
            double rx = -_rotation.x * M_PI / 180.0;
            double ry = -_rotation.y * M_PI / 180.0;
            double rz = -_rotation.z * M_PI / 180.0;

            // Rotation around Z axis (negative)
            double cosRz = std::cos(rz);
            double sinRz = std::sin(rz);
            Math::Vector3D v1(v.x * cosRz - v.y * sinRz, v.x * sinRz + v.y * cosRz, v.z);

            // Rotation around Y axis (negative)
            double cosRy = std::cos(ry);
            double sinRy = std::sin(ry);
            Math::Vector3D v2(v1.x * cosRy + v1.z * sinRy, v1.y, -v1.x * sinRy + v1.z * cosRy);

            // Rotation around X axis (negative)
            double cosRx = std::cos(rx);
            double sinRx = std::sin(rx);
            Math::Vector3D v3(v2.x, v2.y * cosRx - v2.z * sinRx, v2.y * sinRx + v2.z * cosRx);

            return v3;
        }

        double getIntersection(const Ray &ray) const override
        {
            // Transform ray to cube's local space
            Math::Vector3D localOrigin = inverseRotateVector(Math::Vector3D(
                ray.origin.x - _center.x,
                ray.origin.y - _center.y,
                ray.origin.z - _center.z
            ));
            Math::Vector3D localDirection = inverseRotateVector(ray.direction);

            Math::Vector3D invDir = Math::Vector3D(
                1.0 / localDirection.x,
                1.0 / localDirection.y,
                1.0 / localDirection.z
            );

            Math::Vector3D min = Math::Vector3D(
                -_size.x / 2.0,
                -_size.y / 2.0,
                -_size.z / 2.0
            );

            Math::Vector3D max = Math::Vector3D(
                _size.x / 2.0,
                _size.y / 2.0,
                _size.z / 2.0
            );

            double t1 = (min.x - localOrigin.x) * invDir.x;
            double t2 = (max.x - localOrigin.x) * invDir.x;
            double t3 = (min.y - localOrigin.y) * invDir.y;
            double t4 = (max.y - localOrigin.y) * invDir.y;
            double t5 = (min.z - localOrigin.z) * invDir.z;
            double t6 = (max.z - localOrigin.z) * invDir.z;

            double tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
            double tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

            if (tmax < 0)
                return -1.0;

            if (tmin > tmax)
                return -1.0;

            return tmin > 0 ? tmin : tmax;
        }

        bool hits(const Ray &ray) const override
        {
            return getIntersection(ray) > 0;
        }

        Math::Vector3D getNormal(const Math::Vector3D &point) const override
        {
            const double EPS = 1e-6;
            Math::Vector3D localPoint = inverseRotateVector(Math::Vector3D(
                point.x - _center.x,
                point.y - _center.y,
                point.z - _center.z
            ));

            Math::Vector3D normal(0, 0, 0);

            if (std::abs(std::abs(localPoint.x) - _size.x/2.0) < EPS)
                normal = Math::Vector3D(localPoint.x > 0 ? 1 : -1, 0, 0);
            else if (std::abs(std::abs(localPoint.y) - _size.y/2.0) < EPS)
                normal = Math::Vector3D(0, localPoint.y > 0 ? 1 : -1, 0);
            else if (std::abs(std::abs(localPoint.z) - _size.z/2.0) < EPS)
                normal = Math::Vector3D(0, 0, localPoint.z > 0 ? 1 : -1);

            return rotateVector(normal);
        }

        AABB getAABB() const override
        {
            double hx = _size.x / 2.0;
            double hy = _size.y / 2.0;
            double hz = _size.z / 2.0;

            Math::Vector3D corners[8] = {
                Math::Vector3D(-hx, -hy, -hz),
                Math::Vector3D( hx, -hy, -hz),
                Math::Vector3D(-hx,  hy, -hz),
                Math::Vector3D( hx,  hy, -hz),
                Math::Vector3D(-hx, -hy,  hz),
                Math::Vector3D( hx, -hy,  hz),
                Math::Vector3D(-hx,  hy,  hz),
                Math::Vector3D( hx,  hy,  hz),
            };

            double inf = std::numeric_limits<double>::max();
            Math::Vector3D bMin( inf,  inf,  inf);
            Math::Vector3D bMax(-inf, -inf, -inf);

            for (auto &corner : corners) {
                Math::Vector3D rotated = rotateVector(corner);
                Math::Vector3D world(
                    rotated.x + _center.x,
                    rotated.y + _center.y,
                    rotated.z + _center.z
                );

                bMin.x = std::min(bMin.x, world.x);
                bMin.y = std::min(bMin.y, world.y);
                bMin.z = std::min(bMin.z, world.z);

                bMax.x = std::max(bMax.x, world.x);
                bMax.y = std::max(bMax.y, world.y);
                bMax.z = std::max(bMax.z, world.z);
            }

            const double EPS = 1e-4;
            return AABB(
                Math::Vector3D(bMin.x - EPS, bMin.y - EPS, bMin.z - EPS),
                Math::Vector3D(bMax.x + EPS, bMax.y + EPS, bMax.z + EPS)
            );
        }


        void applyTranslation(const Math::Vector3D &t) override
        {
            _center.x += t.x;
            _center.y += t.y;
            _center.z += t.z;
        }

        void setMaterial(std::shared_ptr<IMaterial> material) override
        {
            _material = material;
        }

        std::shared_ptr<IMaterial> getMaterial() const override { return _material; }

        void applyRotation(const Math::Vector3D &angles) override
        {
            _rotation = angles;
        }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "(center: " << _center.x << "," << _center.y << "," << _center.z 
                << " size: " << _size.x << "," << _size.y << "," << _size.z
                << " material=" << (_material ? _material->getDebugInfo() : "none")
                << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "cubes";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new Cube(config);
}
