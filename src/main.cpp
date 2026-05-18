/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** main
*/

#include <iostream>
#include <cstring>

#include "Engine/Core.hpp"

void helper()
{
    std::cout << "USAGE: ./raytracer <SCENE_FILE> [--display | -d]" << std::endl;
    std::cout << "  SCENE_FILE: scene configuration" << std::endl;
    std::cout << "  --display, -d: open the UI window" << std::endl;
}

int main(int ac, char **av)
{
    if (ac >= 2 && (std::strcmp(av[1], "--help") == 0 || std::strcmp(av[1], "-h") == 0)) {
        helper();
        return 0;
    }
    if (ac < 2 || ac > 3) {
        helper();
        return 84;
    }
    bool display = (ac == 3 && (std::strcmp(av[2], "--display") == 0 || std::strcmp(av[2], "-d") == 0));
    try {
        Core core(av[1], display);
    } catch (const Exception &e){
        std::cerr << e.what() << std::endl;
        return 84;
    }
    return 0;
}
