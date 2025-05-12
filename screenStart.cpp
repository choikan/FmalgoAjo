#include <SFML/Graphics.hpp>
#include "keyboard.hpp"
#include "screenStart.hpp"

int screenStart(sf::RenderWindow& window) {
    // ===== 폰트 로딩 =====
    sf::Font font;
    if (!font.loadFromFile("/home/fmalgoajo/fmaj/assets/fonts/DungGeunMo.ttf"))
        return -1;

    // ===== 배경 이미지 =====
    sf::Texture bgTex;
    if (!bgTex.loadFromFile("/home/fmalgoajo/fmaj/assets/images/back.png"))
        return -1;
    sf::Sprite bgSprite(bgTex);

    // ===== 로고 이미지 =====
    sf::Texture logoTex;
    if (!logoTex.loadFromFile("/home/fmalgoajo/fmaj/assets/images/FmalgoAjo.png"))
        return -1;
    sf::Sprite logoSprite(logoTex);
    logoSprite.setScale(0.08f, 0.08f);
    logoSprite.setPosition((800 - logoTex.getSize().x * 0.08f) / 4.f, 30.f); // 화면 상단 중앙

    // ===== 버튼 텍스트 =====
    sf::Text startText("START", font, 40);
    sf::Text ruleText("RULE", font, 40);
    int selected = 0;

    // 고정된 좌표계(800x600) 기준으로 위치 지정
    startText.setPosition(800 / 2.f - 150, 600 - 100);  // 좌측
    ruleText.setPosition(800 / 2.f + 50, 600 - 100);    // 우측

    // ===== 이벤트 루프 =====
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                selected = handleKeyboardInput(selected, 2, event.key);
                if (event.key.code == sf::Keyboard::Enter || isNextScreenKey(event.key))
                    return selected;
            }
        }

        startText.setFillColor(selected == 0 ? sf::Color::Yellow : sf::Color::White);
        ruleText.setFillColor(selected == 1 ? sf::Color::Yellow : sf::Color::White);

        window.clear();
        window.draw(bgSprite);
        window.draw(logoSprite);
        window.draw(startText);
        window.draw(ruleText);
        window.display();
    }

    return -1;
}

