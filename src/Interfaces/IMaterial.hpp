/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** IMaterial
*/

#ifndef IMATERIAL_HPP_
#define IMATERIAL_HPP_

#include <string>

#include "../Math/Math.hpp"

class IMaterial {
    public:
        virtual ~IMaterial() = default;
        
        virtual Math::Vector3D getColor(const Math::Vector3D &point) const = 0;
        virtual double getReflectivity() const = 0;
        virtual double getTransparency() const = 0;
        virtual double getRefractiveIndex() const = 0; 
    
        virtual std::string getDebugInfo() const = 0;
};

#endif /* !IMATERIAL_HPP_ */
