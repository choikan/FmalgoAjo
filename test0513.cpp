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
constexpr int GRADES_PER_ROW = 5;           // 한 줄에 떨어질 학점 개수
constexpr int GRADE_WIDTH = 140;            // 학점 이미지 가로 크기
constexpr int GRADE_HEIGHT = 70;            // 학점 이미지 세로 크기
constexpr int GRADE_SPACING_X = 140;        // 학점 사이 간격
constexpr int WINDOW_WIDTH = GRADES_PER_ROW * GRADE_SPACING_X; // 창 가로 크기
constexpr int WINDOW_HEIGHT = 600;          // 창 세로 크기
constexpr float DROP_SPEED = 5.0f;          // 학점 낙하 속도 (프레임마다 증가하는 y 값)
constexpr float TIME_BETWEEN_ROWS = 0.7f;   // 줄 사이 생성 시간 간격 (초)
constexpr float PLAYER_SPEED = 10.0f;       // 플레이어 좌우 이동 속도

// --- 전역 변수 ---
bool isRunning = true;                      // 게임 루프 실행 여부
vector<Texture> gradeTextures(9);           // 9개의 학점 텍스처 저장
vector<Sprite> gradeSprites(9);             // 텍스처로부터 만든 스프라이트
mutex gradeMutex;                           // 학점 벡터 접근을 위한 뮤텍스
Clock gameClock;
int rowCount = 0;

vector<float> collectedScores;  // 받은 학점 점수 저장
int gradeCollisionCount = 0;    // 충돌 횟수
int currentYear = 0;
vector<float> yearAverages(4);
bool isGamePaused = false;
bool isGameOver = false;
string currentYearAverageStr = "";  // 🔁 매 학년 끝날 때 저장용


// --- 구조체 정의 ---

// 학점 오브젝트 구조
struct Grade {
    Sprite sprite;        // 그릴 이미지
    int x, y;             // 현재 위치
    bool active = false;  // true면 현재 떨어지고 있음
    float activatedTime = 0.f; // 떨어지기 시작할 시간
};

// 플레이어 구조
struct Player {
    Sprite sprite;
    float x, y;
};

Player player; // 전역 플레이어

// --- 함수 정의 ---

// 플레이어를 화면에 그림
void drawPlayer(RenderWindow& window) {
    player.sprite.setPosition(player.x, player.y);
    window.draw(player.sprite);
}

// 한 줄의 학점들을 생성하여 grades 벡터에 추가
void CreateRow(vector<Grade>& grades, int rowIndex, float activatedTime) {
    for (int i = 0; i < GRADES_PER_ROW; ++i) {
        Grade g;

        // 학점 텍스처 순환적으로 선택
        g.sprite = gradeSprites[(rowIndex * GRADES_PER_ROW + i) % gradeSprites.size()];

        // 짝수 줄은 정렬, 홀수 줄은 반 칸 오른쪽으로 오프셋
        int offset = (rowIndex % 2 == 0) ? 0 : GRADE_WIDTH / 2;

        g.x = i * GRADE_SPACING_X + offset;
        g.y = 0; // y=0에서 시작
        g.activatedTime = activatedTime;

        lock_guard<mutex> lock(gradeMutex);
        grades.push_back(g);
    }
}

