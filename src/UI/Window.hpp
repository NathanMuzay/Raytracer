/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Window
*/

#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <mutex>

#include "../Math/Math.hpp"

class Window {
    public:
        enum class Event {
            None,
            Close,
            Reload,
            MoveForward, 
            MoveBack, 
            MoveLeft, 
            MoveRight, 
            MoveUp, 
            MoveDown,
            RotateLeft,
            RotateRight,
            RotateUp,
            RotateDown
        };

        Window(int width, int height, const std::string &title);
        ~Window() = default;

        void setPixel(int x, int y, const Math::Vector3D &color);

        Event   pollEvents();
        void    display();
        bool    isOpen() const;
        void    close();

    private:
        sf::RenderWindow        _window;
        int                     _width;
        int                     _height;
        sf::Texture             _texture;
        sf::Sprite              _sprite;
        std::vector<sf::Uint8>  _buffer; // RGBA
        std::mutex              _bufferMutex;
};

#endif /* !WINDOW_HPP_ */
