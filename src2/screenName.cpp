#include "screenName.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>

std::string screenName(sf::RenderWindow& window) {
    sf::Font font; //폰트 불러오기
    if (!font.loadFromFile("../assets/fonts/DungGeunMo.ttf")) {
        return "Player"; //폰트 불러오기 실패시 "Player"라는 기본 이름 반환하고 함수종료
    }

    sf::Text prompt("Enter your name:", font, 30); //이름 입력 안내 텍스트 객체 prompt
    prompt.setFillColor(sf::Color::White);
    prompt.setPosition(100, 200);

    sf::Text input("", font, 30); //유저가 타이핑한 문자열 실시간으로 보여주는 텍스트 객체 input
    input.setFillColor(sf::Color::Yellow);
    input.setPosition(100, 250);

    std::string name;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) { //사용자 키보드 입력에 따른 처리
                if (event.text.unicode == '\b') { //백스페이스 입력: 한글자 삭제
                    if (!name.empty()) name.pop_back();
                } else if (event.text.unicode == '\r' || event.text.unicode == '\n') {// 엔터키 : 이름 입력완료, name반환
                    if (!name.empty()) return name;
                } else if (name.length() < 12 && event.text.unicode >= 32 && event.text.unicode < 128) { //일반문자 입력
                    name += static_cast<char>(event.text.unicode);
                }
            }
        }

        input.setString(name); //화면에 나타날 텍스트 업데이트

        window.clear();
        window.draw(prompt);
        window.draw(input);//텍스트 렌더링
        window.display(); 
    }

    return "Player";
}

