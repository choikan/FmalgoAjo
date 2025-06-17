#include "screengraderesult.hpp"
#include <SFML/Graphics.hpp>

int screenGradeResult(sf::RenderWindow& window, int grade, int score) {
    sf::Font font;
    if (!font.loadFromFile("../assets/fonts/DungGeunMo.ttf"))
        return -1;

    std::string resultText = std::to_string(grade) + " grade Result : " + std::to_string(score);
    sf::Text result(resultText, font, 40);
    result.setFillColor(sf::Color::White);

    sf::FloatRect bounds = result.getLocalBounds();
    result.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
    result.setPosition(800 / 2, 600 / 2);

    sf::Text guide("Next (Space)", font, 25);
    guide.setFillColor(sf::Color::Yellow);
    sf::FloatRect gBounds = guide.getLocalBounds();
    guide.setOrigin(gBounds.left + gBounds.width / 2, gBounds.top + gBounds.height / 2);
    guide.setPosition(800 / 2, 600 / 2 + 60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                return -1;
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                return 0;
        }

        window.clear(sf::Color::Black);
        window.draw(result);
        window.draw(guide);
        window.display();
    }

    return 0;
}