// 학점을 일정 시간마다 생성하고 떨어뜨리는 스레드 함수
void dropGrades(vector<Grade>& grades) {

    while (isRunning) {
        float currentTime = gameClock.getElapsedTime().asSeconds();

        if (!isGamePaused && !isGameOver) {
            // TIME_BETWEEN_ROWS 초마다 한 줄 생성
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

   
// 플레이어와 충돌한 학점 제거
void checkCollisions(vector<Grade>& grades) {
    FloatRect playerBounds = player.sprite.getGlobalBounds(); // 플레이어의 경계

    grades.erase(
        remove_if(grades.begin(), grades.end(), [&](const Grade& g) {
            // 활성화된 학점 && 충돌한 경우
            if (g.active && g.sprite.getGlobalBounds().intersects(playerBounds)) {
                // 어떤 학점 이미지인지 찾기
                for (int i = 0; i < gradeSprites.size(); ++i) {
                    if (g.sprite.getTexture() == gradeSprites[i].getTexture()) {
                        float score = 4.5f - i * 0.5f; // grade_1.png = 4.5, ..., grade_9.png = 0.0
                        collectedScores.push_back(score);
                        gradeCollisionCount++;

                        cout << "충돌한 학점: grade_" << (i + 1) << " → " << score << "점" << endl;

                        // 11개 학점과 충돌하면 평균 계산 + 일시 정지
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

                            // 마지막 4학년이면 게임 종료
                            if (currentYear == 3) {
                                isGameOver = true;
                                isRunning = false; // dropThread도 종료
                            }
                        }

                        break; // 찾았으면 반복 종료
                    }
                }

                return true; // 충돌한 학점은 제거
            }
            return false;
        }),
        grades.end()
    );
}

// --- 메인 함수 ---
int main() {
    srand(static_cast<unsigned int>(time(nullptr))); // 랜덤 초기화
Font font;
if (!font.loadFromFile("NotoSansKR-VariableFont_wght.ttf")) {
    cerr << "폰트 로딩 실패!" << endl;
}

Text restartText;
restartText.setFont(font);
restartText.setString("Press Enter to Start Next Year");
restartText.setCharacterSize(30);
restartText.setFillColor(Color::Yellow);
restartText.setPosition(100, 250);


    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "FMALGOAJO");
    window.setFramerateLimit(60); // 화면 FPS 제한

    // 학점 텍스처 로딩 (grade_1.png ~ grade_9.png)
    for (int i = 0; i < 9; ++i) {
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

    vector<Grade> grades; // 화면에 존재하는 학점 리스트
    thread dropThread(dropGrades, ref(grades)); // 학점 낙하 스레드 시작

    // 게임 루프
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                isRunning = false; // 낙하 스레드 종료 플래그
                window.close();
            }
        }

        // --- 키보드 입력 처리 (좌우 이동) ---
	if (!isGamePaused && !isGameOver) {
    	if (Keyboard::isKeyPressed(Keyboard::Left)) {
      	  player.x -= PLAYER_SPEED;
      	  if (player.x < 0) player.x = 0;
   	}
    	if (Keyboard::isKeyPressed(Keyboard::Right)) {
        player.x += PLAYER_SPEED;
        if (player.x + 	player.sprite.getGlobalBounds().width > WINDOW_WIDTH)
           player.x = WINDOW_WIDTH -player.sprite.getGlobalBounds().width;
    }
}

// --- Enter 눌러서 다음 학년 시작 ---
if (isGamePaused && Keyboard::isKeyPressed(Keyboard::Enter)) {
    currentYear++;
    isGamePaused = false;
    
    gameClock.restart();  
    rowCount = 0;    
    
    currentYearAverageStr = "";

    lock_guard<mutex> lock(gradeMutex);
    grades.clear(); // 기존 학점 제거
}


        // --- 충돌 검사 ---
        {
            lock_guard<mutex> lock(gradeMutex);
            checkCollisions(grades); // 충돌한 학점 제거
        }

        // --- 화면 그리기 ---
        window.clear(Color::Black);

        {
            lock_guard<mutex> lock(gradeMutex);
            for (auto& g : grades) {
                window.draw(g.sprite); // 학점 그리기
            }
        }

        drawPlayer(window); // 플레이어 그리기

	// 평균 출력용 텍스트
	if (isGamePaused && currentYear < 4 && !isGameOver && currentYearAverageStr != "" && currentYear != 3) {
    	window.draw(restartText);  // Enter 누르라는 메시지
	}
	Text avgText;
  	avgText.setFont(font);
  	avgText.setCharacterSize(25);
  	avgText.setFillColor(Color::White);
  	avgText.setPosition(100, 200);
  	avgText.setString(currentYearAverageStr);  // 전역 변수로 선언된 값
    window.draw(avgText);
	
	// --- 최종 결과 표시 ---
if (isGameOver) {
    for (int i = 0; i < 4; ++i) {
        Text yearText;
        yearText.setFont(font);
        yearText.setCharacterSize(25);
        yearText.setFillColor(Color::White);
        yearText.setPosition(100, 300 + i * 40);  // 각 줄 간격

        string avgStr = to_string(yearAverages[i]);
        avgStr = avgStr.substr(0, avgStr.find('.') + 3); // 소수점 둘째자리까지 자르기 (예: 2.40)

        yearText.setString("Year " + to_string(i + 1) + " Average: " + avgStr);
        window.draw(yearText);
    }

    // 전체 평균 출력 (선택 사항)
    float total = 0;
    for (float s : yearAverages) total += s;
    float finalAvg = total / 4;
    string finalStr = to_string(finalAvg);
    finalStr = finalStr.substr(0, finalStr.find('.') + 3);

    Text finalText;
    finalText.setFont(font);
    finalText.setCharacterSize(28);
    finalText.setFillColor(Color::Yellow);
    finalText.setPosition(100, 300 + 4 * 40 + 20);  // 마지막 줄 밑에 출력

    finalText.setString(" Final GPA: " + finalStr);
    window.draw(finalText);
}

        window.display();
    }

    dropThread.join(); // 스레드 종료 대기

    return 0;
}

