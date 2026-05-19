/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Renderer
*/

#include "Renderer.hpp"
#include "../Interfaces/EmissiveLight.hpp"
#include <cstring>

static const int MAX_DEPTH = 5;

void Renderer::write_color(std::ofstream &file, const Math::Vector3D &color)
{
    file << (int)color.x << " "
         << (int)color.y << " "
         << (int)color.z << "\n";
}

Math::Vector3D Renderer::clamp(const Math::Vector3D &color)
{
    return Math::Vector3D(
        std::min(255.0, std::max(0.0, color.x)),
        std::min(255.0, std::max(0.0, color.y)),
        std::min(255.0, std::max(0.0, color.z))
    );
}

// ── getShadowFactor ──────────────────────────────────────────────────────────
// Retourne 1.0 si le point est pleinement éclairé, 0.0 si complètement dans
// l'ombre, et une valeur intermédiaire pour les lumières de surface (soft shadows).
// Utilise le BVH pour accélérer les requêtes d'ombre.
double Renderer::getShadowFactor(const Math::Vector3D &hitPoint,
                                  ILight *light,
                                  const Scene &scene)
{
    EmissiveLight *em = dynamic_cast<EmissiveLight*>(light);
    double radius = em ? em->getSurfaceRadius() : 0.0;

    Math::Vector3D lightDir  = light->getLightDir(hitPoint);
    double distToLight        = light->getLightDist(hitPoint);

    if (radius <= 0.0) {
        // Lumière ponctuelle — occlusion exacte via BVH
        Ray shadowRay(
            Math::Point3D(
                hitPoint.x + lightDir.x * 1e-4,
                hitPoint.y + lightDir.y * 1e-4,
                hitPoint.z + lightDir.z * 1e-4
            ),
            lightDir
        );
        auto hit = scene.bvh.intersect(shadowRay);
        if (!hit.primitive || hit.t >= distToLight - 1e-3 || hit.primitive->isEmissive())
            return 1.0;
        auto mat = hit.primitive->getMaterial();
        if (mat && mat->getTransparency() > 0.5)
            return 1.0;  // transparent → pas d'ombre
        return 0.0;
    }

    // Lumière de surface (area light) — soft shadows par échantillonnage
    int samples = (scene.antialiasingSamples > 1) ? 4 : 16;

    Math::Vector3D up = (std::fabs(lightDir.x) < 0.9)
                        ? Math::Vector3D(1, 0, 0)
                        : Math::Vector3D(0, 1, 0);
    Math::Vector3D tangent(
        up.y * lightDir.z - up.z * lightDir.y,
        up.z * lightDir.x - up.x * lightDir.z,
        up.x * lightDir.y - up.y * lightDir.x
    );
    tangent = tangent.normalize();
    Math::Vector3D bitangent(
        lightDir.y * tangent.z - lightDir.z * tangent.y,
        lightDir.z * tangent.x - lightDir.x * tangent.z,
        lightDir.x * tangent.y - lightDir.y * tangent.x
    );
    bitangent = bitangent.normalize();

    std::mt19937 rng(static_cast<uint32_t>(
        hitPoint.x * 123 + hitPoint.y * 456 + hitPoint.z * 789));
    std::uniform_real_distribution<double> distRand(-1.0, 1.0);

    Math::Vector3D centerPos(
        hitPoint.x + lightDir.x * distToLight,
        hitPoint.y + lightDir.y * distToLight,
        hitPoint.z + lightDir.z * distToLight
    );

    int visible = 0;
    for (int i = 0; i < samples; i++) {
        double u, v;
        do {
            u = distRand(rng);
            v = distRand(rng);
        } while (u * u + v * v > 1.0);

        Math::Vector3D samplePos(
            centerPos.x + tangent.x * (u * radius) + bitangent.x * (v * radius),
            centerPos.y + tangent.y * (u * radius) + bitangent.y * (v * radius),
            centerPos.z + tangent.z * (u * radius) + bitangent.z * (v * radius)
        );
        Math::Vector3D dir(
            samplePos.x - hitPoint.x,
            samplePos.y - hitPoint.y,
            samplePos.z - hitPoint.z
        );
        double sampleDist = dir.length();
        dir = dir.normalize();

        Ray shadowRay(
            Math::Point3D(
                hitPoint.x + dir.x * 1e-4,
                hitPoint.y + dir.y * 1e-4,
                hitPoint.z + dir.z * 1e-4
            ),
            dir
        );
        // Utilise le BVH pour accélérer les rayons d'ombre de surface
        auto hit = scene.bvh.intersect(shadowRay);
        if (!hit.primitive || hit.t >= sampleDist - 1e-3 || hit.primitive->isEmissive())
            visible++;
        else {
            auto mat = hit.primitive->getMaterial();
            if (mat && mat->getTransparency() > 0.5)
                visible++;
        }
    }
    return (double)visible / (double)samples;
}

