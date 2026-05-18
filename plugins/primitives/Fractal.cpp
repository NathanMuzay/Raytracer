/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Fractal - Mandelbulb via sphere-tracing (distance estimator)
*/

#include <libconfig.h++>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <memory>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class Fractal : public IPrimitive {
public:
    Math::Point3D              _center;
    double                     _scale;      // world-space scale of the bulb
    int                        _power;      // Mandelbulb power (8 = classic)
    int                        _iterations; // max orbit iterations
    double                     _bailout;    // escape radius
    std::shared_ptr<IMaterial> _material;
    Math::Vector3D             _rotation;

    Fractal(const libconfig::Setting &config)
    {
        double x = LibconfigHelper::getNumeric(config, "x");
        double y = LibconfigHelper::getNumeric(config, "y");
        double z = LibconfigHelper::getNumeric(config, "z");
        _center = Math::Point3D(x, y, z);

        _scale = 1.0;
        if (config.exists("scale"))
            _scale = LibconfigHelper::getNumeric(config, "scale");

        _power = 8;
        if (config.exists("power"))
            _power = static_cast<int>(LibconfigHelper::getNumeric(config, "power"));

        _iterations = 8;
        if (config.exists("iterations"))
            _iterations = static_cast<int>(LibconfigHelper::getNumeric(config, "iterations"));

        _bailout = 2.0;
        if (config.exists("bailout"))
            _bailout = LibconfigHelper::getNumeric(config, "bailout");

        _material = nullptr;
        _rotation = Math::Vector3D(0, 0, 0);
    }

    // ── Rotation helpers ─────────────────────────────────────────────────────
    Math::Vector3D rotateVector(const Math::Vector3D &v) const
    {
        double rx = _rotation.x * M_PI / 180.0;
        double ry = _rotation.y * M_PI / 180.0;
        double rz = _rotation.z * M_PI / 180.0;

        double cosX = std::cos(rx), sinX = std::sin(rx);
        Math::Vector3D v1(v.x, v.y * cosX - v.z * sinX, v.y * sinX + v.z * cosX);

        double cosY = std::cos(ry), sinY = std::sin(ry);
        Math::Vector3D v2(v1.x * cosY + v1.z * sinY, v1.y, -v1.x * sinY + v1.z * cosY);

        double cosZ = std::cos(rz), sinZ = std::sin(rz);
        return Math::Vector3D(v2.x * cosZ - v2.y * sinZ, v2.x * sinZ + v2.y * cosZ, v2.z);
    }

    Math::Vector3D inverseRotateVector(const Math::Vector3D &v) const
    {
        double rx = -_rotation.x * M_PI / 180.0;
        double ry = -_rotation.y * M_PI / 180.0;
        double rz = -_rotation.z * M_PI / 180.0;

        double cosZ = std::cos(rz), sinZ = std::sin(rz);
        Math::Vector3D v1(v.x * cosZ - v.y * sinZ, v.x * sinZ + v.y * cosZ, v.z);

        double cosY = std::cos(ry), sinY = std::sin(ry);
        Math::Vector3D v2(v1.x * cosY + v1.z * sinY, v1.y, -v1.x * sinY + v1.z * cosY);

        double cosX = std::cos(rx), sinX = std::sin(rx);
        return Math::Vector3D(v2.x, v2.y * cosX - v2.z * sinX, v2.y * sinX + v2.z * cosX);
    }

    // ── Mandelbulb distance estimator ─────────────────────────────────────────
    // Point p is in local normalised space (scaled by 1/_scale).
    // Returns a lower bound on the distance to the fractal surface.
    double de(double px, double py, double pz) const
    {
        double zx = px, zy = py, zz = pz;
        double dr = 1.0;
        double r  = 0.0;
        double n  = static_cast<double>(_power);

        for (int i = 0; i < _iterations; ++i) {
            r = std::sqrt(zx*zx + zy*zy + zz*zz);
            if (r > _bailout) break;

            // Convert to spherical
            double theta = std::acos(zz / r);
            double phi   = std::atan2(zy, zx);
            dr = std::pow(r, n - 1.0) * n * dr + 1.0;

            // Scale and rotate
            double rn  = std::pow(r, n);
            double thn = theta * n;
            double phn = phi   * n;

            double sinThn = std::sin(thn);
            zx = rn * sinThn * std::cos(phn) + px;
            zy = rn * sinThn * std::sin(phn) + py;
            zz = rn * std::cos(thn)           + pz;
        }

        // 0.5 * log(r) * r / dr  is the standard Mandelbulb DE formula
        return 0.5 * std::log(r) * r / dr;
    }

    // DE in world scale
    double deWorld(double px, double py, double pz) const
    {
        return de(px / _scale, py / _scale, pz / _scale) * _scale;
    }

    // ── Sphere-tracing intersection ───────────────────────────────────────────
    double getIntersection(const Ray &ray) const override
    {
        // To local (un-rotated, centred) space
        Math::Vector3D lOrig = inverseRotateVector(Math::Vector3D(
            ray.origin.x - _center.x,
            ray.origin.y - _center.y,
            ray.origin.z - _center.z
        ));
        Math::Vector3D lDir = inverseRotateVector(ray.direction).normalize();

        // Clip to bounding sphere of radius ~1.3 * _scale
        double R  = _scale * 1.3;
        double b  = lOrig.x*lDir.x + lOrig.y*lDir.y + lOrig.z*lDir.z;
        double cc = lOrig.x*lOrig.x + lOrig.y*lOrig.y + lOrig.z*lOrig.z - R*R;
        double disc = b*b - cc;
        if (disc < 0) return -1.0;

        double sqrtDisc = std::sqrt(disc);
        double tEnter   = -b - sqrtDisc;
        double tExit    = -b + sqrtDisc;
        if (tExit < 1e-4) return -1.0;

        double t = std::max(tEnter, 1e-4);

        const int    MAX_STEPS  = 128;
        const double SURFACE_EPS = 1e-4 * _scale;
        const double MIN_STEP    = 1e-5 * _scale;

        for (int i = 0; i < MAX_STEPS && t < tExit; ++i) {
            double px = lOrig.x + t * lDir.x;
            double py = lOrig.y + t * lDir.y;
            double pz = lOrig.z + t * lDir.z;

            double d = deWorld(px, py, pz);

            if (d < SURFACE_EPS)
                return t;

            t += std::max(d * 0.9, MIN_STEP);
        }
        return -1.0;
    }

    bool hits(const Ray &ray) const override
    {
        return getIntersection(ray) > 0;
    }

    // ── Normal via central-difference on the DE ───────────────────────────────
    Math::Vector3D getNormal(const Math::Vector3D &point) const override
    {
        Math::Vector3D lp = inverseRotateVector(Math::Vector3D(
            point.x - _center.x,
            point.y - _center.y,
            point.z - _center.z
        ));

        double eps = 1e-4 * _scale;
        double dx = deWorld(lp.x + eps, lp.y,       lp.z      )
                  - deWorld(lp.x - eps, lp.y,       lp.z      );
        double dy = deWorld(lp.x,       lp.y + eps, lp.z      )
                  - deWorld(lp.x,       lp.y - eps, lp.z      );
        double dz = deWorld(lp.x,       lp.y,       lp.z + eps)
                  - deWorld(lp.x,       lp.y,       lp.z - eps);

        double len = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (len < 1e-10) return Math::Vector3D(0, 1, 0);

        Math::Vector3D localNormal(dx / len, dy / len, dz / len);
        return rotateVector(localNormal);
    }

    // ── AABB : bounding sphere box ────────────────────────────────────────────
    AABB getAABB() const override
    {
        double R = _scale * 1.3;
        const double EPS = 1e-4;
        return AABB(
            Math::Vector3D(_center.x - R - EPS, _center.y - R - EPS, _center.z - R - EPS),
            Math::Vector3D(_center.x + R + EPS, _center.y + R + EPS, _center.z + R + EPS)
        );
    }

    void applyTranslation(const Math::Vector3D &t) override
    {
        _center.x += t.x;
        _center.y += t.y;
        _center.z += t.z;
    }

    void applyRotation(const Math::Vector3D &angles) override
    {
        _rotation = angles;
    }

    void setMaterial(std::shared_ptr<IMaterial> material) override { _material = material; }
    std::shared_ptr<IMaterial> getMaterial() const override { return _material; }

    std::string getDebugInfo() const override
    {
        std::ostringstream oss;
        oss << "Fractal/Mandelbulb(center=" << _center.x << "," << _center.y << "," << _center.z
            << " scale=" << _scale << " power=" << _power
            << " iterations=" << _iterations
            << " material=" << (_material ? _material->getDebugInfo() : "none") << ")";
        return oss.str();
    }
};

extern "C" const char *getSectionName()
{
    return "fractals";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new Fractal(config);
}
