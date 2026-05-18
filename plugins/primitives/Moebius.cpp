/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5
** File description:
** MoebiusStrip
*/

#include <libconfig.h++>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
** Moebius strip parametric form:
**   P(u,v) = ( (R + v*cos(u/2)) * cos(u),
**              (R + v*cos(u/2)) * sin(u),
**               v * sin(u/2) )
** u in [0, 2*PI], v in [-w, w]
**
** Intersection: numerical ray marching + Newton refinement
** (analytic form is a degree-6 polynomial -- too unstable)
*/

class MoebiusStrip : public IPrimitive {
public:
    Math::Point3D _center;
    double _R;  
    double _w; 
    Math::Vector3D _rotation;
    std::shared_ptr<IMaterial> _material;

    MoebiusStrip(const libconfig::Setting &config)
    {
        _center.x = LibconfigHelper::getNumeric(config, "x");
        _center.y = LibconfigHelper::getNumeric(config, "y");
        _center.z = LibconfigHelper::getNumeric(config, "z");
        _R        = LibconfigHelper::getNumeric(config, "R");
        _w        = LibconfigHelper::getNumeric(config, "w");
        _rotation = Math::Vector3D(0, 0, 0);
        _material = nullptr;

        if (config.exists("rotation")) {
            const libconfig::Setting &rot = config["rotation"];
            _rotation.x = LibconfigHelper::getNumeric(rot, "x");
            _rotation.y = LibconfigHelper::getNumeric(rot, "y");
            _rotation.z = LibconfigHelper::getNumeric(rot, "z");
        }
    }

    Math::Vector3D toLocal(const Math::Vector3D &v) const
    {
        double rx = _rotation.x * M_PI / 180.0;
        double ry = _rotation.y * M_PI / 180.0;
        double rz = _rotation.z * M_PI / 180.0;

        double cx = std::cos(-rx), sx = std::sin(-rx);
        double cy = std::cos(-ry), sy = std::sin(-ry);
        double cz = std::cos(-rz), sz = std::sin(-rz);

        double x1 =  cz*v.x + sz*v.y;
        double y1 = -sz*v.x + cz*v.y;
        double z1 = v.z;

        double x2 =  cy*x1 - sy*z1;
        double y2 = y1;
        double z2 =  sy*x1 + cy*z1;

        double x3 = x2;
        double y3 =  cx*y2 + sx*z2;
        double z3 = -sx*y2 + cx*z2;

        return Math::Vector3D(x3, y3, z3);
    }

    Math::Vector3D toWorld(const Math::Vector3D &v) const
    {
        double rx = _rotation.x * M_PI / 180.0;
        double ry = _rotation.y * M_PI / 180.0;
        double rz = _rotation.z * M_PI / 180.0;

        double cx = std::cos(rx), sx = std::sin(rx);
        double cy = std::cos(ry), sy = std::sin(ry);
        double cz = std::cos(rz), sz = std::sin(rz);

        double y1 =  cx*v.y - sx*v.z;
        double z1 =  sx*v.y + cx*v.z;
        double x1 = v.x;

        double x2 =  cy*x1 + sy*z1;
        double y2 = y1;
        double z2 = -sy*x1 + cy*z1;

        double x3 = cz*x2 - sz*y2;
        double y3 = sz*x2 + cz*y2;
        double z3 = z2;

        return Math::Vector3D(x3, y3, z3);
    }

    double intersectNumerical(
        const Math::Vector3D &ro,
        const Math::Vector3D &rd) const
    {
        const int   STEPS     = 512;
        const double T_MAX    = (_R + _w) * 6.0;
        const double dt       = T_MAX / STEPS;
        const double EPS      = 1e-4;

        double prevF = evalF(ro);
        double prevT = 0.0;
        double tHit  = -1.0;

        for (int i = 1; i <= STEPS; i++) {
            double t = i * dt;
            Math::Vector3D p(
                ro.x + t * rd.x,
                ro.y + t * rd.y,
                ro.z + t * rd.z
            );
            double f = evalF(p);

            if (prevF * f < 0.0) {
                double ta = prevT, tb = t;
                for (int j = 0; j < 32; j++) {
                    double tm = (ta + tb) * 0.5;
                    Math::Vector3D pm(
                        ro.x + tm * rd.x,
                        ro.y + tm * rd.y,
                        ro.z + tm * rd.z
                    );
                    double fm = evalF(pm);
                    if (fm * prevF < 0.0)
                        tb = tm;
                    else
                        ta = tm;
                }
                tHit = (ta + tb) * 0.5;
                if (tHit > EPS)
                    return tHit;
            }
            prevF = f;
            prevT = t;
        }
        return -1.0;
    }

