/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** PrimitiveFactory
*/

#ifndef PRIMITIVEFACTORY_HPP_
#define PRIMITIVEFACTORY_HPP_

#include <iostream>

#include "AFactory.hpp"
#include "../Interfaces/IPrimitive.hpp"
#include "MaterialFactory.hpp"

#include "../Parser/LibconfigHelper.hpp"
#include "../Math/Math.hpp"

class PrimitiveFactory : public AFactory<IPrimitive> {
    public:
        std::vector<std::shared_ptr<IPrimitive>> loadPlugins(
            const std::string &path, 
            const libconfig::Setting &config, 
            MaterialFactory &matFactory,
            const std::string &mPath) 
        {
            std::vector<std::shared_ptr<IPrimitive>> primitives;

            try {
                for (const auto &entry : std::filesystem::directory_iterator(path)) {
                    if (entry.path().extension() == ".so"){
                        PluginLoader<IPrimitive> loader(entry.path().string());
                        _handles.push_back(loader.getHandle()); 
                        
                        std::string section = loader.getSection();
                        if (!config.exists(section.c_str())) 
                            continue;
                        const libconfig::Setting &list = config[section.c_str()];
                        for (int i = 0; i < list.getLength(); i++){
                            auto primitive = std::shared_ptr<IPrimitive>(loader.getInstance(list[i]));
                            applyTransformations(primitive, list[i]);
                            applyMaterial(primitive, list[i], matFactory, mPath);
                            std::cout << "[Factory] " << entry.path().string() << " loaded " << primitive->getDebugInfo() << std::endl;
                            primitives.push_back(std::move(primitive));
                        }
                    }
                }
            } catch (const std::exception &e){
                throw PrimitiveFactoryException(e.what());
            }
            return primitives;
        }

    private:
        void applyTransformations(std::shared_ptr<IPrimitive> &primitive, const libconfig::Setting &config)
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

        void applyMaterial(
            std::shared_ptr<IPrimitive> &primitive, 
            const libconfig::Setting &config, 
            MaterialFactory &matFactory,
            const std::string &path)
        {
            if (config.exists("material")) {
                auto mat = matFactory.createFromConfig(path, config["material"]);
                primitive->setMaterial(mat);
            }
        }
};

#endif /* !PRIMITIVEFACTORY_HPP_ */
