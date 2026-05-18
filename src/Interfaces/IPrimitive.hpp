/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** IPrimitive
*/

#ifndef IPRIMITIVE_HPP_
#define IPRIMITIVE_HPP_

#include <string>
#include <memory>

#include "../Math/Math.hpp"
#include "../Camera/Ray.hpp"
#include "../BVH/AABB.hpp"

#include "IMaterial.hpp"

class IPrimitive {
    public:
        virtual ~IPrimitive() = default;

        virtual double getIntersection(const Ray &ray) const = 0;
        virtual bool hits(const Ray &ray) const = 0;
        virtual Math::Vector3D getNormal(const Math::Vector3D &point) const = 0;

        virtual AABB getAABB() const = 0;

        virtual void setMaterial(std::shared_ptr<IMaterial> material) = 0;
        virtual std::shared_ptr<IMaterial> getMaterial() const = 0;

        virtual void applyTranslation(const Math::Vector3D &translation) = 0;
        virtual void applyRotation(const Math::Vector3D &angles) = 0;

        virtual std::string getDebugInfo() const = 0;
};

#endif /* !IPRIMITIVE_HPP_ */
