#include "screenEnd.hpp"
#include "keyboard.hpp"

int screenEnd(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile("../assets/fonts/DungGeunMo.ttf"))
        return -1;

    sf::Text readmitText("READMISSION", font, 40);
    sf::Text graduateText("GRADUATE", font, 40);

    readmitText.setPosition(800 / 2 - 200, 500);
    graduateText.setPosition(800 / 2 + 50, 500);

    int selected = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                selected = handleKeyboardInput(selected, 2, event.key);
                if (event.key.code == sf::Keyboard::Enter || isNextScreenKey(event.key)) {
                    return selected; // 0: 재입학, 1: 졸업
                }
            }
        }

        readmitText.setFillColor(selected == 0 ? sf::Color::Yellow : sf::Color::White);
        graduateText.setFillColor(selected == 1 ? sf::Color::Yellow : sf::Color::White);

        window.clear();
        window.draw(readmitText);
        window.draw(graduateText);
        window.display();
    }

    return 1; // graduate : 게임 종료
}
