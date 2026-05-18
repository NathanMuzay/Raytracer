/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** AFactory
*/

#ifndef AFACTORY_HPP_
#define AFACTORY_HPP_

#include <vector>
#include <memory>
#include <filesystem>

#include "PluginLoader.hpp"

template <typename T>
class AFactory {
    public:
        virtual ~AFactory()
        {
            for (auto handle : _handles){
                libdl::dlclose(handle);
            }
            _handles.clear();
        }

        virtual std::vector<std::shared_ptr<T>> loadPlugins(const std::string &path, const libconfig::Setting &config) {return {};};

    protected:
        std::vector<void *> _handles;
};

#endif /* !AFACTORY_HPP_ */
