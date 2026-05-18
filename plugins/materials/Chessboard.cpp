/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Chessboard
*/

#include <libconfig.h++>
#include <sstream>
#include <cmath>

#include "../../src/Interfaces/IMaterial.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class Chessboard : public IMaterial {
    public:
        Math::Vector3D _color1;
        Math::Vector3D _color2;
        double _scale;
        double _reflectivity;
        double _transparency;
        double _refractiveIndex;

        Chessboard(const libconfig::Setting &config)
        {
            if (config.exists("color1")) {
                double r = LibconfigHelper::getNumeric(config["color1"], "r");
                double g = LibconfigHelper::getNumeric(config["color1"], "g");
                double b = LibconfigHelper::getNumeric(config["color1"], "b");
                _color1 = Math::Vector3D(r, g, b);
            } else {
                _color1 = Math::Vector3D(255, 255, 255);
            }

            if (config.exists("color2")) {
                double r = LibconfigHelper::getNumeric(config["color2"], "r");
                double g = LibconfigHelper::getNumeric(config["color2"], "g");
                double b = LibconfigHelper::getNumeric(config["color2"], "b");
                _color2 = Math::Vector3D(r, g, b);
            } else {
                _color2 = Math::Vector3D(0, 0, 0);
            }

            _scale = LibconfigHelper::getNumeric(config, "scale");
            if (_scale <= 0)
                _scale = 10;

            _reflectivity = 0.0;
            _transparency = 0.0;
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
            int x = (int)std::floor(point.x / _scale + 0.5);
            int y = (int)std::floor(point.y / _scale + 0.5);
            return ((x + y) % 2 == 0) ? _color1 : _color2;
        }

        double getReflectivity() const override { return _reflectivity; }
        double getTransparency()    const override { return _transparency; }
        double getRefractiveIndex() const override { return _refractiveIndex; }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "Chessboard("
                << "color1=(" << _color1.x << "," << _color1.y << "," << _color1.z << ") "
                << "color2=(" << _color2.x << "," << _color2.y << "," << _color2.z << ") "
                << "scale=" << _scale
                << " reflectivity=" << _reflectivity << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "Chessboard";
}

extern "C" IMaterial *createPlugin(const libconfig::Setting &config)
{
    return new Chessboard(config);
}
