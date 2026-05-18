/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** BVH - Bounding Volume Hierarchy
*/

#ifndef BVH_HPP_
#define BVH_HPP_

#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include <array>

#include "AABB.hpp"
#include "../Interfaces/IPrimitive.hpp"
#include "../Camera/Ray.hpp"

// ── BVHNode ───────────────────────────────────────────────────────────────────
// Chaque nœud contient :
//   - son AABB
//   - soit deux fils (nœud interne)
//   - soit une primitive (feuille)
struct BVHNode {
    AABB box;

    // Nœud interne
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;

    // Feuille
    IPrimitive *primitive = nullptr;

    bool isLeaf() const { return primitive != nullptr; }
};

// ── BVH ───────────────────────────────────────────────────────────────────────
class BVH {
public:
    BVH() = default;

    // Deep copy — used to share the BVH with the preview scene without rebuilding
    BVH(const BVH &other)
    {
        if (other._root)
            _root = copyNode(other._root.get());
    }
    BVH &operator=(const BVH &other)
    {
        if (this != &other)
            _root = other._root ? copyNode(other._root.get()) : nullptr;
        return *this;
    }
    BVH(BVH &&) = default;
    BVH &operator=(BVH &&) = default;
    // Construit le BVH à partir d'un vecteur de shared_ptr<IPrimitive>
    void build(const std::vector<std::shared_ptr<IPrimitive>> &primitives)
    {
        if (primitives.empty()) {
            _root = nullptr;
            return;
        }
        // Copie des raw ptrs pour le tri (les shared_ptr restent owner dans Scene)
        std::vector<IPrimitive *> ptrs;
        ptrs.reserve(primitives.size());
        for (auto &p : primitives)
            ptrs.push_back(p.get());

        _root = buildNode(ptrs, 0, static_cast<int>(ptrs.size()));
    }

    // ── Intersection principale ───────────────────────────────────────────────
    // Retourne la primitive la plus proche et sa distance t,
    // ou {nullptr, inf} si aucun hit.
    struct Hit {
        IPrimitive *primitive = nullptr;
        double t = std::numeric_limits<double>::max();
    };

    Hit intersect(const Ray &ray) const
    {
        Hit result;
        if (_root)
            traverse(_root.get(), ray, result);
        return result;
    }

    // ── Test d'ombre ─────────────────────────────────────────────────────────
    // Retourne true si un objet bloque le ray entre 1e-4 et distToLight.
    bool intersectShadow(const Ray &ray, double distToLight) const
    {
        if (!_root) return false;
        return traverseShadow(_root.get(), ray, distToLight);
    }

private:
    std::unique_ptr<BVHNode> _root;

    // ── Deep copy helper ─────────────────────────────────────────────────────
    static std::unique_ptr<BVHNode> copyNode(const BVHNode *src)
    {
        if (!src) return nullptr;
        auto node = std::make_unique<BVHNode>();
        node->box       = src->box;
        node->primitive = src->primitive;   // raw ptr — owned by Scene
        node->left      = copyNode(src->left.get());
        node->right     = copyNode(src->right.get());
        return node;
    }

