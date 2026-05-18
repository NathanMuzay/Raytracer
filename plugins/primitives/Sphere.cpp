/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Sphere
*/

#include <libconfig.h++>
#include <sstream>
#include <memory>

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class Sphere : public IPrimitive {
    public:
        Math::Point3D _center;
        double _radius;
        std::shared_ptr<IMaterial> _material;

        Sphere(const libconfig::Setting &config)
        {
            double x = LibconfigHelper::getNumeric(config, "x");
            double y = LibconfigHelper::getNumeric(config, "y");
            double z = LibconfigHelper::getNumeric(config, "z");
            double r = LibconfigHelper::getNumeric(config, "r");

            _center = Math::Point3D(x, y, z);
            _radius = r;
            _material = nullptr;
        }

        double getIntersection(const Ray &ray) const override
        {
            Math::Vector3D oc(
                ray.origin.x - _center.x,
                ray.origin.y - _center.y,
                ray.origin.z - _center.z
            );
            double a = ray.direction.dot(ray.direction);
            double b = 2.0 * oc.dot(ray.direction);
            double c = oc.dot(oc) - _radius * _radius;
            double discriminant = b * b - 4.0 * a * c;

            if (discriminant < 0)
                return -1;
            return (-b - std::sqrt(discriminant)) / (2.0 * a);
        }

        bool hits(const Ray &ray) const override
        {
            return getIntersection(ray) > 0;
        }

        Math::Vector3D getNormal(const Math::Vector3D &point) const override
        {
            Math::Vector3D normal(
                point.x - _center.x,
                point.y - _center.y,
                point.z - _center.z
            );
            return normal.normalize();
        }

        AABB getAABB() const override
        {
            return AABB(
                Math::Vector3D(_center.x - _radius, _center.y - _radius, _center.z - _radius),
                Math::Vector3D(_center.x + _radius, _center.y + _radius, _center.z + _radius)
            );
        }

        void setMaterial(std::shared_ptr<IMaterial> material) override
        {
            _material = material;
        }

        std::shared_ptr<IMaterial> getMaterial() const override { return _material; }

        void applyTranslation(const Math::Vector3D &translation) override
        {
            _center.x += translation.x;
            _center.y += translation.y;
            _center.z += translation.z;
        }

        void applyRotation(const Math::Vector3D &angles) override
        {
            (void)angles;
        }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "(pos: " << _center.x << "," << _center.y << "," << _center.z
                << " r: " << _radius
                << " material=" << (_material ? _material->getDebugInfo() : "none")
                << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "spheres";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new Sphere(config);
}
