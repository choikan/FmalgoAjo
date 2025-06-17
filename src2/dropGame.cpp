#include "dropGame.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <cmath>

extern std::string playerName;
void drawRanking(sf::RenderWindow& window, sf::Font& font) {
    std::vector<std::pair<std::string, float>> rankings;
    std::ifstream in("rank.txt");
    std::string name;
    float score;

    while (in >> name >> score) {
        rankings.push_back({name, score});
    }
    in.close();

    std::sort(rankings.begin(), rankings.end(), [](auto& a, auto& b) {
        return a.second > b.second;
    });

    int count = std::min(5, static_cast<int>(rankings.size()));

    for (int i = 0; i < count; ++i) {
        sf::Text rankText;
        rankText.setFont(font);
        rankText.setCharacterSize(22);
        rankText.setFillColor(sf::Color::Cyan);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << rankings[i].second;

        std::string line = std::to_string(i + 1) + ". " + rankings[i].first + " - " + oss.str();
        rankText.setString(line);

        float rankX = (WINDOW_WIDTH - rankText.getLocalBounds().width) / 2.f;
        rankText.setPosition(rankX, 460 + i * 30);
        window.draw(rankText);
    }
}

using namespace std;
using namespace sf;

bool isRunning = true;
bool isGamePaused = false;
bool isGameOver = false; //게임 실행, 정지, 종료 여부를 제어하는 플래그
int currentYear = 0; //현재 학년(0~3)
float lastSpawnTime = 0.f; //마지막으로 학점 생성된 시간
int gradeCollisionCount = 0;// 한 학년에서 받은 학점개수 (총 11개)
int rowCount = 0;// 생성된 행 수 
Clock gameClock; //게임 시간 측정용 SFML시계
mutex gradeMutex;//쓰레드 안전을 위한 뮤텍스

vector<Texture> gradeTextures(9);
vector<Sprite> gradeSprites(9);
Player player;
vector<float> collectedScores;
vector<float> yearAverages(4);
string currentYearAverageStr = "";

const float DROP_SPEED_BY_YEAR[4] = {4.5f, 5.5f, 8.5f, 9.0f};

void loadAssets() { //학점 파일을 텍스쳐로 로딩하고 스프라이트에 연결
    for (int i = 0; i < 9; ++i) {
        string filename = "grade_" + to_string(i + 1) + ".png";
        if (!gradeTextures[i].loadFromFile(filename)) {
            cerr << "파일 로드 실패: " << filename << endl;
            exit(-1);
        }
        gradeSprites[i].setTexture(gradeTextures[i]);
    }
}


void initPlayer() {//플레이어텍스쳐 로드 , 화면 하단 중앙에 배치
    if (!player.texture.loadFromFile("player.png")) {
        cerr << "player.png 로드 실패\n";
        exit(-1);
    }
    player.sprite.setTexture(player.texture);  // ⚠️ 중요: player가 텍스처 보관함
    player.x = WINDOW_WIDTH / 2.f - player.texture.getSize().x / 2.f;
    player.y = WINDOW_HEIGHT - 80;
}

void drawPlayer(RenderWindow& window) {//플레이어 스프라이트를 초기화된 위치에 그림
    player.sprite.setPosition(player.x, player.y);
    window.draw(player.sprite);
}

