/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** PluginLoader
*/

#ifndef PLUGINLOADER_HPP_
#define PLUGINLOADER_HPP_

#include <dlfcn.h>
#include <libconfig.h++>

#include "../Exception.hpp"

namespace libdl {
    using ::dlopen;
    using ::dlsym;
    using ::dlerror;
    using ::dlclose;
}

template <typename T>
class PluginLoader {
    public:
        PluginLoader(const std::string &path) : _path(path)
        {
            _handle = libdl::dlopen(path.c_str(), RTLD_LAZY);
            if (!_handle)
                throw PluginLoaderException(libdl::dlerror());
        }
        ~PluginLoader() = default;

        T *getInstance(const libconfig::Setting &config)
        {
            using Module = T* (*)(const libconfig::Setting &config);
            Module module = (Module)libdl::dlsym(_handle, "createPlugin");
            if (!module)
                throw PluginLoaderException(libdl::dlerror());
            return module(config);
        };

        std::string getSection()
        {
            using GetSection = const char *(*)();
            GetSection fn = (GetSection)libdl::dlsym(_handle, "getSectionName");
            if (!fn)
                throw PluginLoaderException(libdl::dlerror());
            return fn();
        }

        void *getHandle() const { return _handle; };

    private:
        std::string _path;
        void *_handle;
};

#endif /* !PLUGINLOADER_HPP_ */
