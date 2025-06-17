#include <SFML/Graphics.hpp>
#include <vector>
#include "screenStart.hpp"
#include "screenRule.hpp"
#include "screenGrade.hpp"
#include "countdown.hpp"
#include "playGrade.hpp"
#include "screengraderesult.hpp"
#include "screenEnd.hpp"
#include <iostream>
#include <string>
#include "screenName.hpp"
std::string playerName;

using namespace std;



int main() {
  
    sf::Texture bgTex;
    if (!bgTex.loadFromFile("../assets/images/back.png"))
        return -1;
    sf::Vector2u bgSize = bgTex.getSize();

    sf::RenderWindow window(sf::VideoMode(bgSize.x, bgSize.y), "FmalgoAjo Game");
    window.setFramerateLimit(110); // 프레임 제한


    // 창 크기 고정
    sf::View fixedView(sf::FloatRect(0, 0, 800, 600));
    window.setView(fixedView);

    while (window.isOpen()) {
        sf::Event event; //이벤트 처리; 창 닫히거나 크기 변경 등의 사용자 이벤트 감지
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();// 창 닫히면 프로그램도 종료
            } else if (event.type == sf::Event::Resized) {
                window.setView(fixedView); // 레이아웃 깨지지 않도록 고정된 비율 유지
            }
        }
	playerName = screenName(window);//플레이어 이름 입력
	// 화면 초기화
	window.clear();
	window.display();
        int result = screenStart(window);//게임 시작화면 0이면 시작, 1이면 게임 규칙보기

        if (result == 0) {  // START → 1~4학년 루프
            playGrade(window);

            int endResult = screenEnd(window);  // 0: 재입학, 1: 졸업

            if (endResult == 0) {
                continue; // 재입학-현재 루프 나머지 부분 건너뛰고 루프의 처음으로
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
                    continue; // 재입학-현재 루프 나머지 부분 건너뛰고 루프의 처음으로
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

