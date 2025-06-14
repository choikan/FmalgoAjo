#include "dropGame.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace sf;

bool isRunning = true;
bool isGamePaused = false;
bool isGameOver = false;
int currentYear = 0;
float lastSpawnTime = 0.f;
int gradeCollisionCount = 0;
int rowCount = 0;
Clock gameClock;
mutex gradeMutex;

vector<Texture> gradeTextures(9);
vector<Sprite> gradeSprites(9);
Player player;
vector<float> collectedScores;
vector<float> yearAverages(4);
string currentYearAverageStr = "";

const float DROP_SPEED_BY_YEAR[4] = {4.5f, 5.5f, 8.5f, 9.0f};

void loadAssets() {
    for (int i = 0; i < 9; ++i) {
        string filename = "grade_" + to_string(i + 1) + ".png";
        if (!gradeTextures[i].loadFromFile(filename)) {
            cerr << "파일 로드 실패: " << filename << endl;
            exit(-1);
        }
        gradeSprites[i].setTexture(gradeTextures[i]);
    }
}


void initPlayer() {
    if (!player.texture.loadFromFile("player.png")) {
        cerr << "player.png 로드 실패\n";
        exit(-1);
    }
    player.sprite.setTexture(player.texture);  // ⚠️ 중요: player가 텍스처 보관함
    player.x = WINDOW_WIDTH / 2.f - player.texture.getSize().x / 2.f;
    player.y = WINDOW_HEIGHT - 80;
}

void drawPlayer(RenderWindow& window) {
    player.sprite.setPosition(player.x, player.y);
    window.draw(player.sprite);
}

void createSingleGrade(vector<Grade>& grades, float currentTime) {
    int randomCol = rand() % GRADES_PER_ROW;
    float x = randomCol * GRADE_SPACING_X;

    Grade g;
    g.sprite = gradeSprites[rand() % gradeSprites.size()];
    g.x = x;
    g.y = 0;
    g.activatedTime = currentTime;
    g.active = false;
    g.sprite.setPosition(g.x, g.y);

    grades.push_back(g);
}

void dropGrades(vector<Grade>& grades) {
    float spawnInterval = 0.3f;

    while (isRunning) {
        float currentTime = gameClock.getElapsedTime().asSeconds();

        if (!isGamePaused && !isGameOver) {
            if (currentTime - lastSpawnTime >= spawnInterval) {
		// 학년별로 떨어질 개수 결정-6.15일 수정
		int minCount = currentYear +1;  
		int maxCount = currentYear +2;
		int dropCount = rand() % (maxCount - minCount +1) + minCount;
                
		for (int i = 0; i < dropCount; ++i) {
			createSingleGrade(grades, currentTime);
		}

                lastSpawnTime = currentTime;
                spawnInterval = MIN_SPAWN_INTERVAL + static_cast<float>(rand()) / RAND_MAX * (MAX_SPAWN_INTERVAL - MIN_SPAWN_INTERVAL);
                ++rowCount;
            }

            {
                lock_guard<mutex> lock(gradeMutex);
                for (auto& g : grades) {
                    if (!g.active && currentTime >= g.activatedTime) {
                        g.active = true;
                    }
                    if (g.active) {
                        g.y += DROP_SPEED_BY_YEAR[currentYear];
                        g.sprite.setPosition(g.x, g.y);
                    }
                }

                grades.erase(
                    remove_if(grades.begin(), grades.end(), [](const Grade& g) {
                        return g.active && g.y >= WINDOW_HEIGHT - GRADE_HEIGHT;
                    }),
                    grades.end()
                );
            }
        }

        this_thread::sleep_for(chrono::milliseconds(30));
    }
}

