/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** LightFactory
*/

#ifndef LIGHTFACTORY_HPP_
#define LIGHTFACTORY_HPP_

#include <iostream>

#include "AFactory.hpp"
#include "../Interfaces/ILight.hpp"

class LightFactory : public AFactory<ILight> {
    public:
        std::vector<std::shared_ptr<ILight>> loadPlugins(const std::string &path, const libconfig::Setting &config) override
        {
            std::vector<std::shared_ptr<ILight>> lights;

            try {
                for (const auto &entry : std::filesystem::directory_iterator(path)) {
                    if (entry.path().extension() == ".so"){
                        PluginLoader<ILight> loader(entry.path().string());
                        _handles.push_back(loader.getHandle());

                        std::string section = loader.getSection();
                        if (!config.exists(section.c_str()))
                            continue;
                        const libconfig::Setting &list = config[section.c_str()];
                        for (int i = 0; i < list.getLength(); i++){
                            auto light = std::shared_ptr<ILight>(loader.getInstance(list[i]));
                            std::cout << "[Factory] " << entry.path().string() << " loaded " << light->getDebugInfo() << std::endl;
                            lights.push_back(std::move(light));
                        }
                    }
                }
            } catch (const std::exception &e){
                throw LightFactoryException(e.what());
            }
            return lights;
        }
};

#endif /* !LIGHTFACTORY_HPP_ */