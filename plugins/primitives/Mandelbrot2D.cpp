/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5
** File description:
** Mandelbrot2D primitive plugin
*/

#include <libconfig.h++>
#include <cmath>
#include <memory>
#include <sstream>

#include "../../src/Interfaces/IPrimitive.hpp"
#include "../../src/Parser/LibconfigHelper.hpp"

namespace {
    Math::Vector3D clampColor(const Math::Vector3D &color)
    {
        return Math::Vector3D(
            std::max(0.0, std::min(255.0, color.x)),
            std::max(0.0, std::min(255.0, color.y)),
            std::max(0.0, std::min(255.0, color.z))
        );
    }

    class MandelbrotMaterial : public IMaterial {
        public:
            MandelbrotMaterial(const Math::Point3D &center,
                double width, double height,
                double minRe, double maxRe,
                double minIm, double maxIm,
                int maxIter,
                const Math::Vector3D &inside,
                const Math::Vector3D &outside)
                : _center(center)
                , _width(width)
                , _height(height)
                , _minRe(minRe)
                , _maxRe(maxRe)
                , _minIm(minIm)
                , _maxIm(maxIm)
                , _maxIter(maxIter)
                , _inside(clampColor(inside))
                , _outside(clampColor(outside))
            {
            }

            Math::Vector3D getColor(const Math::Vector3D &point) const override
            {
                double u = (point.x - _center.x) / _width + 0.5;
                double v = (point.z - _center.z) / _height + 0.5;

                if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0)
                    return _inside;

                double re = _minRe + u * (_maxRe - _minRe);
                double im = _maxIm - v * (_maxIm - _minIm);
                int iter = mandelbrot(re, im);

                if (iter >= _maxIter)
                    return _inside;

                double ratio = static_cast<double>(iter) / static_cast<double>(_maxIter);
                double smooth = std::pow(ratio, 0.45);

                return clampColor(Math::Vector3D(
                    _inside.x * (1.0 - smooth) + _outside.x * smooth,
                    _inside.y * (1.0 - smooth) + _outside.y * smooth,
                    _inside.z * (1.0 - smooth) + _outside.z * smooth
                ));
            }

            double getReflectivity() const override
            {
                return 0.0;
            }

            double getTransparency() const override
            {
                return 0.0;
            }

            double getRefractiveIndex() const override
            {
                return 1.0;
            }

            std::string getDebugInfo() const override
            {
                return "MandelbrotMaterial";
            }

        private:
            int mandelbrot(double re, double im) const
            {
                double x = 0.0;
                double y = 0.0;

                for (int iter = 0; iter < _maxIter; ++iter) {
                    double nextX = x * x - y * y + re;
                    y = 2.0 * x * y + im;
                    x = nextX;
                    if (x * x + y * y > 4.0)
                        return iter;
                }
                return _maxIter;
            }

            Math::Point3D _center;
            double _width;
            double _height;
            double _minRe;
            double _maxRe;
            double _minIm;
            double _maxIm;
            int _maxIter;
            Math::Vector3D _inside;
            Math::Vector3D _outside;
    };
}

class Mandelbrot2D : public IPrimitive {
    public:
        Mandelbrot2D(const libconfig::Setting &config)
        {
            _center = Math::Point3D(
                LibconfigHelper::getNumeric(config, "x"),
                LibconfigHelper::getNumeric(config, "y"),
                LibconfigHelper::getNumeric(config, "z")
            );
            _width = config.exists("width") ? LibconfigHelper::getNumeric(config, "width") : 400.0;
            _height = config.exists("height") ? LibconfigHelper::getNumeric(config, "height") : 300.0;
            _minRe = config.exists("minRe") ? LibconfigHelper::getNumeric(config, "minRe") : -2.0;
            _maxRe = config.exists("maxRe") ? LibconfigHelper::getNumeric(config, "maxRe") : 1.0;
            _minIm = config.exists("minIm") ? LibconfigHelper::getNumeric(config, "minIm") : -1.2;
            _maxIm = config.exists("maxIm") ? LibconfigHelper::getNumeric(config, "maxIm") : 1.2;
            _maxIter = config.exists("maxIter") ? LibconfigHelper::getInt(config, "maxIter") : 100;
            _rotation = Math::Vector3D(0, 0, 0);
            _material = std::make_shared<MandelbrotMaterial>(
                _center,
                _width,
                _height,
                _minRe,
                _maxRe,
                _minIm,
                _maxIm,
                _maxIter,
                readColor(config, "colorInside", Math::Vector3D(0, 0, 0)),
                readColor(config, "colorOutside", Math::Vector3D(255, 180, 40))
            );
        }

