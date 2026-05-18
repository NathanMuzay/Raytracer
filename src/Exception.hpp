/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-tekspice-8 [WSL : Ubuntu]
** File description:
** Exception
*/

#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <string>

class Exception : public std::exception {
    public:
        explicit Exception(const std::string &msg) : _msg(msg) {}
        const char *what() const noexcept override { return _msg.c_str(); }

    protected:
        std::string _msg;
};

class ParserException : public Exception {
    public:
        explicit ParserException(const std::string &msg)
            : Exception("[ParserException] " + msg) {}
};

class PluginLoaderException : public Exception {
    public:
        explicit PluginLoaderException(const std::string &msg)
            : Exception("[PluginLoaderException] " + msg) {}
};

class PrimitiveFactoryException : public Exception {
    public:
        explicit PrimitiveFactoryException(const std::string &msg)
            : Exception("[PrimitiveFactoryException] " + msg) {}
};

class LightFactoryException : public Exception {
    public:
        explicit LightFactoryException(const std::string &msg)
            : Exception("[LightFactoryException] " + msg) {}
};

class MaterialFactoryException : public Exception {
    public:
        explicit MaterialFactoryException(const std::string &msg)
            : Exception("[MaterialFactoryException] " + msg) {}
};

class RendererException : public Exception {
    public:
        explicit RendererException(const std::string &msg)
            : Exception("[RendererException] " + msg) {}
};

#endif /* !EXCEPTION_HPP_ */
