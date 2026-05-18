/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5
** File description:
** Texture material plugin
*/

#define STB_IMAGE_IMPLEMENTATION
#include "../../src/stb/stb_image.h"

#include "../../src/Interfaces/IMaterial.hpp"
#include <libconfig.h++>
#include <vector>
#include <cmath>
#include <stdexcept>    
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Texture : public IMaterial {
public:
    Texture(const libconfig::Setting &config)
    {
        std::string path;
        if (!config.lookupValue("path", path) || path.empty())
            throw std::runtime_error("TextureImage: missing 'path' field");

        if (config.exists("reflectivity")) {
            double r = 0.0;
            int ri = 0;
            if (config.lookupValue("reflectivity", r))
                _reflectivity = r;
            else if (config.lookupValue("reflectivity", ri))
                _reflectivity = static_cast<double>(ri);
        }
        if (config.exists("transparency")) {
            double t = 0.0; int ti = 0;
            if (config.lookupValue("transparency", t))       _transparency = t;
            else if (config.lookupValue("transparency", ti)) _transparency = static_cast<double>(ti);
        }
        if (config.exists("refractiveIndex")) {
            double n = 1.0; int ni = 0;
            if (config.lookupValue("refractiveIndex", n))       _refractiveIndex = n;
            else if (config.lookupValue("refractiveIndex", ni)) _refractiveIndex = static_cast<double>(ni);
        }

        int channels = 0;
        unsigned char *data = stbi_load(path.c_str(), &_width, &_height, &channels, 3);
        if (!data)
            throw std::runtime_error("TextureImage: cannot load image: " + path);

        _pixels.assign(data, data + _width * _height * 3);
        stbi_image_free(data);
    }

    ~Texture() override = default;

    // Mapping UV sphérique
    // normal doit être un vecteur normalisé pointant depuis le centre de la primitive
    Math::Vector3D getColor(const Math::Vector3D &normal) const override
    {
        Math::Vector3D n = normal;
        double len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
        if (len > 0.0) { n.x /= len; n.y /= len; n.z /= len; }

        // Coordonnées UV sphériques
        double u = 0.5 + std::atan2(n.y, n.x) / (2.0 * M_PI);
        double v = 0.5 - std::asin(std::max(-1.0, std::min(1.0, n.z))) / M_PI;

        int px = static_cast<int>(u * (_width  - 1)) % _width;
        int py = static_cast<int>(v * (_height - 1)) % _height;

        if (px < 0) px += _width;
        if (py < 0) py += _height;

        int idx = (py * _width + px) * 3;
        return Math::Vector3D(
            static_cast<double>(_pixels[idx + 0]),
            static_cast<double>(_pixels[idx + 1]),
            static_cast<double>(_pixels[idx + 2])
        );
    }

    double getReflectivity() const override { return _reflectivity; }
    double getTransparency()    const override { return _transparency; }
    double getRefractiveIndex() const override { return _refractiveIndex; }

    std::string getDebugInfo() const override
    {
        return "TextureImage(size=" + std::to_string(_width) + "x"
            + std::to_string(_height)
            + " reflectivity=" + std::to_string(_reflectivity) + ")";
    }

private:
    int _width  = 0;
    int _height = 0;
    std::vector<unsigned char> _pixels;
    double _reflectivity = 0.0;
    double _transparency;
    double _refractiveIndex;
};

// ── Exports plugin ────────────────────────────────────────────────────────────

extern "C" const char *getSectionName()
{
    return "Texture";
}

extern "C" IMaterial *createPlugin(const libconfig::Setting &config)
{
    return new Texture(config);
}
