/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Cylindre
*/

#include <libconfig.h++>
#include <sstream>
#include <cmath>
#include <memory>

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class Cylindre : public IPrimitive {
    public:
        Math::Point3D _center;
        Math::Vector3D _axis;
        double _radius;
        double _height;
        std::shared_ptr<IMaterial> _material;

        Cylindre(const libconfig::Setting &config)
        {
            double x = LibconfigHelper::getNumeric(config, "x");
            double y = LibconfigHelper::getNumeric(config, "y");
            double z = LibconfigHelper::getNumeric(config, "z");
            double r = LibconfigHelper::getNumeric(config, "r");
            double h = LibconfigHelper::getNumeric(config, "h");

            _center = Math::Point3D(x, y, z);
            _axis = Math::Vector3D(0, 1, 0);
            _radius = r;
            _height = h;
            _material = nullptr;
        }

        double getIntersection(const Ray &ray) const override
        {
            const double EPS = 1e-4;

            Math::Vector3D axis = _axis.normalize();
            Math::Vector3D oc(
                ray.origin.x - _center.x,
                ray.origin.y - _center.y,
                ray.origin.z - _center.z
            );

            double dDotA = ray.direction.dot(axis);
            double ocDotA = oc.dot(axis);

            Math::Vector3D dPerp = ray.direction - axis * dDotA;
            Math::Vector3D ocPerp = oc - axis * ocDotA;

            double a = dPerp.dot(dPerp);
            double b = 2.0 * ocPerp.dot(dPerp);
            double c = ocPerp.dot(ocPerp) - _radius * _radius;

            double halfH = _height / 2.0;
            double bestT = -1.0;

            if (std::abs(a) > 1e-12) {
                double disc = b * b - 4.0 * a * c;
                if (disc >= 0.0) {
                    double sq = std::sqrt(disc);
                    double t0 = (-b - sq) / (2.0 * a);
                    double t1 = (-b + sq) / (2.0 * a);
                    if (t0 > t1) std::swap(t0, t1);

                    for (double t : {t0, t1}) {
                        if (t <= EPS) continue;
                        double axisPos = ocDotA + t * dDotA;
                        if (std::abs(axisPos) <= halfH + 1e-8) {
                            bestT = (bestT < 0.0) ? t : std::min(bestT, t);
                            break;
                        }
                    }
                }
            }

            if (std::abs(dDotA) > 1e-12) {
                for (double capSign : {-1.0, 1.0}) {
                    double t = ((capSign * halfH) - ocDotA) / dDotA;
                    if (t <= EPS)
                        continue;

                    Math::Vector3D hit = oc + ray.direction * t;
                    double axisDist = hit.dot(axis);
                    Math::Vector3D radial = hit - axis * axisDist;
                    if (radial.dot(radial) <= _radius * _radius + 1e-8) {
                        if (bestT < 0.0 || t < bestT)
                            bestT = t;
                    }
                }
            }

            return (bestT > EPS) ? bestT : -1.0;
        }

        bool hits(const Ray &ray) const override
        {
            return getIntersection(ray) > 0;
        }

        Math::Vector3D getNormal(const Math::Vector3D &point) const override
        {
            const double EPSN = 1e-4;
            Math::Vector3D axis = _axis.normalize();
            Math::Vector3D pc(
                point.x - _center.x,
                point.y - _center.y,
                point.z - _center.z
            );
            double axisPos = pc.dot(axis);
            double halfH = _height / 2.0;

            if (std::abs(axisPos + halfH) < EPSN)
                return axis * -1.0;
            if (std::abs(axisPos - halfH) < EPSN)
                return axis;

            Math::Vector3D n = pc - axis * axisPos;
            return n.normalize();
        }

        AABB getAABB() const override
        {
            Math::Vector3D ax = _axis.normalize();
            double halfH = _height / 2.0;

            double ex = _radius * std::sqrt(std::max(0.0, 1.0 - ax.x * ax.x));
            double ey = _radius * std::sqrt(std::max(0.0, 1.0 - ax.y * ax.y));
            double ez = _radius * std::sqrt(std::max(0.0, 1.0 - ax.z * ax.z));

            double hx = std::abs(ax.x) * halfH;
            double hy = std::abs(ax.y) * halfH;
            double hz = std::abs(ax.z) * halfH;

            double dx = ex + hx;
            double dy = ey + hy;
            double dz = ez + hz;

            return AABB(
                Math::Vector3D(_center.x - dx, _center.y - dy, _center.z - dz),
                Math::Vector3D(_center.x + dx, _center.y + dy, _center.z + dz)
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
            double rx = angles.x * M_PI / 180.0;
            double ry = angles.y * M_PI / 180.0;
            double rz = angles.z * M_PI / 180.0;

            auto rotateVector = [&](const Math::Vector3D &v) {
                Math::Vector3D result = v;
                if (rx != 0.0) {
                    double ny = result.y * std::cos(rx) - result.z * std::sin(rx);
                    double nz = result.y * std::sin(rx) + result.z * std::cos(rx);
                    result.y = ny; result.z = nz;
                }
                if (ry != 0.0) {
                    double nx = result.x * std::cos(ry) + result.z * std::sin(ry);
                    double nz = -result.x * std::sin(ry) + result.z * std::cos(ry);
                    result.x = nx; result.z = nz;
                }
                if (rz != 0.0) {
                    double nx = result.x * std::cos(rz) - result.y * std::sin(rz);
                    double ny = result.x * std::sin(rz) + result.y * std::cos(rz);
                    result.x = nx; result.y = ny;
                }
                return result;
            };

            _axis = rotateVector(_axis).normalize();
        }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "(center: " << _center.x << "," << _center.y << "," << _center.z
                << " radius: " << _radius << " height: " << _height
                << " material=" << (_material ? _material->getDebugInfo() : "none")
                << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "cylindres";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new Cylindre(config);
}
