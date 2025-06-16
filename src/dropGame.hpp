#ifndef DROPGAME_HPP
#define DROPGAME_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <mutex>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const int GRADES_PER_ROW = 8;
const int GRADE_SPACING_X = WINDOW_WIDTH / GRADES_PER_ROW;
const int GRADE_HEIGHT = 55;

const float PLAYER_SPEED = 5.0f;

const float MIN_SPAWN_INTERVAL = 0.4f;
const float MAX_SPAWN_INTERVAL = 1.0f;

struct Grade {
    sf::Sprite sprite;
    float x, y;
    bool active;
    float activatedTime;
};

struct Player {
    sf::Sprite sprite;
    sf::Texture texture;
    float x, y;
};

extern bool isRunning;
extern bool isGamePaused;
extern bool isGameOver;
extern int currentYear;
extern float lastSpawnTime;
extern int gradeCollisionCount;
extern int rowCount;

extern std::mutex gradeMutex;

extern std::vector<sf::Texture> gradeTextures;
extern std::vector<sf::Sprite> gradeSprites;
extern Player player;
extern std::vector<float> collectedScores;
extern std::vector<float> yearAverages;
extern std::string currentYearAverageStr;

void loadAssets();
void initPlayer();
void createSingleGrade(std::vector<Grade>& grades, float currentTime);
void dropGrades(std::vector<Grade>& grades);
void checkCollisions(std::vector<Grade>& grades);
void handleInput(sf::RenderWindow& window);
void resetYear(std::vector<Grade>& grades);
void drawPlayer(sf::RenderWindow& window);
void drawUI(sf::RenderWindow& window, sf::Font& font, std::vector<Grade>& grades);

#endif // DROPGAME_HPP

