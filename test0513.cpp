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
constexpr float DROP_SPEED = 5.0f;
constexpr float TIME_BETWEEN_ROWS = 0.7f;
constexpr float PLAYER_SPEED = 10.0f; // 플레이어 이동 속도

// --- 전역 변수 ---
bool isRunning = true;
vector<Texture> gradeTextures(6);
vector<Sprite> gradeSprites(6);
mutex gradeMutex;

// --- 구조체 정의 ---
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

// --- 함수 정의 ---

void drawPlayer(RenderWindow& window) {
    player.sprite.setPosition(player.x, player.y);
    window.draw(player.sprite);
}

// 한 줄 생성
void CreateRow(vector<Grade>& grades, int rowIndex, float activatedTime) {
    for (int i = 0; i < GRADES_PER_ROW; ++i) {
        Grade g;
        g.sprite = gradeSprites[(rowIndex * GRADES_PER_ROW + i) % 6];

        int offset = (rowIndex % 2 == 0) ? 0 : GRADE_WIDTH / 2;
        g.x = i * GRADE_SPACING_X + offset;
        g.y = 0;
        g.activatedTime = activatedTime;

        lock_guard<mutex> lock(gradeMutex);
        grades.push_back(g);
    }
}

// 학점 떨어뜨리기
void dropGrades(vector<Grade>& grades) {
    Clock gameClock;
    int rowCount = 0;

    while (isRunning) {
        float currentTime = gameClock.getElapsedTime().asSeconds();

        if (currentTime >= rowCount * TIME_BETWEEN_ROWS) {
            CreateRow(grades, rowCount, currentTime);
            ++rowCount;
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

            // 바닥 도달한 학점 제거
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

// 충돌 검사
void checkCollisions(vector<Grade>& grades) {
    FloatRect playerBounds = player.sprite.getGlobalBounds();

    grades.erase(
        remove_if(grades.begin(), grades.end(), [&](const Grade& g) {
            if (g.active && g.sprite.getGlobalBounds().intersects(playerBounds)) {
                cout << "충돌!" << endl;
                return true; // 충돌하면 제거
            }
            return false;
        }),
        grades.end()
    );
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

        // 키보드 입력 처리
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            player.x -= PLAYER_SPEED;
            if (player.x < 0) player.x = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) {
            player.x += PLAYER_SPEED;
            if (player.x + player.sprite.getGlobalBounds().width > WINDOW_WIDTH)
                player.x = WINDOW_WIDTH - player.sprite.getGlobalBounds().width;
        }

        // 충돌 검사
        {
            lock_guard<mutex> lock(gradeMutex);
            checkCollisions(grades);
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


