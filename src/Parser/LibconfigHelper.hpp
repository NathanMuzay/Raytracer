/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** LibconfigHelper
*/

#ifndef LIBCONFIG_HELPER_HPP_
#define LIBCONFIG_HELPER_HPP_

#include <libconfig.h++>

namespace LibconfigHelper {
    inline double getNumeric(const libconfig::Setting &setting, const char *name)
    {
        double value = 0.0;
        int intValue = 0;

        if (setting.lookupValue(name, value))
            return value;
        if (setting.lookupValue(name, intValue))
            return static_cast<double>(intValue);
        return 0.0;
    }

    inline int getInt(const libconfig::Setting &setting, const char *name)
    {
        int value = 0;
        double doubleValue = 0.0;

        if (setting.lookupValue(name, value))
            return value;
        if (setting.lookupValue(name, doubleValue))
            return static_cast<int>(doubleValue);
        return 0;
    }
}

#endif /* !LIBCONFIG_HELPER_HPP_ */
