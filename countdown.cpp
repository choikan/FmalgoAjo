#include "countdown.hpp"
#include <unistd.h>

void showCountdown(sf::RenderWindow& window, const std::string& gradeLabel) {
    sf::Font font;
    if (!font.loadFromFile("/home/fmalgoajo/fmaj/assets/fonts/DungGeunMo.ttf"))
        return;

    // 개강 텍스트
    sf::Text title(gradeLabel + " semester start soon!", font, 40);
    title.setFillColor(sf::Color::Yellow);

    // 중앙 정렬
    sf::FloatRect tBounds = title.getLocalBounds();
    title.setOrigin(tBounds.left + tBounds.width / 2.0f, tBounds.top + tBounds.height / 2.0f);
    title.setPosition(800 / 2.0f, 330);

    // 개강 텍스트 2초간 표시
    sf::Clock clock;
    while (clock.getElapsedTime().asSeconds() < 2.f) {
        window.clear(sf::Color::Black);
        window.draw(title);
        window.display();
    }

    // D-3 ~ D-day 카운트다운
    for (int i = 3; i >= 0; --i) {
        std::string label = (i == 0) ? "D - day" : "D - " + std::to_string(i);

        sf::Text countdown(label, font, 35);
        countdown.setFillColor(sf::Color::White);

        // 중앙 정렬
        sf::FloatRect cBounds = countdown.getLocalBounds();
        countdown.setOrigin(cBounds.left + cBounds.width / 2.0f, cBounds.top + cBounds.height / 2.0f);
        countdown.setPosition(800 / 2.0f, 330);

        window.clear(sf::Color::Black);
        window.draw(countdown);
        window.display();

	sf::Event event;
	while (window.pollEvent(event)) {
   		if (event.type == sf::Event::Closed)
        		window.close();
	}

        sf::sleep(sf::seconds(1));
    }
}

