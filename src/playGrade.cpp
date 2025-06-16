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

            if (isGameOver) break;
        }

        dropThread.join();

        int result = screenGrade(window, currentYear + 1);
        if (result == -1) {
            window.close();
            return -1;
        }
    }
    return 0;
}

