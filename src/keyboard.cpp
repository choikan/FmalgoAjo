#include "keyboard.hpp"

int handleKeyboardInput(int selected, int totalOptions, const sf::Event::KeyEvent& key) {
    if (key.code == sf::Keyboard::W || key.code == sf::Keyboard::A) {
        selected = (selected - 1 + totalOptions) % totalOptions;
    } else if (key.code == sf::Keyboard::S || key.code == sf::Keyboard::D) {
        selected = (selected + 1) % totalOptions;
    }
    return selected;
}

bool isNextScreenKey(const sf::Event::KeyEvent& key) {
    return key.code == sf::Keyboard::Space;
}

