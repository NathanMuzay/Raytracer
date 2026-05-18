/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Triangle
*/

#include <libconfig.h++>
#include <sstream>
#include <memory>
#include <cmath>

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

// Helper cross product car Vector3D n'en a pas
static Math::Vector3D crossProduct(const Math::Vector3D &a, const Math::Vector3D &b)
{
    return Math::Vector3D(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

class Triangle : public IPrimitive {
    public:
        Math::Vector3D _v0, _v1, _v2;
        Math::Vector3D _normal;
        std::shared_ptr<IMaterial> _material;

        Triangle(const libconfig::Setting &config)
        {
            _v0 = Math::Vector3D(
                LibconfigHelper::getNumeric(config["v0"], "x"),
                LibconfigHelper::getNumeric(config["v0"], "y"),
                LibconfigHelper::getNumeric(config["v0"], "z")
            );
            _v1 = Math::Vector3D(
                LibconfigHelper::getNumeric(config["v1"], "x"),
                LibconfigHelper::getNumeric(config["v1"], "y"),
                LibconfigHelper::getNumeric(config["v1"], "z")
            );
            _v2 = Math::Vector3D(
                LibconfigHelper::getNumeric(config["v2"], "x"),
                LibconfigHelper::getNumeric(config["v2"], "y"),
                LibconfigHelper::getNumeric(config["v2"], "z")
            );

            Math::Vector3D e1 = _v1 - _v0;
            Math::Vector3D e2 = _v2 - _v0;
            _normal = crossProduct(e1, e2).normalize();
        }

        // Möller–Trumbore
        double getIntersection(const Ray &ray) const override
        {
            const double EPSILON = 1e-8;

            Math::Vector3D e1 = _v1 - _v0;
            Math::Vector3D e2 = _v2 - _v0;

            Math::Vector3D h = crossProduct(ray.direction, e2);
            double det = e1.dot(h);

            if (std::fabs(det) < EPSILON)
                return -1.0;

            double invDet = 1.0 / det;

            Math::Vector3D s = Math::Vector3D(
                ray.origin.x - _v0.x,
                ray.origin.y - _v0.y,
                ray.origin.z - _v0.z
            );

            double u = s.dot(h) * invDet;
            if (u < 0.0 || u > 1.0)
                return -1.0;

            Math::Vector3D q = crossProduct(s, e1);
            double v = ray.direction.dot(q) * invDet;
            if (v < 0.0 || u + v > 1.0)
                return -1.0;

            double t = e2.dot(q) * invDet;
            if (t < EPSILON)
                return -1.0;

            return t;
        }

        bool hits(const Ray &ray) const override
        {
            return getIntersection(ray) > 0.0;
        }

        Math::Vector3D getNormal(const Math::Vector3D &point) const override
        {
            (void)point;
            return _normal;
        }

        AABB getAABB() const override
        {
            const double EPS = 1e-4;

            Math::Vector3D minP(
                std::min({_v0.x, _v1.x, _v2.x}) - EPS,
                std::min({_v0.y, _v1.y, _v2.y}) - EPS,
                std::min({_v0.z, _v1.z, _v2.z}) - EPS
            );
            Math::Vector3D maxP(
                std::max({_v0.x, _v1.x, _v2.x}) + EPS,
                std::max({_v0.y, _v1.y, _v2.y}) + EPS,
                std::max({_v0.z, _v1.z, _v2.z}) + EPS
            );

            return AABB(minP, maxP);
        }

        void setMaterial(std::shared_ptr<IMaterial> material) override
        {
            _material = material;
        }

        std::shared_ptr<IMaterial> getMaterial() const override
        {
            return _material;
        }

        void applyTranslation(const Math::Vector3D &t) override
        {
            _v0 += t;
            _v1 += t;
            _v2 += t;
            // La normale ne change pas avec une translation
        }

        void applyRotation(const Math::Vector3D &angles) override
        {
            // Rotation autour du centroïde
            Math::Vector3D center(
                (_v0.x + _v1.x + _v2.x) / 3.0,
                (_v0.y + _v1.y + _v2.y) / 3.0,
                (_v0.z + _v1.z + _v2.z) / 3.0
            );

            auto rotateVertex = [&](Math::Vector3D &v) {
                // Translate vers l'origine
                v = v - center;

                // Rotation X
                if (angles.x != 0.0) {
                    double cx = std::cos(angles.x * M_PI / 180.0);
                    double sx = std::sin(angles.x * M_PI / 180.0);
                    double ny = v.y * cx - v.z * sx;
                    double nz = v.y * sx + v.z * cx;
                    v.y = ny;
                    v.z = nz;
                }
                // Rotation Y
                if (angles.y != 0.0) {
                    double cy = std::cos(angles.y * M_PI / 180.0);
                    double sy = std::sin(angles.y * M_PI / 180.0);
                    double nx = v.x * cy + v.z * sy;
                    double nz = -v.x * sy + v.z * cy;
                    v.x = nx;
                    v.z = nz;
                }
                // Rotation Z
                if (angles.z != 0.0) {
                    double cz = std::cos(angles.z * M_PI / 180.0);
                    double sz = std::sin(angles.z * M_PI / 180.0);
                    double nx = v.x * cz - v.y * sz;
                    double ny = v.x * sz + v.y * cz;
                    v.x = nx;
                    v.y = ny;
                }

                // Retranslate
                v += center;
            };

            rotateVertex(_v0);
            rotateVertex(_v1);
            rotateVertex(_v2);

            // Recalcule la normale
            Math::Vector3D e1 = _v1 - _v0;
            Math::Vector3D e2 = _v2 - _v0;
            _normal = crossProduct(e1, e2).normalize();
        }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "(v0: " << _v0.x << "," << _v0.y << "," << _v0.z
                << " v1: " << _v1.x << "," << _v1.y << "," << _v1.z
                << " v2: " << _v2.x << "," << _v2.y << "," << _v2.z
                << " material=" << (_material ? _material->getDebugInfo() : "none")
                << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "triangles";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new Triangle(config);
}
