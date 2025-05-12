#include <SFML/Graphics.hpp>
#include <vector>
#include "screenStart.hpp"
#include "screenRule.hpp"
#include "screenGrade.hpp"
#include "countdown.hpp"
#include "playGrade.hpp"
#include "screenGraderesult.hpp"
#include "screenEnd.hpp"

int main() {
    sf::Texture bgTex;
    if (!bgTex.loadFromFile("/home/fmalgoajo/fmaj/assets/images/back.png"))
        return -1;
    sf::Vector2u bgSize = bgTex.getSize();

    sf::RenderWindow window(sf::VideoMode(bgSize.x, bgSize.y), "FmalgoAjo Game");

    // 창 크기 고정
    sf::View fixedView(sf::FloatRect(0, 0, 800, 600));
    window.setView(fixedView);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                window.setView(fixedView); // 다시 비율 고정
            }
        }

        int result = screenStart(window);

        if (result == 0) {  // START → 1~4학년 루프
            playGrade(window);

            int endResult = screenEnd(window);  // 0: 재입학, 1: 졸업

            if (endResult == 0) {
                continue; // 재입학 → 다시 처음부터
            } else {
                break;    // 졸업 → 종료
            }
        }
        else if (result == 1) {  // RULE
            int ruleResult = screenRule(window);

            if (ruleResult == 0 && window.isOpen()) {
                playGrade(window);

                int endResult = screenEnd(window);  // 0: 재입학, 1: 졸업

                if (endResult == 0) {
                    continue;
                } else {
                    break;
                }
            }
            else if (ruleResult == 1) {
                window.close();  // LEAVE
            }
        }

        break;
    }

    return 0;
}

