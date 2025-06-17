#include "playGrade.hpp"
#include "countdown.hpp"
#include "screenGrade.hpp"
#include "dropGame.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>
#include <iostream>
#include "screengraderesult.hpp"
#include "screenEnd.hpp"
#include <chrono>

extern bool isRunning;
extern bool isGamePaused;
extern bool isGameOver;
extern int currentYear;
extern float lastSpawnTime;
extern int gradeCollisionCount;
extern int rowCount;
extern sf::Clock gameClock;
extern std::mutex gradeMutex;
extern std::vector<float> yearAverages;
extern std::string currentYearAverageStr;
extern std::vector<float> collectedScores;

int playGrade(sf::RenderWindow& window) {
sf::Texture bgTex;
    if (!bgTex.loadFromFile("../assets/images/back.png"))
        return -1;

    for (currentYear = 0; currentYear < 4 && window.isOpen(); ++currentYear) {
        showCountdown(window, std::to_string(currentYear + 1) + " grade");

        // 낙하 게임 초기화
        isRunning = true;
        isGamePaused = false;
        isGameOver = false;
        lastSpawnTime = 0.f;
        gradeCollisionCount = 0;
        rowCount = 0;
        gameClock.restart();
        collectedScores.clear();
        currentYearAverageStr = "";

        std::vector<Grade> grades;
        loadAssets();
        initPlayer();

        sf::Font font;
        if (!font.loadFromFile("../assets/fonts/DungGeunMo.ttf")) {
            std::cerr << "폰트 로드 실패\n";
            return -1;
        }

        std::thread dropThread(dropGrades, std::ref(grades));

        while (isRunning && window.isOpen()) {
            handleInput(window);
            checkCollisions(grades);
            resetYear(grades);

            window.clear();
            {
                std::lock_guard<std::mutex> lock(gradeMutex);
                for (auto& g : grades) {
                    window.draw(g.sprite);
                }
            }
            drawPlayer(window);
            drawUI(window, font, grades);
            window.display();

            if (isGameOver) {
                isRunning = false; // ✅ dropGrades 루프를 종료시키기 위한 플래그 설정
                break;
            }
        }

        dropThread.join(); // 이제 안전하게 스레드를 종료할 수 있음

      
    }

    // 모든 학년이 끝났을 때 결과 화면 → 선택 화면
    if (currentYear == 4) {
        int grade = 4; // 또는 어떤 변수를 쓰든 간에 현재 학년
        float score = finalGPA; 
        std::this_thread::sleep_for(std::chrono::seconds(9));

        int result = screenGradeResult(window, grade, score);
        if (result == -1) {
            window.close();
            return 1;
        }

    }

    return 0;
}

