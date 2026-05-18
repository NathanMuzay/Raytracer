/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5
** File description:
** ObjMesh primitive plugin
*/

#include <libconfig.h++>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include <string>

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

static Math::Vector3D vec_cross(const Math::Vector3D &a, const Math::Vector3D &b)
{
    return Math::Vector3D(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static double vec_dot(const Math::Vector3D &a, const Math::Vector3D &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Math::Vector3D vec_sub(const Math::Vector3D &a, const Math::Vector3D &b)
{
    return Math::Vector3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

static Math::Vector3D point_sub_vec(const Math::Point3D &p, const Math::Vector3D &v)
{
    return Math::Vector3D(p.x - v.x, p.y - v.y, p.z - v.z);
}

struct Triangle {
    Math::Vector3D v0, v1, v2;
    Math::Vector3D normal;

    Triangle(const Math::Vector3D &a, const Math::Vector3D &b, const Math::Vector3D &c)
        : v0(a), v1(b), v2(c)
    {
        Math::Vector3D e1 = vec_sub(b, a);
        Math::Vector3D e2 = vec_sub(c, a);
        Math::Vector3D n  = vec_cross(e1, e2);
        double len = std::sqrt(vec_dot(n, n));
        normal = (len > 1e-12)
            ? Math::Vector3D(n.x / len, n.y / len, n.z / len)
            : Math::Vector3D(0, 0, 1);
    }

    double intersect(const Ray &ray) const
    {
        constexpr double EPS = 1e-8;
        Math::Vector3D e1 = vec_sub(v1, v0);
        Math::Vector3D e2 = vec_sub(v2, v0);

        Math::Vector3D h = vec_cross(ray.direction, e2);
        double a = vec_dot(e1, h);
        if (std::abs(a) < EPS)
            return -1.0;

        double f = 1.0 / a;

        Math::Vector3D s = point_sub_vec(ray.origin, v0);

        double u = f * vec_dot(s, h);
        if (u < 0.0 || u > 1.0)
            return -1.0;

        Math::Vector3D q = vec_cross(s, e1);
        double v = f * vec_dot(ray.direction, q);
        if (v < 0.0 || u + v > 1.0)
            return -1.0;

        double t = f * vec_dot(e2, q);
        return (t > 1e-4) ? t : -1.0;
    }
};

class ObjMesh : public IPrimitive {
public:
    std::vector<Triangle>      _triangles;
    std::shared_ptr<IMaterial> _material;

    ObjMesh(const libconfig::Setting &config)
    {
        std::string path = (const char *)config["path"];

        double scale = 1.0;
        if (config.exists("scale"))
            scale = LibconfigHelper::getNumeric(config, "scale");

        double ox = 0.0, oy = 0.0, oz = 0.0;
        if (config.exists("offset")) {
            ox = LibconfigHelper::getNumeric(config["offset"], "x");
            oy = LibconfigHelper::getNumeric(config["offset"], "y");
            oz = LibconfigHelper::getNumeric(config["offset"], "z");
        }

        loadObj(path, scale, Math::Vector3D(ox, oy, oz));
        _material = nullptr;
    }

    double getIntersection(const Ray &ray) const override
    {
        double best = -1.0;
        for (const auto &tri : _triangles) {
            double t = tri.intersect(ray);
            if (t > 0.0 && (best < 0.0 || t < best))
                best = t;
        }
        return best;
    }

    bool hits(const Ray &ray) const override
    {
        return getIntersection(ray) > 0.0;
    }

    Math::Vector3D getNormal(const Math::Vector3D &point) const override
    {
        double best    = std::numeric_limits<double>::max();
        int    bestIdx = 0;
        for (int i = 0; i < (int)_triangles.size(); ++i) {
            double d = std::abs(vec_dot(_triangles[i].normal,
                                        vec_sub(point, _triangles[i].v0)));
            if (d < best) { best = d; bestIdx = i; }
        }
        return _triangles[bestIdx].normal;
    }

    void setMaterial(std::shared_ptr<IMaterial> mat) override { _material = mat; }

    std::shared_ptr<IMaterial> getMaterial() const override { return _material; }

    AABB getAABB() const override
    {
        double inf = std::numeric_limits<double>::max();
        Math::Vector3D bMin( inf,  inf,  inf);
        Math::Vector3D bMax(-inf, -inf, -inf);

        for (const auto &tri : _triangles) {
            for (const auto &v : {tri.v0, tri.v1, tri.v2}) {
                bMin.x = std::min(bMin.x, v.x);
                bMin.y = std::min(bMin.y, v.y);
                bMin.z = std::min(bMin.z, v.z);

                bMax.x = std::max(bMax.x, v.x);
                bMax.y = std::max(bMax.y, v.y);
                bMax.z = std::max(bMax.z, v.z);
            }
        }

        const double EPS = 1e-4;
        return AABB(
            Math::Vector3D(bMin.x - EPS, bMin.y - EPS, bMin.z - EPS),
            Math::Vector3D(bMax.x + EPS, bMax.y + EPS, bMax.z + EPS)
        );
    }


    void applyTranslation(const Math::Vector3D &t) override
    {
        for (auto &tri : _triangles) {
            tri.v0 += t; tri.v1 += t; tri.v2 += t;
        }
    }

    void applyRotation(const Math::Vector3D &angles) override
    {
        double rx = angles.x * M_PI / 180.0;
        double ry = angles.y * M_PI / 180.0;
        double rz = angles.z * M_PI / 180.0;

        auto rotateVec = [&](Math::Vector3D &v) {
            if (rx != 0.0) {
                double y = v.y * std::cos(rx) - v.z * std::sin(rx);
                double z = v.y * std::sin(rx) + v.z * std::cos(rx);
                v.y = y; v.z = z;
            }
            if (ry != 0.0) {
                double x =  v.x * std::cos(ry) + v.z * std::sin(ry);
                double z = -v.x * std::sin(ry) + v.z * std::cos(ry);
                v.x = x; v.z = z;
            }
            if (rz != 0.0) {
                double x = v.x * std::cos(rz) - v.y * std::sin(rz);
                double y = v.x * std::sin(rz) + v.y * std::cos(rz);
                v.x = x; v.y = y;
            }
        };

        for (auto &tri : _triangles) {
            rotateVec(tri.v0);
            rotateVec(tri.v1);
            rotateVec(tri.v2);
            Math::Vector3D e1 = vec_sub(tri.v1, tri.v0);
            Math::Vector3D e2 = vec_sub(tri.v2, tri.v0);
            Math::Vector3D n  = vec_cross(e1, e2);
            double len = std::sqrt(vec_dot(n, n));
            tri.normal = (len > 1e-12)
                ? Math::Vector3D(n.x / len, n.y / len, n.z / len)
                : Math::Vector3D(0, 0, 1);
        }
    }

    std::string getDebugInfo() const override
    {
        std::ostringstream oss;
        oss << "ObjMesh(" << _triangles.size() << " triangles"
            << " material=" << (_material ? _material->getDebugInfo() : "none")
            << ")";
        return oss.str();
    }

private:
    void loadObj(const std::string &path, double scale, const Math::Vector3D &offset)
    {
        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error("ObjMesh: cannot open file: " + path);

        std::vector<Math::Vector3D> verts;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream ss(line);
            std::string token;
            ss >> token;

            if (token == "v") {
                double x, y, z;
                ss >> x >> y >> z;
                verts.push_back(Math::Vector3D(
                    x * scale + offset.x,
                    y * scale + offset.y,
                    z * scale + offset.z
                ));
            } else if (token == "f") {
                std::vector<int> indices;
                std::string part;
                while (ss >> part) {
                    int idx = std::stoi(part.substr(0, part.find('/')));
                    idx = (idx < 0) ? (int)verts.size() + idx : idx - 1;
                    indices.push_back(idx);
                }
                for (int i = 1; i + 1 < (int)indices.size(); ++i) {
                    int a = indices[0], b = indices[i], c = indices[i + 1];
                    if (a < 0 || b < 0 || c < 0 ||
                        a >= (int)verts.size() ||
                        b >= (int)verts.size() ||
                        c >= (int)verts.size())
                        continue;
                    _triangles.emplace_back(verts[a], verts[b], verts[c]);
                }
            }
        }

        if (_triangles.empty())
            throw std::runtime_error("ObjMesh: no triangles loaded from: " + path);
    }
};

extern "C" const char *getSectionName()
{
    return "objmeshes";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new ObjMesh(config);
}
