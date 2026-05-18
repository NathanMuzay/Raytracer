/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** MaterialFactory
*/

#ifndef MATERIALFACTORY_HPP_
#define MATERIALFACTORY_HPP_

#include <iostream>

#include "AFactory.hpp"
#include "../Interfaces/IMaterial.hpp"

class MaterialFactory : public AFactory<IMaterial> {
    public:
        std::shared_ptr<IMaterial> createFromConfig(const std::string &path, const libconfig::Setting &config)
        {
            std::string type = (const char *)config["type"];

            try {
                for (const auto &entry : std::filesystem::directory_iterator(path)) {
                    if (entry.path().extension() != ".so")
                        continue;
                    PluginLoader<IMaterial> loader(entry.path().string());
                    _handles.push_back(loader.getHandle());
                    std::string section = loader.getSection();
                    if (section != type)
                        continue;
                    return std::shared_ptr<IMaterial>(loader.getInstance(config));
                }
            } catch (const std::exception &e){
                throw MaterialFactoryException(e.what());
            }
            throw MaterialFactoryException("No material plugin found for type: " + type);
        }
};

#endif /* !MATERIALFACTORY_HPP_ */