        double getIntersection(const Ray &ray) const override
        {
            Math::Vector3D origin = toLocal(ray.origin.toVector());
            Math::Vector3D direction = toLocal(ray.direction).normalize();

            if (std::abs(direction.y) < 1e-8)
                return -1.0;

            double t = (_center.y - origin.y) / direction.y;
            if (t <= 1e-4)
                return -1.0;

            Math::Vector3D localHit = origin + direction * t;
            double u = (localHit.x - _center.x) / _width + 0.5;
            double v = (localHit.z - _center.z) / _height + 0.5;

            if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0)
                return -1.0;

            double re = _minRe + u * (_maxRe - _minRe);
            double im = _maxIm - v * (_maxIm - _minIm);
            if (mandelbrot(re, im) >= _maxIter)
                return -1.0;

            return t;
        }

        bool hits(const Ray &ray) const override
        {
            return getIntersection(ray) > 0.0;
        }

        Math::Vector3D getNormal(const Math::Vector3D &point) const override
        {
            (void)point;
            return Math::Vector3D(0, 1, 0);
        }

        AABB getAABB() const override
        {
            const double epsilon = 1e-3;
            return AABB(
                Math::Vector3D(_center.x - _width * 0.5, _center.y - epsilon, _center.z - _height * 0.5),
                Math::Vector3D(_center.x + _width * 0.5, _center.y + epsilon, _center.z + _height * 0.5)
            );
        }

        void setMaterial(std::shared_ptr<IMaterial> material) override
        {
            _material = material;
        }

        std::shared_ptr<IMaterial> getMaterial() const override
        {
            return _material;
        }

        void applyTranslation(const Math::Vector3D &translation) override
        {
            _center.x += translation.x;
            _center.y += translation.y;
            _center.z += translation.z;
        }

        void applyRotation(const Math::Vector3D &angles) override
        {
            _rotation = angles;
        }

        std::string getDebugInfo() const override
        {
            std::ostringstream oss;
            oss << "Mandelbrot2D(center=" << _center.x << "," << _center.y << "," << _center.z
                << " width=" << _width
                << " height=" << _height
                << " maxIter=" << _maxIter << ")";
            return oss.str();
        }

    private:
        Math::Vector3D readColor(const libconfig::Setting &config, const char *name, const Math::Vector3D &fallback) const
        {
            if (!config.exists(name))
                return fallback;
            const libconfig::Setting &color = config[name];
            return Math::Vector3D(
                LibconfigHelper::getNumeric(color, "r"),
                LibconfigHelper::getNumeric(color, "g"),
                LibconfigHelper::getNumeric(color, "b")
            );
        }

        Math::Vector3D toLocal(const Math::Point3D &point) const
        {
            return Math::Vector3D(point.x, point.y, point.z);
        }

        Math::Vector3D toLocal(const Math::Vector3D &vector) const
        {
            return vector;
        }

        int mandelbrot(double re, double im) const
        {
            double x = 0.0;
            double y = 0.0;

            for (int iter = 0; iter < _maxIter; ++iter) {
                double nextX = x * x - y * y + re;
                y = 2.0 * x * y + im;
                x = nextX;
                if (x * x + y * y > 4.0)
                    return iter;
            }
            return _maxIter;
        }

        Math::Point3D _center;
        double _width;
        double _height;
        double _minRe;
        double _maxRe;
        double _minIm;
        double _maxIm;
        int _maxIter;
        Math::Vector3D _rotation;
        std::shared_ptr<IMaterial> _material;
};

extern "C" const char *getSectionName()
{
    return "mandelbrot2d";
}

extern "C" IPrimitive *createPlugin(const libconfig::Setting &config)
{
    return new Mandelbrot2D(config);
}
