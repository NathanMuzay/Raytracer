##
## EPITECH PROJECT, 2026
## G-PDG-300-MAR-3-1-PDGRUSH3-10
## File description:
## Makefile
##

CC = g++
CFLAGS = -std=c++20 -Wall -Wextra -Werror -g3 -fno-gnu-unique
SRC = $(shell find src -name '*.cpp')
OBJ = $(SRC:.cpp=.o)
NAME = raytracer

PLUGINS_SRC = $(shell find plugins -name '*.cpp')
PLUGINS = $(PLUGINS_SRC:.cpp=.so)

all: plugins $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lconfig++ -ldl

plugins: $(PLUGINS)

%.so: %.cpp
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@ -lconfig++

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
	rm -f $(PLUGINS)

re: fclean all
