/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Tanglecube - implicit surface: x^4 + y^4 + z^4 - k*(x^2+y^2+z^2) + c = 0
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

class Tanglecube : public IPrimitive {
public:
    Math::Point3D              _center;
    double                     _scale;
    double                     _k;
    double                     _c;
    std::shared_ptr<IMaterial> _material;
    Math::Vector3D             _rotation;

    Tanglecube(const libconfig::Setting &config)
    {
        double x = LibconfigHelper::getNumeric(config, "x");
        double y = LibconfigHelper::getNumeric(config, "y");
        double z = LibconfigHelper::getNumeric(config, "z");
        _center = Math::Point3D(x, y, z);

        _scale = 1.0;
        if (config.exists("scale"))
            _scale = LibconfigHelper::getNumeric(config, "scale");

        _k = 5.0;
        if (config.exists("k"))
            _k = LibconfigHelper::getNumeric(config, "k");

        _c = 11.8;
        if (config.exists("c"))
            _c = LibconfigHelper::getNumeric(config, "c");

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

    // ── Implicit function in normalised local space ───────────────────────────
    // p must already be divided by _scale
    double f(double x, double y, double z) const
    {
        double x2 = x*x, y2 = y*y, z2 = z*z;
        return x2*x2 + y2*y2 + z2*z2 - _k*(x2 + y2 + z2) + _c;
    }

    // Analytical gradient of f
    Math::Vector3D grad(double x, double y, double z) const
    {
        return Math::Vector3D(
            4.0*x*x*x - 2.0*_k*x,
            4.0*y*y*y - 2.0*_k*y,
            4.0*z*z*z - 2.0*_k*z
        );
    }

    // Bounding radius in normalised space (outer zero of f(r,0,0))
    double boundRadiusNorm() const
    {
        double disc = _k*_k - 4.0*_c;
        if (disc <= 0.0) return std::sqrt(_k) * 1.1;
        return std::sqrt((_k + std::sqrt(disc)) * 0.5) * 1.05;
    }

    // ── Intersection via sign-change ray marching ─────────────────────────────
    double getIntersection(const Ray &ray) const override
    {
        // Transform ray to local centred un-rotated space.
        // We do NOT scale the direction — t values stay in the same unit as
        // the world space ray, we only divide positions by _scale when feeding f().
        Math::Vector3D localOrig = inverseRotateVector(Math::Vector3D(
            ray.origin.x - _center.x,
            ray.origin.y - _center.y,
            ray.origin.z - _center.z
        ));
        Math::Vector3D rawDir = inverseRotateVector(ray.direction);

        // Normalise direction so t == arc-length in local space.
        // We track dirLen to convert the hit t back to world t at the end.
        double dirLen = std::sqrt(rawDir.x*rawDir.x + rawDir.y*rawDir.y + rawDir.z*rawDir.z);
        if (dirLen < 1e-12) return -1.0;
        Math::Vector3D localDir(rawDir.x / dirLen, rawDir.y / dirLen, rawDir.z / dirLen);

        // Clip to bounding sphere (radius in local un-normalised space)
        double R = boundRadiusNorm() * _scale;

        double bCoef = localOrig.x*localDir.x + localOrig.y*localDir.y + localOrig.z*localDir.z;
        double cCoef = localOrig.x*localOrig.x + localOrig.y*localOrig.y + localOrig.z*localOrig.z - R*R;
        double sphereDisc = bCoef*bCoef - cCoef;
        if (sphereDisc < 0.0) return -1.0;

        double sqrtDisc = std::sqrt(sphereDisc);
        double tEnter   = -bCoef - sqrtDisc;
        double tExit    = -bCoef + sqrtDisc;
        if (tExit < 1e-4) return -1.0;

        double tStart = std::max(tEnter, 1e-4);

        // Uniform marching — step = diameter / MAX_STEPS
        // 512 steps guarantees we don't skip thin lobes of the tanglecube
        const int MAX_STEPS = 512;
        double step = (tExit - tStart) / static_cast<double>(MAX_STEPS);

        // Helper: evaluate f at a given t (divides by _scale internally)
        auto eval = [&](double tt) -> double {
            return f(
                (localOrig.x + tt * localDir.x) / _scale,
                (localOrig.y + tt * localDir.y) / _scale,
                (localOrig.z + tt * localDir.z) / _scale
            );
        };

        double t     = tStart;
        double fPrev = eval(t);

        for (int i = 0; i < MAX_STEPS; ++i) {
            t += step;
            double fCur = eval(t);

            if (fPrev * fCur <= 0.0) {
                // Bisect to refine the crossing
                double tLo = t - step, tHi = t;
                double fLo = fPrev;
                for (int iter = 0; iter < 24; ++iter) {
                    double tMid = (tLo + tHi) * 0.5;
                    double fMid = eval(tMid);
                    if (fLo * fMid <= 0.0) {
                        tHi = tMid;
                    } else {
                        tLo = tMid;
                        fLo = fMid;
                    }
                }
                // Convert normalised-direction t back to world-ray t
                return (tLo + tHi) * 0.5 / dirLen;
            }
            fPrev = fCur;
        }
        return -1.0;
    }

    bool hits(const Ray &ray) const override
    {
        return getIntersection(ray) > 0;
    }

    // ── Normal via analytical gradient ────────────────────────────────────────
    Math::Vector3D getNormal(const Math::Vector3D &point) const override
    {
        Math::Vector3D lp = inverseRotateVector(Math::Vector3D(
            point.x - _center.x,
            point.y - _center.y,
            point.z - _center.z
        ));
        Math::Vector3D g = grad(lp.x / _scale, lp.y / _scale, lp.z / _scale);
        double len = std::sqrt(g.x*g.x + g.y*g.y + g.z*g.z);
        if (len < 1e-10) return Math::Vector3D(0, 1, 0);
        g.x /= len; g.y /= len; g.z /= len;
        return rotateVector(g);
    }

    // ── AABB ──────────────────────────────────────────────────────────────────
    AABB getAABB() const override
    {
        double R = boundRadiusNorm() * _scale;
        const double EPS = 1e-3;
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

    void applyRotation(const Math::Vector3D &angles) override { _rotation = angles; }
    void setMaterial(std::shared_ptr<IMaterial> m) override   { _material = m; }
    std::shared_ptr<IMaterial> getMaterial() const override   { return _material; }

    std::string getDebugInfo() const override
    {
        std::ostringstream oss;
        oss << "Tanglecube(center=" << _center.x << "," << _center.y << "," << _center.z
            << " scale=" << _scale << " k=" << _k << " c=" << _c
            << " material=" << (_material ? _material->getDebugInfo() : "none") << ")";
        return oss.str();
    }
};

extern "C" const char *getSectionName() { return "tanglecubes"; }
extern "C" IPrimitive *createPlugin(const libconfig::Setting &config) { return new Tanglecube(config); }
