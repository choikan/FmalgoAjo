#include <SFML/Graphics.hpp>
#include "screenRule.hpp"
#include "keyboard.hpp" // ← 키보드 입력 함수 사용

int screenRule(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile("../assets/fonts/DungGeunMo.ttf"))
        return -1;

    const char* ruleContent =
        "This is a college student's reality life game. \n\n\n"
        "0. Use only A (move left) & D (move right) & Space (click).\n\n"
        "1. When you click Start, the 1st grade of college begins.\n"
        "   At the same time, a countdown starts from D-3. \n"
	"   And D-day arrives, your credits will start to drop.\n\n"
        "2. Use the A and D keyboards to control \n"
	"   your character to reach the falling credits.\n"
	"   When you have 33 credits, one school year ends. \n\n"
        "3. The above process is repeated \n"
	"   from the first year to the fourth year.\n"
	"   Add, you cannot return to the previous year.\n\n"
        "4. When all four years are finished, \n"
	"   you can choose to re-enroll \n"
	"   or quit along with your total credits.\n"
	"   If you click re-enroll,\n"
	"   you will start over from the first year.\n"
	"   And if you choose quit, the game will end completely.\n";

    sf::Text ruleText(ruleContent, font, 20);
    ruleText.setFillColor(sf::Color::White);
    ruleText.setPosition(50, 30);

    sf::Text startText("START", font, 35);
    sf::Text leaveText("LEAVE", font, 35);

    startText.setPosition(800 / 2 - 150, 530);
    leaveText.setPosition(800 / 2 + 50, 530);

    int selected = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                selected = handleKeyboardInput(selected, 2, event.key);

                if (event.key.code == sf::Keyboard::Enter || isNextScreenKey(event.key)) {
                    return selected; // 0: START, 1: LEAVE
                }
            }
        }

        startText.setFillColor(selected == 0 ? sf::Color::Yellow : sf::Color::White);
        leaveText.setFillColor(selected == 1 ? sf::Color::Yellow : sf::Color::White);

        window.clear(sf::Color::Black);
        window.draw(ruleText);
        window.draw(leaveText);
        window.draw(startText);
        window.display();
    }

    return 0;
}
