/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** PerlinNoise
*/

#include <libconfig.h++>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>

#include "../../src/Interfaces/IMaterial.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

class PerlinNoise : public IMaterial {
    private:
        int perm[512];

        static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
        static double lerp(double t, double a, double b) { return a + t * (b - a); }
        static double grad(int hash, double x, double y) {
            int h = hash & 15;
            double u = h < 8 ? x : y;
            double v = h < 8 ? y : x;
            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        }

    public:
        Math::Vector3D _color1;
        Math::Vector3D _color2;
        double _scale;
        double _persistence;
        double _reflectivity;
        double _transparency;
        double _refractiveIndex;
        int _octaves;

        PerlinNoise(const libconfig::Setting &config)
        {
            int basePermutation[256] = {
                151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
                140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
                247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
                57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
                74,165,71,134,139,48,27,166,102,143,97,205,150,133,67,98,
                85,200,113,216,206,98,243,91,149,188,205,96,47,196,38,189,
                93,244,171,100,9,216,52,201,229,121,79,107,16,120,147,150,
                72,165,241,226,235,91,141,126,127,80,124,123,231,200,190,7,
                84,63,143,107,150,201,189,178,121,130,201,125,164,64,130,79,
                153,142,227,239,121,72,195,181,202,4,143,97,142,99,65,38,
                7,206,19,51,99,62,4,231,7,65,21,193,7,91,105,72,
                64,142,42,204,36,243,211,12,65,160,15,2,140,4,97,53,
                87,185,134,193,29,158,225,248,152,17,105,217,142,148,155,30,
                135,233,206,85,40,223,140,161,137,13,191,230,66,104,153,105,
                102,192,143,4,224,122,51,238,33,196,143,97,60,188,203,247,
                55,107,142,174,76,12,100,155,36,112,160,13,52,27,191,143
            };
            std::memcpy(perm, basePermutation, 256 * sizeof(int));
            std::memcpy(perm + 256, basePermutation, 256 * sizeof(int));

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
            if (_scale <= 0) _scale = 50.0;

            _persistence = LibconfigHelper::getNumeric(config, "persistence");
            if (_persistence <= 0 || _persistence > 1.0) _persistence = 0.5;

            _octaves = LibconfigHelper::getInt(config, "octaves");
            if (_octaves <= 0) _octaves = 4;
            if (_octaves > 8)  _octaves = 8;

            _reflectivity = 0.0;
            _transparency    = 0.0;
            _refractiveIndex = 1.0;
            if (config.exists("reflectivity"))
                _reflectivity = LibconfigHelper::getNumeric(config, "reflectivity");
            if (config.exists("transparency"))
                _transparency = LibconfigHelper::getNumeric(config, "transparency");
            if (config.exists("refractiveIndex"))
                _refractiveIndex = LibconfigHelper::getNumeric(config, "refractiveIndex");
        }

        double perlinNoise(double x, double y) const
        {
            x = x / _scale;
            y = y / _scale;
            int xi = ((int)std::floor(x)) & 255;
            int yi = ((int)std::floor(y)) & 255;
            double xf = x - std::floor(x);
            double yf = y - std::floor(y);
            double u = fade(xf), v = fade(yf);
            int aa = perm[perm[xi]     + yi];
            int ab = perm[perm[xi]     + yi + 1];
            int ba = perm[perm[xi + 1] + yi];
            int bb = perm[perm[xi + 1] + yi + 1];
            double x1 = lerp(u, grad(aa, xf, yf),           grad(ba, xf - 1.0, yf));
            double x2 = lerp(u, grad(ab, xf, yf - 1.0),     grad(bb, xf - 1.0, yf - 1.0));
            return lerp(v, x1, x2);
        }

        double fbm(double x, double y) const
        {
            double result = 0.0, amplitude = 1.0, frequency = 1.0, maxValue = 0.0;
            for (int i = 0; i < _octaves; i++) {
                result   += perlinNoise(x * frequency, y * frequency) * amplitude;
                maxValue += amplitude;
                amplitude  *= _persistence;
                frequency  *= 2.0;
            }
            return result / maxValue;
        }

        Math::Vector3D getColor(const Math::Vector3D &point) const override
        {
            double t = std::max(0.0, std::min(1.0, (fbm(point.x, point.y) + 1.0) * 0.5));
            return Math::Vector3D(
                _color1.x * (1.0 - t) + _color2.x * t,
                _color1.y * (1.0 - t) + _color2.y * t,
                _color1.z * (1.0 - t) + _color2.z * t
            );
        }

        double getReflectivity() const override { return _reflectivity; }
        double getTransparency()    const override { return _transparency; }
        double getRefractiveIndex() const override { return _refractiveIndex; }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "PerlinNoise("
                << "color1=(" << _color1.x << "," << _color1.y << "," << _color1.z << ") "
                << "color2=(" << _color2.x << "," << _color2.y << "," << _color2.z << ") "
                << "scale=" << _scale
                << " persistence=" << _persistence
                << " octaves=" << _octaves
                << " reflectivity=" << _reflectivity << ")";
            return oss.str();
        }
};

extern "C" const char *getSectionName()
{
    return "PerlinNoise";
}

extern "C" IMaterial *createPlugin(const libconfig::Setting &config)
{
    return new PerlinNoise(config);
}
