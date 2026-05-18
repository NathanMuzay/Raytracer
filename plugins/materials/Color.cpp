/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Color
*/

#include <libconfig.h++>
#include <sstream>

#include "../../src/Interfaces/IMaterial.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class Color : public IMaterial {
    public:
        Math::Vector3D _color;
        double _reflectivity;
        double _transparency;
        double _refractiveIndex;

        Color(const libconfig::Setting &config)
        {
            double r = LibconfigHelper::getNumeric(config["color"], "r");
            double g = LibconfigHelper::getNumeric(config["color"], "g");
            double b = LibconfigHelper::getNumeric(config["color"], "b");
            _color = Math::Vector3D(r, g, b);

            _reflectivity    = 0.0;
            _transparency    = 0.0;
            _refractiveIndex = 1.0;

            if (config.exists("reflectivity"))
                _reflectivity = LibconfigHelper::getNumeric(config, "reflectivity");
            if (config.exists("transparency"))
                _transparency = LibconfigHelper::getNumeric(config, "transparency");
            if (config.exists("refractiveIndex"))
                _refractiveIndex = LibconfigHelper::getNumeric(config, "refractiveIndex");
        }

        Math::Vector3D getColor(const Math::Vector3D &point) const override
        {
            (void)point;
            return _color;
        }

        double getReflectivity() const override { return _reflectivity; }
        double getTransparency()    const override { return _transparency; }
        double getRefractiveIndex() const override { return _refractiveIndex; }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "Color(r=" << _color.x << " g=" << _color.y << " b=" << _color.z
                << " reflectivity=" << _reflectivity << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "Color";
}

extern "C" IMaterial *createPlugin(const libconfig::Setting &config)
{
    return new Color(config);
}
