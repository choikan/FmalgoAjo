#include "screenName.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>

std::string screenName(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile("../assets/fonts/DungGeunMo.ttf")) {
        return "Player";
    }

    sf::Text prompt("Enter your name:", font, 30);
    prompt.setFillColor(sf::Color::White);
    prompt.setPosition(100, 200);

    sf::Text input("", font, 30);
    input.setFillColor(sf::Color::Yellow);
    input.setPosition(100, 250);

    std::string name;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b') {
                    if (!name.empty()) name.pop_back();
                } else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    if (!name.empty()) return name;
                } else if (name.length() < 12 && event.text.unicode >= 32 && event.text.unicode < 128) {
                    name += static_cast<char>(event.text.unicode);
                }
            }
        }

        input.setString(name);

        window.clear();
        window.draw(prompt);
        window.draw(input);
        window.display();
    }

    return "Player";
}

