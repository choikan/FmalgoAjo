#include "screenGrade.hpp"
#include <SFML/Graphics.hpp>
#include <string>

int screenGrade(sf::RenderWindow& window, int grade) {
    sf::Font font;
    if (!font.loadFromFile("/home/fmalgoajo/fmaj/assets/fonts/DungGeunMo.ttf"))
        return -1;

    sf::Text playingText("~ PLAYING : " + std::to_string(grade) + " grade ~", font, 30);
    playingText.setPosition(220, 300);

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                return -1;
        }

        window.clear();
        window.draw(playingText);
        window.display();

        if (clock.getElapsedTime().asSeconds() > 3.f)
            break;
    }

    return 0;
}
