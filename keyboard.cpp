#include "keyboard.hpp"

int handleKeyboardInput(int selected, int totalOptions, const sf::Event::KeyEvent& key) {
    if (key.code == sf::Keyboard::A || key.code == sf::Keyboard::Left) {
        selected = (selected - 1 + totalOptions) % totalOptions;
    } else if (key.code == sf::Keyboard::D || key.code == sf::Keyboard::Right) {
        selected = (selected + 1) % totalOptions;
    }
    return selected;
}

bool isNextScreenKey(const sf::Event::KeyEvent& key) {
    return key.code == sf::Keyboard::Space;
}

