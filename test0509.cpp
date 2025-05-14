#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <iostream>
#include <mutex>

using namespace std;
using namespace sf;


// --- 상수 정의 ---
constexpr int GRADES_PER_ROW = 5;
constexpr int GRADE_WIDTH = 140;
constexpr int GRADE_HEIGHT = 70;
constexpr int GRADE_SPACING_X = 140;
constexpr int WINDOW_WIDTH = GRADES_PER_ROW * GRADE_SPACING_X;
constexpr int WINDOW_HEIGHT = 600;
constexpr float DROP_SPEED = 5.0f; // 픽셀/프레임
constexpr float TIME_BETWEEN_ROWS = 0.7f; // 줄 간격 시간 (초)

// --- 전역 변수 ---
bool isRunning = true;
vector<Texture> gradeTextures(6);
vector<Sprite> gradeSprites(6);
mutex gradeMutex; // 스레드 안전성 확보용

// --- 구조체 정의 ---
struct Grade {
    Sprite sprite;
    int x, y;
    bool active = false;
    float activatedTime = 0.f;
};

struct Player {
    Sprite sprite;
    int x, y;
};

Player player;

// --- 함수 정의 ---

void drawPlayer(RenderWindow& window) {
    player.sprite.setPosition(player.x, player.y);
    window.draw(player.sprite);
}

// 학점 한 줄 생성 함수
void CreateRow(vector<Grade>& grades, int rowIndex, float activatedTime) {
    for (int i = 0; i < GRADES_PER_ROW; ++i) {
        Grade g;
        g.sprite = gradeSprites[(rowIndex * GRADES_PER_ROW + i) % 6];

        // 짝수 줄은 정렬, 홀수 줄은 쉬프트
        int offset = (rowIndex % 2 == 0) ? 0 : GRADE_WIDTH / 2;
        g.x = i * GRADE_SPACING_X + offset;
        g.y = 0;

        g.activatedTime = activatedTime;

        lock_guard<mutex> lock(gradeMutex);
        grades.push_back(g);
    }
}

// 학점 떨어뜨리기 스레드
void dropGrades(vector<Grade>& grades) {
    Clock gameClock;
    int rowCount = 0;

    while (isRunning) {
        float currentTime = gameClock.getElapsedTime().asSeconds();

        // 새로운 줄 추가 (시간 간격에 따라)
        if (currentTime >= rowCount * TIME_BETWEEN_ROWS) {
            CreateRow(grades, rowCount, currentTime);
            ++rowCount;
        }

        {
            lock_guard<mutex> lock(gradeMutex);

            // 학점 이동 및 활성화 처리
            for (auto& g : grades) {
                if (!g.active && currentTime >= g.activatedTime) {
                    g.active = true;
                }
                if (g.active) {
                    g.y += DROP_SPEED;
                    g.sprite.setPosition(g.x, g.y);
                }
            }

            // 바닥에 도달한 학점 제거
            grades.erase(
                remove_if(grades.begin(), grades.end(), [](const Grade& g) {
                    return g.active && g.y >= WINDOW_HEIGHT - GRADE_HEIGHT;
                }),
                grades.end()
            );
        }

        this_thread::sleep_for(chrono::milliseconds(30));
    }
}


int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "FMALGOAJO");
    window.setFramerateLimit(60);

    // 학점 텍스처 로딩
    for (int i = 0; i < 6; ++i) {
        string filename = "grade_" + to_string(i + 1) + ".png";
        if (!gradeTextures[i].loadFromFile(filename)) {
            cerr << "파일 로드 실패: " << filename << endl;
            return -1;
        }
        gradeSprites[i].setTexture(gradeTextures[i]);
    }

    // 플레이어 텍스처 로딩
    Texture playerTexture;
    if (!playerTexture.loadFromFile("player.png")) {
        cerr << "player.png 로드 실패\n";
        return -1;
    }
    player.sprite.setTexture(playerTexture);
    player.x = WINDOW_WIDTH / 2;
    player.y = WINDOW_HEIGHT - 80;

    // 학점 벡터
    vector<Grade> grades;

    // 스레드 시작
    thread dropThread(dropGrades, ref(grades));

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                isRunning = false;
                window.close();
            }
        }

        window.clear(Color::Black);

        // 학점 그리기
        {
            lock_guard<mutex> lock(gradeMutex);
            for (auto& g : grades) {
                window.draw(g.sprite);
            }
        }

        drawPlayer(window);

        window.display();
    }

    dropThread.join();

    return 0;
}

