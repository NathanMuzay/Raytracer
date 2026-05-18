/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Window
*/

#include "Window.hpp"

Window::Window(int width, int height, const std::string &title)
    : _width(width), _height(height)
{
    _window.create(sf::VideoMode(width, height), title);
    _window.setFramerateLimit(144);
    _buffer.assign(width * height * 4, 0);
    _texture.create(width, height);
    _sprite.setTexture(_texture);
}

void Window::setPixel(int x, int y, const Math::Vector3D &color)
{
    if (x < 0 || x >= _width || y < 0 || y >= _height)
        return;
    std::lock_guard<std::mutex> lock(_bufferMutex);
    int idx = (y * _width + x) * 4;
    _buffer[idx + 0] = static_cast<sf::Uint8>(color.x);
    _buffer[idx + 1] = static_cast<sf::Uint8>(color.y);
    _buffer[idx + 2] = static_cast<sf::Uint8>(color.z);
    _buffer[idx + 3] = 255;
}

void Window::display()
{
    {
        std::lock_guard<std::mutex> lock(_bufferMutex);
        _texture.update(_buffer.data());
    }
    _window.clear();
    _window.draw(_sprite);
    _window.display();
}

Window::Event Window::pollEvents()
{
    sf::Event event;
    while (_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            return Event::Close;
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape)
                return Event::Close;
            if (event.key.code == sf::Keyboard::R)
                return Event::Reload;
            if (event.key.code == sf::Keyboard::Z)
                return Event::MoveForward;
            if (event.key.code == sf::Keyboard::S)
                return Event::MoveBack;
            if (event.key.code == sf::Keyboard::Q)
                return Event::MoveLeft;
            if (event.key.code == sf::Keyboard::D)
                return Event::MoveRight;
            if (event.key.code == sf::Keyboard::Space)
                return Event::MoveUp;
            if (event.key.code == sf::Keyboard::LShift)
                return Event::MoveDown;
            if (event.key.code == sf::Keyboard::Left)
                return Event::RotateLeft;
            if (event.key.code == sf::Keyboard::Right)
                return Event::RotateRight;
            if (event.key.code == sf::Keyboard::Up)
                return Event::RotateUp;
            if (event.key.code == sf::Keyboard::Down)
                return Event::RotateDown;
        }
    }
    return Event::None;
}

bool Window::isOpen() const
{
    return _window.isOpen();
}

void Window::close()
{
    _window.close();
}
