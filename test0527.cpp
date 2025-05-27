#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <iostream>
#include <mutex>
#include <sstream>

using namespace std;
using namespace sf;

// --- 상수 정의 ---
constexpr int GRADES_PER_ROW = 5;
constexpr int GRADE_WIDTH = 140;
constexpr int GRADE_HEIGHT = 70;
constexpr int GRADE_SPACING_X = 140;
constexpr int WINDOW_WIDTH = GRADES_PER_ROW * GRADE_SPACING_X;
constexpr int WINDOW_HEIGHT = 600;
constexpr float DROP_SPEED = 5.0f;
constexpr float MIN_SPAWN_INTERVAL = 0.3f;
constexpr float MAX_SPAWN_INTERVAL = 0.7f;
constexpr float PLAYER_SPEED = 10.0f;

// --- 전역 변수 ---
bool isRunning = true;
float lastSpawnTime = true;
vector<Texture> gradeTextures(9);
vector<Sprite> gradeSprites(9);
mutex gradeMutex;
Clock gameClock;

vector<float> collectedScores;
int gradeCollisionCount = 0;
int currentYear = 0;
vector<float> yearAverages(4);
bool isGamePaused = false;
bool isGameOver = false;
string currentYearAverageStr = "";

struct Grade {
    Sprite sprite;
    int x, y;
    bool active = false;
    float activatedTime = 0.f;
};

struct Player {
    Sprite sprite;
    float x, y;
};

Player player;

void drawPlayer(RenderWindow& window) {
    player.sprite.setPosition(player.x, player.y);
    window.draw(player.sprite);
}

void CreateSingleGrade(vector<Grade>& grades, float currentTime) {
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
   
    float spawnInterval = 0.5f;

    while (isRunning) {
        float currentTime = gameClock.getElapsedTime().asSeconds();

        if (!isGamePaused && !isGameOver) {
            if (currentTime - lastSpawnTime >= spawnInterval) {
                lock_guard<mutex> lock(gradeMutex);
                CreateSingleGrade(grades, currentTime);
                lastSpawnTime = currentTime;

                spawnInterval = MIN_SPAWN_INTERVAL + static_cast<float>(rand()) / RAND_MAX * (MAX_SPAWN_INTERVAL - MIN_SPAWN_INTERVAL);
            }

            {
                lock_guard<mutex> lock(gradeMutex);
                for (auto& g : grades) {
                    if (!g.active && currentTime >= g.activatedTime) {
                        g.active = true;
                    }
                    if (g.active) {
                        g.y += DROP_SPEED;
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
                for (int i = 0; i < gradeSprites.size(); ++i) {
                    if (g.sprite.getTexture() == gradeSprites[i].getTexture()) {
                        float score = 4.5f - i * 0.5f;
                        collectedScores.push_back(score);
                        gradeCollisionCount++;

                        cout << "추부한 학점: grade_" << (i + 1) << " → " << score << "점" << endl;

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

                            cout << "\n📘 " << currentYear + 1 << "학년 평균 점수: " << average << "점\n" << endl;

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

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    Font font;
    if (!font.loadFromFile("NotoSansKR-VariableFont_wght.ttf")) {
        cerr << "폰트 로드 실패!" << endl;
    }

    Text restartText;
    restartText.setFont(font);
    restartText.setString("Press Enter to Start Next Year");
    restartText.setCharacterSize(30);
    restartText.setFillColor(Color::Yellow);
    restartText.setPosition(100, 250);

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "FMALGOAJO");
    window.setFramerateLimit(60);

    for (int i = 0; i < 9; ++i) {
        string filename = "grade_" + to_string(i + 1) + ".png";
        if (!gradeTextures[i].loadFromFile(filename)) {
            cerr << "파일 로드 실패: " << filename << endl;
            return -1;
        }
        gradeSprites[i].setTexture(gradeTextures[i]);
    }

    Texture playerTexture;
    if (!playerTexture.loadFromFile("player.png")) {
        cerr << "player.png 로드 실패\n";
        return -1;
    }

    player.sprite.setTexture(playerTexture);
    player.x = WINDOW_WIDTH / 2.f - playerTexture.getSize().x / 2.f;
    player.y = WINDOW_HEIGHT - 80;

    vector<Grade> grades;
    thread dropThread(dropGrades, ref(grades));

    while (window.isOpen()) {
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

        if (isGamePaused && Keyboard::isKeyPressed(Keyboard::Enter)) {
            currentYear++;
            isGamePaused = false;

            gameClock.restart();
	    lastSpawnTime = 0.0f;
            currentYearAverageStr = "";

            lock_guard<mutex> lock(gradeMutex);
            grades.clear();
        }

        {
            lock_guard<mutex> lock(gradeMutex);
            checkCollisions(grades);
        }

        window.clear(Color::Black);

        {
            lock_guard<mutex> lock(gradeMutex);
            for (auto& g : grades) {
                window.draw(g.sprite);
            }
        }

        drawPlayer(window);

        if (isGamePaused && currentYear < 4 && !isGameOver && currentYearAverageStr != "" && currentYear != 3) {
            window.draw(restartText);
        }

        Text avgText;
        avgText.setFont(font);
        avgText.setCharacterSize(25);
        avgText.setFillColor(Color::White);
        avgText.setPosition(100, 200);
        avgText.setString(currentYearAverageStr);
        window.draw(avgText);

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

        window.display();
    }

    dropThread.join();
    return 0;
}