    double evalF(const Math::Vector3D &p) const
    {
        double u = std::atan2(p.y, p.x);
        double cu2 = std::cos(u / 2.0);
        double su2 = std::sin(u / 2.0);
        double cu  = std::cos(u);
        double su  = std::sin(u);

        double cx = _R * cu;
        double cy = _R * su;
        double cz = 0.0;

        double tx = cu2 * cu;
        double ty = cu2 * su;
        double tz = su2;

        double dx = p.x - cx;
        double dy = p.y - cy;
        double dz = p.z - cz;

        double v = dx*tx + dy*ty + dz*tz;

        double rx = dx - v*tx;
        double ry = dy - v*ty;
        double rz = dz - v*tz;
        double residual = std::sqrt(rx*rx + ry*ry + rz*rz);

        double thickness = 0.015 * _R;
        if (std::abs(v) > _w) return 1.0; 
        return residual - thickness;
    }

    double getIntersection(const Ray &ray) const override
    {
        Math::Vector3D orig(
            ray.origin.x - _center.x,
            ray.origin.y - _center.y,
            ray.origin.z - _center.z
        );
        Math::Vector3D o = toLocal(orig);
        Math::Vector3D d = toLocal(ray.direction);

        return intersectNumerical(o, d);
    }

    bool hits(const Ray &ray) const override
    {
        return getIntersection(ray) > 0;
    }

    Math::Vector3D getNormal(const Math::Vector3D &point) const override
    {
        Math::Vector3D local(
            point.x - _center.x,
            point.y - _center.y,
            point.z - _center.z
        );
        Math::Vector3D lp = toLocal(local);

        // Numerical gradient of evalF
        double eps = 1e-4;
        double fx = evalF(Math::Vector3D(lp.x+eps, lp.y, lp.z))
                  - evalF(Math::Vector3D(lp.x-eps, lp.y, lp.z));
        double fy = evalF(Math::Vector3D(lp.x, lp.y+eps, lp.z))
                  - evalF(Math::Vector3D(lp.x, lp.y-eps, lp.z));
        double fz = evalF(Math::Vector3D(lp.x, lp.y, lp.z+eps))
                  - evalF(Math::Vector3D(lp.x, lp.y, lp.z-eps));

        Math::Vector3D nLocal(fx, fy, fz);
        Math::Vector3D nWorld = toWorld(nLocal);
        return nWorld.normalize();
    }

    AABB getAABB() const override
    {
        double extent = _R + _w;
        return AABB(
            Math::Vector3D(_center.x - extent, _center.y - _w, _center.z - extent),
            Math::Vector3D(_center.x + extent, _center.y + _w, _center.z + extent)
        );
    }

    void setMaterial(std::shared_ptr<IMaterial> material) override { _material = material; }
    std::shared_ptr<IMaterial> getMaterial() const override { return _material; }

    void applyTranslation(const Math::Vector3D &t) override
    {
        _center.x += t.x;
        _center.y += t.y;
        _center.z += t.z;
    }

    void applyRotation(const Math::Vector3D &angles) override
    {
        _rotation.x += angles.x;
        _rotation.y += angles.y;
        _rotation.z += angles.z;
    }

    std::string getDebugInfo() const override
    {
        std::ostringstream oss;
        oss << "(pos: " << _center.x << "," << _center.y << "," << _center.z
            << " R: " << _R << " w: " << _w
            << " material=" << (_material ? _material->getDebugInfo() : "none")
            << ")";
        return oss.str();
    }
};

extern "C" const char *getSectionName()
{
    return "moebius";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new MoebiusStrip(config);
}