void createSingleGrade(vector<Grade>& grades, float currentTime) {
//무작위 위치에 학점 스프라이트 생성, grades 벡터에 추가함
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

/*void dropGrades(vector<Grade>& grades) {
//게임 루프에서 학점들 일정시간 간격으로 떨어드리는 역할
//현재 학년에  따라 떨어뜨릴 개수 증가(난이도 조절)
//grades 벡터에 추가되고 바닥에 도달한 학점 제거
//쓰레드에서 실행되도록 설계됨
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
}*/
float getSpawnInterval(int currentYear) {
    const float baseInterval = 1.0f;    // 초기 스폰 간격 (초)
    const float minInterval = 0.2f;     // 최소 스폰 간격 (초)
    float interval = std::max(minInterval, baseInterval * static_cast<float>(std::pow(0.85f, currentYear)));

    if (interval < minInterval)
        interval = minInterval;

    return interval;
}

void dropGrades(vector<Grade>& grades) {
    float baseInterval = 1.0f;  // 시작 간격
    float minInterval = 0.2f;   // 가장 짧은 간격
    float spawnInterval = baseInterval;

    while (isRunning) {
        float currentTime = gameClock.getElapsedTime().asSeconds();

        if (!isGamePaused && !isGameOver) {
            if (currentTime - lastSpawnTime >= spawnInterval) {
                // 학점 1개만 생성
                createSingleGrade(grades, currentTime);

                lastSpawnTime = currentTime;

                spawnInterval = getSpawnInterval(currentYear);
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
// 플레이어와 학점의 충돌 감지
// 충돌 시 해당 학점의 점수를 계산하여 저장
// 11개 충돌시 평균 계산->학년 평균 저장->게임 정지(isGamePaused = true)
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
                                //isRunning = false;
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
//좌우 키를 이용해 플레이어 이동
//엔터키로 게임 종료 또는 다음 학년 이동
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
    if (isGameOver && Keyboard::isKeyPressed(Keyboard::Enter)) {
        isRunning = false;  // 사용자가 ENTER를 누르면 종료
}
}

void resetYear(vector<Grade>& grades) {
//게임이 일시정지상태이고 엔터 키 누르면 다음 학년으로 진급(currentYear++)
//점수, 학점 등 초기화됨
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

void drawUI(sf::RenderWindow& window, sf::Font& font, std::vector<Grade>& grades) {
 //점수, ui표시담당 중간 평균 출력
    sf::Text avgText;
    avgText.setFont(font);
    avgText.setCharacterSize(25);
    avgText.setFillColor(sf::Color::White);
    avgText.setPosition(100, 200);
    avgText.setString(currentYearAverageStr); // 중간 평균 출력
    window.draw(avgText);

    if (isGamePaused && currentYear < 4 && !isGameOver && currentYear != 3 && currentYearAverageStr != "") {//다음 학년 안내
        sf::Text restartText;
        restartText.setFont(font);
        restartText.setString("Press Enter to Start Next Year");
        restartText.setCharacterSize(30);
        restartText.setFillColor(sf::Color::Yellow);
        restartText.setPosition(100, 250);
        window.draw(restartText);
    }

    if (isGameOver) { //게임 종료 시 : 각 학년의 평균 표시, GPA계산 후 rank.txt에 저장, 랭킹 출력
        // 연도별 평균
        for (int i = 0; i < 4; ++i) {
            sf::Text yearText;
            yearText.setFont(font);
            yearText.setCharacterSize(25);
            yearText.setFillColor(sf::Color::White);

            std::string avgStr = std::to_string(yearAverages[i]);
            avgStr = avgStr.substr(0, avgStr.find('.') + 3);

            yearText.setString("Year " + std::to_string(i + 1) + " Average: " + avgStr);
            float yearX = (WINDOW_WIDTH - yearText.getLocalBounds().width) / 2.f;
            yearText.setPosition(yearX, 250 + i * 40);
            window.draw(yearText);
        }

        // 최종 GPA
        float total = 0;
        for (float s : yearAverages) total += s;
        float finalAvg = total / 4;
        std::string finalStr = std::to_string(finalAvg);
        finalStr = finalStr.substr(0, finalStr.find('.') + 3);

        static bool scoreSaved = false;
        if (!scoreSaved) {
            std::ofstream out("rank.txt", std::ios::app);
            if (out.is_open()) {
                out << playerName << " " << finalAvg << '\n';
                out.close();
            }
            scoreSaved = true;
        }

        sf::Text finalText;
        finalText.setFont(font);
        finalText.setCharacterSize(28);
        finalText.setFillColor(sf::Color::Yellow);
        finalText.setString(" Final GPA: " + finalStr);
        float finalX = (WINDOW_WIDTH - finalText.getLocalBounds().width) / 2.f;
        finalText.setPosition(finalX, 250 + 4 * 40 + 20);
        window.draw(finalText);

        drawRanking(window, font); //rank.txt에서 상위 5명  랭킹 불러와서 내림차순 출력
 
    }
}




