/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** PrimitiveFactory
*/

#ifndef PRIMITIVEFACTORY_HPP_
#define PRIMITIVEFACTORY_HPP_

#include <iostream>
#include <filesystem>
#include <cmath>
#include <algorithm>

#include "AFactory.hpp"
#include "../Interfaces/IPrimitive.hpp"
#include "../Interfaces/EmissiveLight.hpp"
#include "MaterialFactory.hpp"

#include "../Parser/LibconfigHelper.hpp"
#include "../Math/Math.hpp"

class PrimitiveFactory : public AFactory<IPrimitive> {
    public:
        /**
         * Charge tous les plugins .so du dossier @path.
         * - Si une primitive a "emit = true" dans sa config, elle est marquée
         *   émissive ET une (ou plusieurs) EmissiveLight sont poussées dans
         *   @outLights pour l'éclairage direct.
         * - Le paramètre @outLights est optionnel : si nullptr est passé les
         *   lumières émissives sont simplement ignorées (comportement ancien).
         */
        std::vector<std::shared_ptr<IPrimitive>> loadPlugins(
            const std::string &path,
            const libconfig::Setting &config,
            MaterialFactory &matFactory,
            const std::string &mPath,
            std::vector<std::shared_ptr<ILight>> *outLights)
        {
            std::vector<std::shared_ptr<IPrimitive>> primitives;

            try {
                for (const auto &entry : std::filesystem::directory_iterator(path)) {
                    if (entry.path().extension() != ".so")
                        continue;

                    PluginLoader<IPrimitive> loader(entry.path().string());
                    _handles.push_back(loader.getHandle());

                    std::string section = loader.getSection();
                    if (!config.exists(section.c_str()))
                        continue;

                    const libconfig::Setting &list = config[section.c_str()];
                    for (int i = 0; i < list.getLength(); i++) {
                        auto primitive = std::shared_ptr<IPrimitive>(loader.getInstance(list[i]));
                        applyTransformations(primitive, list[i]);
                        applyMaterial(primitive, list[i], matFactory, mPath);

                        // ── Emission ─────────────────────────────────────────
                        if (outLights)
                            applyEmission(primitive, list[i], section, *outLights);

                        std::cout << "[Factory] " << entry.path().string()
                                  << " loaded " << primitive->getDebugInfo() << std::endl;
                        primitives.push_back(std::move(primitive));
                    }
                }
            } catch (const std::exception &e) {
                throw PrimitiveFactoryException(e.what());
            }
            return primitives;
        }

        // Surcharge de compatibilité avec l'ancienne signature (sans outLights)
        std::vector<std::shared_ptr<IPrimitive>> loadPlugins(
            const std::string &path,
            const libconfig::Setting &config,
            MaterialFactory &matFactory,
            const std::string &mPath)
        {
            return loadPlugins(path, config, matFactory, mPath, nullptr);
        }

    private:
        // ── Transformations ──────────────────────────────────────────────────
        void applyTransformations(std::shared_ptr<IPrimitive> &primitive,
                                   const libconfig::Setting &config)
        {
            if (config.exists("translation")) {
                double tx = LibconfigHelper::getNumeric(config["translation"], "x");
                double ty = LibconfigHelper::getNumeric(config["translation"], "y");
                double tz = LibconfigHelper::getNumeric(config["translation"], "z");
                primitive->applyTranslation(Math::Vector3D(tx, ty, tz));
            }
            if (config.exists("rotation")) {
                double rx = LibconfigHelper::getNumeric(config["rotation"], "x");
                double ry = LibconfigHelper::getNumeric(config["rotation"], "y");
                double rz = LibconfigHelper::getNumeric(config["rotation"], "z");
                primitive->applyRotation(Math::Vector3D(rx, ry, rz));
            }
        }

        // ── Material ─────────────────────────────────────────────────────────
        void applyMaterial(std::shared_ptr<IPrimitive> &primitive,
                            const libconfig::Setting &config,
                            MaterialFactory &matFactory,
                            const std::string &path)
        {
            if (config.exists("material")) {
                auto mat = matFactory.createFromConfig(path, config["material"]);
                primitive->setMaterial(mat);
            }
        }

