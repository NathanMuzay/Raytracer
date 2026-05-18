/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** AABB - Axis-Aligned Bounding Box
*/

#ifndef AABB_HPP_
#define AABB_HPP_

#include <algorithm>
#include <limits>
#include "../Math/Math.hpp"
#include "../Camera/Ray.hpp"

struct AABB {
    Math::Vector3D min;
    Math::Vector3D max;

    AABB()
        : min(Math::Vector3D( std::numeric_limits<double>::max(),
                               std::numeric_limits<double>::max(),
                               std::numeric_limits<double>::max()))
        , max(Math::Vector3D(-std::numeric_limits<double>::max(),
                              -std::numeric_limits<double>::max(),
                              -std::numeric_limits<double>::max()))
    {}

    AABB(const Math::Vector3D &min, const Math::Vector3D &max)
        : min(min), max(max)
    {}

    // Fusionne deux AABB en une seule qui englobe les deux
    static AABB merge(const AABB &a, const AABB &b)
    {
        return AABB(
            Math::Vector3D(std::min(a.min.x, b.min.x),
                           std::min(a.min.y, b.min.y),
                           std::min(a.min.z, b.min.z)),
            Math::Vector3D(std::max(a.max.x, b.max.x),
                           std::max(a.max.y, b.max.y),
                           std::max(a.max.z, b.max.z))
        );
    }

    // Test d'intersection ray/AABB (slab method)
    // Retourne true si le ray touche la boîte dans [tMin, tMax]
    bool intersects(const Ray &ray, double tMin = 1e-4, double tMax = std::numeric_limits<double>::max()) const
    {
        // Axe X
        double invDx = (ray.direction.x != 0.0) ? 1.0 / ray.direction.x : std::numeric_limits<double>::max();
        double tx0 = (min.x - ray.origin.x) * invDx;
        double tx1 = (max.x - ray.origin.x) * invDx;
        if (invDx < 0) std::swap(tx0, tx1);
        tMin = std::max(tMin, tx0);
        tMax = std::min(tMax, tx1);
        if (tMax < tMin) return false;

        // Axe Y
        double invDy = (ray.direction.y != 0.0) ? 1.0 / ray.direction.y : std::numeric_limits<double>::max();
        double ty0 = (min.y - ray.origin.y) * invDy;
        double ty1 = (max.y - ray.origin.y) * invDy;
        if (invDy < 0) std::swap(ty0, ty1);
        tMin = std::max(tMin, ty0);
        tMax = std::min(tMax, ty1);
        if (tMax < tMin) return false;

        // Axe Z
        double invDz = (ray.direction.z != 0.0) ? 1.0 / ray.direction.z : std::numeric_limits<double>::max();
        double tz0 = (min.z - ray.origin.z) * invDz;
        double tz1 = (max.z - ray.origin.z) * invDz;
        if (invDz < 0) std::swap(tz0, tz1);
        tMin = std::max(tMin, tz0);
        tMax = std::min(tMax, tz1);
        if (tMax < tMin) return false;

        return true;
    }

    // Centre de la boîte (utilisé pour le tri SAH)
    Math::Vector3D centroid() const
    {
        return Math::Vector3D(
            (min.x + max.x) * 0.5,
            (min.y + max.y) * 0.5,
            (min.z + max.z) * 0.5
        );
    }
};

#endif /* !AABB_HPP_ */
