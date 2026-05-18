/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Parser
*/

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <libconfig.h++>

#include "../Exception.hpp"

class Parser {
    public:
        Parser(const std::string &configFile);
        ~Parser() = default;

        const libconfig::Config &getConfig() const;
        const libconfig::Setting &getCamera() const;
        const libconfig::Setting &getPrimitives() const;
        const libconfig::Setting &getLights() const;

    private:
        libconfig::Config _config;

        void validateConfig() const;
        void validateCamera(const libconfig::Setting &camera) const;
        void validatePrimitives(const libconfig::Setting &primitives) const;
        void validateLights(const libconfig::Setting &lights) const;
};

#endif /* !PARSER_HPP_ */