// ── computeAO ────────────────────────────────────────────────────────────────
double Renderer::computeAO(const Math::Vector3D &point,
                            const Math::Vector3D &normal,
                            const Scene &scene) const
{
    const AOSettings &ao = scene.ao;

    Math::Vector3D up = (std::fabs(normal.x) < 0.9)
                        ? Math::Vector3D(1, 0, 0)
                        : Math::Vector3D(0, 1, 0);

    Math::Vector3D tangent   = up.cross(normal).normalize();
    Math::Vector3D bitangent = normal.cross(tangent).normalize();

    // Seed robuste (évite les bandes sur les surfaces plates)
    auto hashDouble = [](double val) -> uint64_t {
        uint64_t bits;
        std::memcpy(&bits, &val, sizeof(bits));
        bits ^= bits >> 33;
        bits *= 0xff51afd7ed558ccdULL;
        bits ^= bits >> 33;
        return bits;
    };
    uint32_t seed = static_cast<uint32_t>(
        hashDouble(point.x) ^
        (hashDouble(point.y) * 2654435761ULL) ^
        (hashDouble(point.z) * 2246822519ULL)
    );
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    int unoccluded = 0;
    for (int i = 0; i < ao.samples; ++i) {
        double u1   = dist(rng);
        double u2   = dist(rng);
        double phi  = 2.0 * M_PI * u2;
        double sinT = std::sqrt(u1);
        double cosT = std::sqrt(1.0 - u1);

        Math::Vector3D dir(
            sinT * std::cos(phi) * tangent.x   + sinT * std::sin(phi) * bitangent.x   + cosT * normal.x,
            sinT * std::cos(phi) * tangent.y   + sinT * std::sin(phi) * bitangent.y   + cosT * normal.y,
            sinT * std::cos(phi) * tangent.z   + sinT * std::sin(phi) * bitangent.z   + cosT * normal.z
        );
        dir = dir.normalize();

        Ray aoRay(
            Math::Point3D(
                point.x + normal.x * 0.5,
                point.y + normal.y * 0.5,
                point.z + normal.z * 0.5
            ),
            dir
        );
        auto hit = scene.bvh.intersect(aoRay);
        if (!(hit.primitive && hit.t < ao.radius))
            unoccluded++;
    }
    return static_cast<double>(unoccluded) / static_cast<double>(ao.samples);
}