    // ── Construction récursive ────────────────────────────────────────────────
    std::unique_ptr<BVHNode> buildNode(std::vector<IPrimitive *> &prims, int start, int end)
    {
        auto node = std::make_unique<BVHNode>();

        // Compute AABB of all primitives in this node
        AABB combined;
        for (int i = start; i < end; i++)
            combined = AABB::merge(combined, prims[i]->getAABB());
        node->box = combined;

        int count = end - start;

        // Leaf: single primitive
        if (count == 1) {
            node->primitive = prims[start];
            return node;
        }

        // Leaf: two primitives — skip SAH, just split directly
        if (count == 2) {
            node->left  = buildNode(prims, start,     start + 1);
            node->right = buildNode(prims, start + 1, end);
            return node;
        }

        // ── SAH split ────────────────────────────────────────────────────────
        // Try all 3 axes, pick the (axis, split) pair with minimum SAH cost.
        // Cost = N_left * SA_left + N_right * SA_right  (parent SA normalised to 1)
        const int SAH_BINS = 12;

        double bestCost = std::numeric_limits<double>::max();
        int    bestAxis = 0;
        int    bestMid  = start + count / 2; // fallback

        auto surfaceArea = [](const AABB &b) -> double {
            double dx = b.max.x - b.min.x;
            double dy = b.max.y - b.min.y;
            double dz = b.max.z - b.min.z;
            return 2.0 * (dx * dy + dy * dz + dz * dx);
        };

        double parentSA = surfaceArea(combined);
        if (parentSA <= 0.0) parentSA = 1.0; // degenerate box guard

        for (int axis = 0; axis < 3; axis++) {
            // Compute centroid range on this axis
            double cMin = std::numeric_limits<double>::max();
            double cMax = -std::numeric_limits<double>::max();
            for (int i = start; i < end; i++) {
                double c = (axis == 0) ? prims[i]->getAABB().centroid().x
                         : (axis == 1) ? prims[i]->getAABB().centroid().y
                                       : prims[i]->getAABB().centroid().z;
                cMin = std::min(cMin, c);
                cMax = std::max(cMax, c);
            }
            if (cMax - cMin < 1e-10) continue; // all centroids identical on this axis

            // Bin each primitive
            struct Bin { AABB box; int count = 0; };
            std::array<Bin, SAH_BINS> bins;
            double invRange = SAH_BINS / (cMax - cMin);

            for (int i = start; i < end; i++) {
                double c = (axis == 0) ? prims[i]->getAABB().centroid().x
                         : (axis == 1) ? prims[i]->getAABB().centroid().y
                                       : prims[i]->getAABB().centroid().z;
                int b = static_cast<int>((c - cMin) * invRange);
                if (b >= SAH_BINS) b = SAH_BINS - 1;
                bins[b].box = AABB::merge(bins[b].box, prims[i]->getAABB());
                bins[b].count++;
            }

            // Evaluate SAH cost for each of the (SAH_BINS - 1) candidate splits
            for (int split = 1; split < SAH_BINS; split++) {
                AABB leftBox, rightBox;
                int  leftCount = 0, rightCount = 0;
                for (int b = 0;     b < split;    b++) { leftBox  = AABB::merge(leftBox,  bins[b].box); leftCount  += bins[b].count; }
                for (int b = split; b < SAH_BINS; b++) { rightBox = AABB::merge(rightBox, bins[b].box); rightCount += bins[b].count; }
                if (leftCount == 0 || rightCount == 0) continue;

                double cost = (leftCount  * surfaceArea(leftBox) +
                               rightCount * surfaceArea(rightBox)) / parentSA;
                if (cost < bestCost) {
                    bestCost = cost;
                    bestAxis = axis;
                    // Compute the actual centroid threshold for this split
                    double threshold = cMin + split * (cMax - cMin) / SAH_BINS;
                    // Partition into bestMid
                    auto pivot = std::partition(prims.begin() + start, prims.begin() + end,
                        [axis, threshold](IPrimitive *p) {
                            auto c = p->getAABB().centroid();
                            return (axis == 0 ? c.x : axis == 1 ? c.y : c.z) < threshold;
                        });
                    bestMid = static_cast<int>(pivot - prims.begin());
                    // Guard: never produce an empty partition
                    if (bestMid == start) bestMid = start + 1;
                    if (bestMid == end)   bestMid = end   - 1;
                }
            }
        }

        // If SAH didn't find a good split (all axes degenerate), fall back to median
        if (bestCost == std::numeric_limits<double>::max()) {
            double dx = combined.max.x - combined.min.x;
            double dy = combined.max.y - combined.min.y;
            double dz = combined.max.z - combined.min.z;
            bestAxis = 0;
            if (dy > dx) bestAxis = 1;
            if (dz > (bestAxis == 0 ? dx : dy)) bestAxis = 2;

            std::sort(prims.begin() + start, prims.begin() + end,
                [bestAxis](IPrimitive *a, IPrimitive *b) {
                    auto ca = a->getAABB().centroid();
                    auto cb = b->getAABB().centroid();
                    if (bestAxis == 0) return ca.x < cb.x;
                    if (bestAxis == 1) return ca.y < cb.y;
                    return ca.z < cb.z;
                }
            );
            bestMid = start + count / 2;
        }

        node->left  = buildNode(prims, start,   bestMid);
        node->right = buildNode(prims, bestMid, end);
        return node;
    }

    // ── Traversée pour le ray primaire ───────────────────────────────────────
    void traverse(const BVHNode *node, const Ray &ray, Hit &best) const
    {
        if (!node->box.intersects(ray, 1e-4, best.t))
            return;

        if (node->isLeaf()) {
            double t = node->primitive->getIntersection(ray);
            if (t > 1e-4 && t < best.t) {
                best.t = t;
                best.primitive = node->primitive;
            }
            return;
        }

        traverse(node->left.get(),  ray, best);
        traverse(node->right.get(), ray, best);
    }

    // ── Traversée pour les shadow rays ───────────────────────────────────────
    bool traverseShadow(const BVHNode *node, const Ray &ray, double distToLight) const
    {
        if (!node->box.intersects(ray, 1e-4, distToLight))
            return false;

        if (node->isLeaf()) {
            double t = node->primitive->getIntersection(ray);
            return (t > 1e-4 && t < distToLight);
        }

        return traverseShadow(node->left.get(),  ray, distToLight)
            || traverseShadow(node->right.get(), ray, distToLight);
    }
};

#endif /* !BVH_HPP_ */