void checkCollisions(vector<Grade>& grades) {
    FloatRect playerBounds = player.sprite.getGlobalBounds();

    grades.erase(
        remove_if(grades.begin(), grades.end(), [&](const Grade& g) {
            if (g.active && g.sprite.getGlobalBounds().intersects(playerBounds)) {
                for (size_t i = 0; i < gradeSprites.size(); ++i) {
                    if (g.sprite.getTexture() == gradeSprites[i].getTexture()) {
                        float score = 4.5f - i * 0.5f;
                        collectedScores.push_back(score);
                        gradeCollisionCount++;

                        if (gradeCollisionCount == 11) {
                            float total = 0;
                            for (float s : collectedScores) total += s;
                            float average = total / collectedScores.size();

                            stringstream oss;
                            oss.precision(2);
                            oss << fixed << average;
                            if (currentYear < 3) {
                                currentYearAverageStr = "Year " + to_string(currentYear + 1) + " Average: " + oss.str();
                            }

                            yearAverages[currentYear] = average;
                            collectedScores.clear();
                            gradeCollisionCount = 0;
                            isGamePaused = true;

                            if (currentYear == 3) {
                                isGameOver = true;
                                isRunning = false;
                            }
                        }
                        break;
                    }
                }
                return true;
            }
            return false;
        }),
        grades.end()
    );
}

void handleInput(RenderWindow& window) {
    Event event;
    while (window.pollEvent(event)) {
        if (event.type == Event::Closed) {
            isRunning = false;
            window.close();
        }
    }

    if (!isGamePaused && !isGameOver) {
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            player.x -= PLAYER_SPEED;
            if (player.x < 0) player.x = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) {
            player.x += PLAYER_SPEED;
            if (player.x + player.sprite.getGlobalBounds().width > WINDOW_WIDTH)
                player.x = WINDOW_WIDTH - player.sprite.getGlobalBounds().width;
        }
    }
}

void resetYear(vector<Grade>& grades) {
    if (isGamePaused && Keyboard::isKeyPressed(Keyboard::Enter)) {
        currentYear++;
        isGamePaused = false;
        gameClock.restart();
        lastSpawnTime = 0.0f;
        currentYearAverageStr = "";
        lock_guard<mutex> lock(gradeMutex);
        grades.clear();
    }
}

void drawUI(RenderWindow& window, Font& font, vector<Grade>& grades) {
    Text avgText;
    avgText.setFont(font);
    avgText.setCharacterSize(25);
    avgText.setFillColor(Color::White);
    avgText.setPosition(100, 200);
    avgText.setString(currentYearAverageStr);
    window.draw(avgText);

    if (isGamePaused && currentYear < 4 && !isGameOver && currentYearAverageStr != "" && currentYear != 3) {
        Text restartText;
        restartText.setFont(font);
        restartText.setString("Press Enter to Start Next Year");
        restartText.setCharacterSize(30);
        restartText.setFillColor(Color::Yellow);
        restartText.setPosition(100, 250);
        window.draw(restartText);
    }

    if (isGameOver) {
        for (int i = 0; i < 4; ++i) {
            Text yearText;
            yearText.setFont(font);
            yearText.setCharacterSize(25);
            yearText.setFillColor(Color::White);
            yearText.setPosition(100, 300 + i * 40);

            string avgStr = to_string(yearAverages[i]);
            avgStr = avgStr.substr(0, avgStr.find('.') + 3);

            yearText.setString("Year " + to_string(i + 1) + " Average: " + avgStr);
            window.draw(yearText);
        }

        float total = 0;
        for (float s : yearAverages) total += s;
        float finalAvg = total / 4;
        string finalStr = to_string(finalAvg);
        finalStr = finalStr.substr(0, finalStr.find('.') + 3);

        Text finalText;
        finalText.setFont(font);
        finalText.setCharacterSize(28);
        finalText.setFillColor(Color::Yellow);
        finalText.setPosition(100, 300 + 4 * 40 + 20);
        finalText.setString(" Final GPA: " + finalStr);
        window.draw(finalText);
    }
} 