// ── computeColor ─────────────────────────────────────────────────────────────
Math::Vector3D Renderer::computeColor(const Ray &ray, const Scene &scene, int depth)
{
    // ── Intersection via BVH ─────────────────────────────────────────────────
    auto hit = scene.bvh.intersect(ray);
    if (!hit.primitive)
        return scene.backgroundColor;

    const IPrimitive *closest = hit.primitive;
    double minT = hit.t;

    // ── Données géométriques ─────────────────────────────────────────────────
    Math::Vector3D hitPoint(
        ray.origin.x + ray.direction.x * minT,
        ray.origin.y + ray.direction.y * minT,
        ray.origin.z + ray.direction.z * minT
    );
    Math::Vector3D normal  = closest->getNormal(hitPoint);
    Math::Vector3D viewDir = ray.direction.normalize();

    auto mat = closest->getMaterial();
    Math::Vector3D baseColor = mat ? mat->getColor(hitPoint) : Math::Vector3D(255, 255, 255);
    Math::Vector3D base01(baseColor.x / 255.0, baseColor.y / 255.0, baseColor.z / 255.0);

    // ── Mode rapide ───────────────────────────────────────────────────────────
    if (scene.fastMode) {
        double fakeDiffuse = std::max(0.0, normal.dot(Math::Vector3D(0.577, 0.577, 0.577)));
        return clamp(Math::Vector3D(
            (base01.x * (scene.ambientLight + fakeDiffuse * 0.7)) * 255.0,
            (base01.y * (scene.ambientLight + fakeDiffuse * 0.7)) * 255.0,
            (base01.z * (scene.ambientLight + fakeDiffuse * 0.7)) * 255.0
        ));
    }

    // ── Ambient + AO ─────────────────────────────────────────────────────────
    double aoFactor = scene.ao.enabled
        ? 1.0 - scene.ao.intensity * (1.0 - computeAO(hitPoint, normal, scene))
        : 1.0;

    Math::Vector3D result(
        base01.x * scene.ambientLight * aoFactor,
        base01.y * scene.ambientLight * aoFactor,
        base01.z * scene.ambientLight * aoFactor
    );

    // ── Primitive émissive : s'affiche à pleine luminosité ───────────────────
    if (closest->isEmissive()) {
        result.x = base01.x * closest->getEmissionIntensity();
        result.y = base01.y * closest->getEmissionIntensity();
        result.z = base01.z * closest->getEmissionIntensity();
    }

    // ── Éclairage direct ─────────────────────────────────────────────────────
    for (auto &light : scene.lights) {
        EmissiveLight *em = dynamic_cast<EmissiveLight*>(light.get());

        if (em && em->getSurfaceRadius() > 0.0) {
            // ── Area light : grille d'échantillons + shadow par BVH ──────────
            double radius    = em->getSurfaceRadius();
            int gridSteps    = (scene.antialiasingSamples > 1) ? 2 : 5;
            int totalSamples = gridSteps * gridSteps;

            Math::Vector3D centerPos = em->getPosition();
            Math::Vector3D lightDir  = light->getLightDir(hitPoint);

            Math::Vector3D up = (std::fabs(lightDir.x) < 0.9)
                                ? Math::Vector3D(1, 0, 0)
                                : Math::Vector3D(0, 1, 0);
            Math::Vector3D tangent(
                up.y * lightDir.z - up.z * lightDir.y,
                up.z * lightDir.x - up.x * lightDir.z,
                up.x * lightDir.y - up.y * lightDir.x
            );
            tangent = tangent.normalize();
            Math::Vector3D bitangent(
                lightDir.y * tangent.z - lightDir.z * tangent.y,
                lightDir.z * tangent.x - lightDir.x * tangent.z,
                lightDir.x * tangent.y - lightDir.y * tangent.x
            );
            bitangent = bitangent.normalize();

            Math::Vector3D diffuseSum(0, 0, 0);
            Math::Vector3D specularSum(0, 0, 0);

            for (int gi = 0; gi < gridSteps; gi++) {
                for (int gj = 0; gj < gridSteps; gj++) {
                    double u = (double(gi) + 0.5) / gridSteps * 2.0 - 1.0;
                    double v = (double(gj) + 0.5) / gridSteps * 2.0 - 1.0;

                    Math::Vector3D samplePos(
                        centerPos.x + tangent.x * (u * radius) + bitangent.x * (v * radius),
                        centerPos.y + tangent.y * (u * radius) + bitangent.y * (v * radius),
                        centerPos.z + tangent.z * (u * radius) + bitangent.z * (v * radius)
                    );
                    Math::Vector3D sampleDir(
                        samplePos.x - hitPoint.x,
                        samplePos.y - hitPoint.y,
                        samplePos.z - hitPoint.z
                    );
                    double sampleDist = sampleDir.length();
                    sampleDir = sampleDir.normalize();

                    Ray shadowRay(
                        Math::Point3D(
                            hitPoint.x + normal.x * 1e-3,
                            hitPoint.y + normal.y * 1e-3,
                            hitPoint.z + normal.z * 1e-3
                        ),
                        sampleDir
                    );

                    // Test d'ombre via BVH
                    auto sh = scene.bvh.intersect(shadowRay);
                    bool occluded = sh.primitive && sh.t < sampleDist - 1e-3 && !sh.primitive->isEmissive();
                    if (!occluded) {
                        double cosAngle = std::max(0.0, normal.dot(sampleDir));
                        Math::Vector3D diff = em->getColor() * (em->getIntensity() * cosAngle);

                        Math::Vector3D spec(0, 0, 0);
                        if (em->getPhong() > 0.0 && cosAngle > 0.0) {
                            Math::Vector3D reflected(
                                2.0 * cosAngle * normal.x - sampleDir.x,
                                2.0 * cosAngle * normal.y - sampleDir.y,
                                2.0 * cosAngle * normal.z - sampleDir.z
                            );
                            reflected = reflected.normalize();
                            Math::Vector3D toCamera(viewDir.x * -1.0, viewDir.y * -1.0, viewDir.z * -1.0);
                            double rdotv = reflected.dot(toCamera);
                            if (rdotv > 0.0) {
                                double s = std::pow(rdotv, em->getPhong()) * em->getIntensity();
                                spec = Math::Vector3D(s, s, s);
                            }
                        }
                        diffuseSum.x  += diff.x;  diffuseSum.y  += diff.y;  diffuseSum.z  += diff.z;
                        specularSum.x += spec.x;  specularSum.y += spec.y;  specularSum.z += spec.z;
                    }
                }
            }

            diffuseSum  = diffuseSum  / (double)totalSamples;
            specularSum = specularSum / (double)totalSamples;

            result.x += base01.x * diffuseSum.x * scene.diffuseLight + specularSum.x;
            result.y += base01.y * diffuseSum.y * scene.diffuseLight + specularSum.y;
            result.z += base01.z * diffuseSum.z * scene.diffuseLight + specularSum.z;

        } else {
            // ── Lumière ponctuelle classique (avec BVH pour l'ombre) ─────────
            double shadowFactor = getShadowFactor(hitPoint, light.get(), scene);
            if (shadowFactor > 0.0) {
                Math::Vector3D diffuse  = light->computeLight(hitPoint, normal, viewDir);
                Math::Vector3D specular = light->computeSpecular(hitPoint, normal, viewDir);
                result.x += (base01.x * diffuse.x * scene.diffuseLight + specular.x) * shadowFactor;
                result.y += (base01.y * diffuse.y * scene.diffuseLight + specular.y) * shadowFactor;
                result.z += (base01.z * diffuse.z * scene.diffuseLight + specular.z) * shadowFactor;
            }
        }
    }

    // ── Réflexion récursive ───────────────────────────────────────────────────
    double reflectivity = mat ? mat->getReflectivity() : 0.0;
    if (reflectivity > 0.0 && depth < MAX_DEPTH) {
        double dot = viewDir.dot(normal);
        Math::Vector3D reflected(
            viewDir.x - 2.0 * dot * normal.x,
            viewDir.y - 2.0 * dot * normal.y,
            viewDir.z - 2.0 * dot * normal.z
        );
        reflected = reflected.normalize();

        Ray reflectedRay(
            Math::Point3D(
                hitPoint.x + reflected.x * 1e-4,
                hitPoint.y + reflected.y * 1e-4,
                hitPoint.z + reflected.z * 1e-4
            ),
            reflected
        );

        Math::Vector3D reflectedColor255 = computeColor(reflectedRay, scene, depth + 1);
        Math::Vector3D reflectedColor01(
            reflectedColor255.x / 255.0,
            reflectedColor255.y / 255.0,
            reflectedColor255.z / 255.0
        );

        result.x = result.x * (1.0 - reflectivity) + reflectedColor01.x * reflectivity;
        result.y = result.y * (1.0 - reflectivity) + reflectedColor01.y * reflectivity;
        result.z = result.z * (1.0 - reflectivity) + reflectedColor01.z * reflectivity;
    }

    // ── Réfraction ────────────────────────────────────────────────────────────
    double transparency = mat ? mat->getTransparency() : 0.0;
    if (transparency > 0.0 && depth < MAX_DEPTH) {
        double ior  = mat ? mat->getRefractiveIndex() : 1.0;
        double cosI = -normal.dot(viewDir);
        double etaI = 1.0, etaT = ior;
        Math::Vector3D n = normal;

        if (cosI < 0.0) {
            cosI = -cosI;
            n = Math::Vector3D(-normal.x, -normal.y, -normal.z);
            std::swap(etaI, etaT);
        }

        double eta   = etaI / etaT;
        double sinT2 = eta * eta * (1.0 - cosI * cosI);

        if (sinT2 <= 1.0) {
            double cosT = std::sqrt(1.0 - sinT2);
            Math::Vector3D refracted(
                eta * viewDir.x + (eta * cosI - cosT) * n.x,
                eta * viewDir.y + (eta * cosI - cosT) * n.y,
                eta * viewDir.z + (eta * cosI - cosT) * n.z
            );
            refracted = refracted.normalize();

            Ray refractedRay(
                Math::Point3D(
                    hitPoint.x + refracted.x * 1e-3,
                    hitPoint.y + refracted.y * 1e-3,
                    hitPoint.z + refracted.z * 1e-3
                ),
                refracted
            );

            Math::Vector3D refractedColor255 = computeColor(refractedRay, scene, depth + 1);
            Math::Vector3D refractedColor01(
                refractedColor255.x / 255.0,
                refractedColor255.y / 255.0,
                refractedColor255.z / 255.0
            );

            result.x = result.x * (1.0 - transparency) + refractedColor01.x * transparency;
            result.y = result.y * (1.0 - transparency) + refractedColor01.y * transparency;
            result.z = result.z * (1.0 - transparency) + refractedColor01.z * transparency;
        }
    }

    // ── Conversion finale + fog ───────────────────────────────────────────────
    Math::Vector3D finalColor = clamp(Math::Vector3D(
        result.x * 255.0,
        result.y * 255.0,
        result.z * 255.0
    ));

    if (scene.fog.enabled) {
        double fogDist   = std::max(0.0, minT - scene.fog.start);
        double fogFactor = 1.0 - std::exp(-scene.fog.density * fogDist);
        fogFactor = std::min(1.0, std::max(0.0, fogFactor));
        finalColor.x = finalColor.x * (1.0 - fogFactor) + scene.fog.color.x * fogFactor;
        finalColor.y = finalColor.y * (1.0 - fogFactor) + scene.fog.color.y * fogFactor;
        finalColor.z = finalColor.z * (1.0 - fogFactor) + scene.fog.color.z * fogFactor;
        finalColor = clamp(finalColor);
    }

    return finalColor;
}

