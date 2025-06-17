#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <SFML/Window.hpp>

int handleKeyboardInput(int selected, int totalOptions, const sf::Event::KeyEvent& key);
bool isNextScreenKey(const sf::Event::KeyEvent& key);

#endif