        // ── Emission ─────────────────────────────────────────────────────────
        void applyEmission(std::shared_ptr<IPrimitive> &primitive,
                            const libconfig::Setting &cfg,
                            const std::string &sectionName,
                            std::vector<std::shared_ptr<ILight>> &outLights)
        {
            try {
                if (!cfg.exists("emit"))
                    return;
                if (cfg["emit"].getType() != libconfig::Setting::TypeBoolean)
                    return;
                if (!cfg["emit"].operator bool())
                    return;

                double intensity = 1.0;
                if (cfg.exists("emit_intensity"))
                    intensity = LibconfigHelper::getNumeric(cfg, "emit_intensity");

                // Marque la primitive comme émissive (rendu self-lit)
                primitive->setEmissive(true, intensity);

                // Couleur normalisée [0..1]
                Math::Vector3D matColor = primitive->getColor(Math::Vector3D(0, 0, 0));
                Math::Vector3D colorNorm(matColor.x / 255.0,
                                          matColor.y / 255.0,
                                          matColor.z / 255.0);

                // Position de la primitive
                double lx = cfg.exists("x") ? LibconfigHelper::getNumeric(cfg, "x") : 0.0;
                double ly = cfg.exists("y") ? LibconfigHelper::getNumeric(cfg, "y") : 0.0;
                double lz = cfg.exists("z") ? LibconfigHelper::getNumeric(cfg, "z") : 0.0;

                double surfaceRadius = 0.0;

                // ── Sphère ───────────────────────────────────────────────────
                if (sectionName == "spheres" && cfg.exists("r")) {
                    surfaceRadius = LibconfigHelper::getNumeric(cfg, "r");
                    outLights.push_back(std::make_shared<EmissiveLight>(
                        Math::Vector3D(lx, ly, lz),
                        (float)intensity, colorNorm, 0.0, surfaceRadius));

                // ── Cube ─────────────────────────────────────────────────────
                } else if (sectionName == "cubes" &&
                           cfg.exists("sizeX") && cfg.exists("sizeY") && cfg.exists("sizeZ")) {

                    double sx = LibconfigHelper::getNumeric(cfg, "sizeX");
                    double sy = LibconfigHelper::getNumeric(cfg, "sizeY");
                    double sz = LibconfigHelper::getNumeric(cfg, "sizeZ");
                    surfaceRadius = std::max({sx, sy, sz}) * 0.45;

                    double rx = 0.0, ry = 0.0, rz = 0.0;
                    if (cfg.exists("rotation")) {
                        rx = LibconfigHelper::getNumeric(cfg["rotation"], "x");
                        ry = LibconfigHelper::getNumeric(cfg["rotation"], "y");
                        rz = LibconfigHelper::getNumeric(cfg["rotation"], "z");
                    }

                    auto rotateVec = [&](const Math::Vector3D &v) -> Math::Vector3D {
                        double rxd = rx * M_PI / 180.0;
                        double ryd = ry * M_PI / 180.0;
                        double rzd = rz * M_PI / 180.0;
                        double cosRx = std::cos(rxd), sinRx = std::sin(rxd);
                        Math::Vector3D v1(v.x, v.y * cosRx - v.z * sinRx,
                                                v.y * sinRx + v.z * cosRx);
                        double cosRy = std::cos(ryd), sinRy = std::sin(ryd);
                        Math::Vector3D v2(v1.x * cosRy + v1.z * sinRy, v1.y,
                                         -v1.x * sinRy + v1.z * cosRy);
                        double cosRz = std::cos(rzd), sinRz = std::sin(rzd);
                        return Math::Vector3D(v2.x * cosRz - v2.y * sinRz,
                                              v2.x * sinRz + v2.y * cosRz,
                                              v2.z);
                    };

                    auto addFace = [&](double nx, double ny, double nz) {
                        Math::Vector3D local(nx * (sx / 2.0 + 0.05),
                                              ny * (sy / 2.0 + 0.05),
                                              nz * (sz / 2.0 + 0.05));
                        Math::Vector3D rotated = rotateVec(local);
                        Math::Vector3D pos(lx + rotated.x, ly + rotated.y, lz + rotated.z);
                        outLights.push_back(std::make_shared<EmissiveLight>(
                            pos, (float)(intensity / 7.0), colorNorm, 0.0, surfaceRadius));
                    };

                    addFace( 1,  0,  0);
                    addFace(-1,  0,  0);
                    addFace( 0,  1,  0);
                    addFace( 0, -1,  0);
                    addFace( 0,  0,  1);
                    addFace( 0,  0, -1);

                // ── Cylindre ─────────────────────────────────────────────────
                } else if ((sectionName == "cylindres" || sectionName == "cylinders") &&
                            cfg.exists("r") && cfg.exists("h")) {
                    double r = LibconfigHelper::getNumeric(cfg, "r");
                    double h = LibconfigHelper::getNumeric(cfg, "h");
                    surfaceRadius = std::max(r, h * 0.5);
                    outLights.push_back(std::make_shared<EmissiveLight>(
                        Math::Vector3D(lx, ly, lz),
                        (float)intensity, colorNorm, 0.0, surfaceRadius));

                // ── Cône ─────────────────────────────────────────────────────
                } else if (sectionName == "cones" && cfg.exists("r") && cfg.exists("h")) {
                    double r = LibconfigHelper::getNumeric(cfg, "r");
                    double h = LibconfigHelper::getNumeric(cfg, "h");
                    surfaceRadius = std::max(r, h * 0.5);
                    outLights.push_back(std::make_shared<EmissiveLight>(
                        Math::Vector3D(lx, ly, lz),
                        (float)intensity, colorNorm, 0.0, surfaceRadius));

                // ── Autre forme ───────────────────────────────────────────────
                } else {
                    outLights.push_back(std::make_shared<EmissiveLight>(
                        Math::Vector3D(lx, ly, lz),
                        (float)intensity, colorNorm, 0.0, 0.0));
                }

            } catch (...) {
                // Ne pas planter si la config d'émission est mal formée
            }
        }
};

#endif /* !PRIMITIVEFACTORY_HPP_ */