// ── computePixelColor ────────────────────────────────────────────────────────
Math::Vector3D Renderer::computePixelColor(int x, int y, const Scene &scene,
                                            std::mt19937 &rng,
                                            std::uniform_real_distribution<double> &dist)
{
    Math::Vector3D pixelColor(0, 0, 0);
    int samplesPerSide = scene.antialiasingSamples;
    int totalSamples   = samplesPerSide * samplesPerSide;

    for (int sy = 0; sy < samplesPerSide; sy++) {
        for (int sx = 0; sx < samplesPerSide; sx++) {
            double offsetX = (sx + dist(rng)) / samplesPerSide;
            double offsetY = (sy + dist(rng)) / samplesPerSide;

            double u = ((double)x + offsetX) / scene.width;
            double v = 1.0 - ((double)y + offsetY) / scene.height;
            Ray r = scene.camera.ray(u, v);
            auto color = computeColor(r, scene, 0);
            pixelColor.x += color.x;
            pixelColor.y += color.y;
            pixelColor.z += color.z;
        }
    }

    pixelColor.x /= totalSamples;
    pixelColor.y /= totalSamples;
    pixelColor.z /= totalSamples;
    return pixelColor;
}

// ── render ───────────────────────────────────────────────────────────────────
void Renderer::render(const Scene &scene, const std::string &output,
            std::atomic<bool> &stop,
            std::function<void(int, int, const Math::Vector3D&)> onPixel)
{
    auto startTotal = std::chrono::high_resolution_clock::now();

    std::vector<Math::Vector3D> pixels(scene.width * scene.height);
    unsigned int nThreads = std::thread::hardware_concurrency();
    if (nThreads == 0)
        nThreads = 4;
    std::vector<std::thread> threads;
    int rowsPerThread = scene.height / nThreads;

    auto startRender = std::chrono::high_resolution_clock::now();

    std::mutex pixelMutex;

    for (unsigned int t = 0; t < nThreads; t++) {
        int yStart = t * rowsPerThread;
        int yEnd   = (t == nThreads - 1) ? scene.height : yStart + rowsPerThread;

        threads.emplace_back([&, yStart, yEnd]() {
            std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<double> dist(0.0, 1.0);

            for (int y = yStart; y < yEnd; y++) {
                if (stop) return;
                for (int x = 0; x < scene.width; x++) {
                    Math::Vector3D pixelColor = computePixelColor(x, y, scene, rng, dist);
                    pixels[y * scene.width + x] = pixelColor;
                    if (onPixel) {
                        std::lock_guard<std::mutex> lock(pixelMutex);
                        onPixel(x, y, pixelColor);
                    }
                }
            }
        });
    }
    for (auto &th : threads)
        th.join();

    auto endRender  = std::chrono::high_resolution_clock::now();
    double renderMs = std::chrono::duration<double, std::milli>(endRender - startRender).count();
    auto startWrite = std::chrono::high_resolution_clock::now();

    if (!output.empty()) {
        std::ofstream file(output, std::ios::out | std::ios::trunc);
        if (!file.is_open())
            throw RendererException("Cannot open output file: " + output);
        file << "P3\n" << scene.width << " " << scene.height << "\n255\n";
        for (auto &color : pixels)
            write_color(file, color);
    }

    auto endWrite  = std::chrono::high_resolution_clock::now();
    double writeMs = std::chrono::duration<double, std::milli>(endWrite - startWrite).count();
    double totalMs = std::chrono::duration<double, std::milli>(endWrite - startTotal).count();

    std::cout << "[Renderer] " << nThreads << " threads | "
              << "render: " << renderMs << "ms | "
              << "write: "  << writeMs  << "ms | "
              << "total: "  << totalMs  << "ms"
              << " -> " << output << std::endl;
}
