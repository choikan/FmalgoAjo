#include <iostream>
#include <windows.h>
#include <conio.h>

using namespace std;

constexpr int KEY_ESC = 27;
constexpr int KEY_SPACE = 32;

struct location {
    int x = 10;
    int y = 20;
};

int GetKeyDown() {
    if (_kbhit()) return _getch();
    return 0;
}

void CursorSettings() {
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void SetKeyCursor(int x, int y) {
    COORD cursorPos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
}

void DrawPlayer(location& L, int nX, int nY) {
    SetKeyCursor(L.x, L.y);
    cout << " ";

    L.x += nX;
    L.y -= nY;  // y는 위로 갈수록 작아지므로 빼기
    SetKeyCursor(L.x, L.y);
    cout << "@";
}

int GameStart(location& L) {
    DrawPlayer(L, 0, 0);
    int key;

    while (true) {
        key = GetKeyDown();
        switch (key) {
        case KEY_ESC: return -1;
        case 'a': DrawPlayer(L, -2, 0); break;
        case 'd': DrawPlayer(L, 2, 0); break;
        case 'w': DrawPlayer(L, 0, 1); break;
        case 's': DrawPlayer(L, 0, -1); break;
        default: break;
        }
        Sleep(50);
    }
    return 0;
}

int main() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD bufferSize = { 100, 50 };
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    SMALL_RECT windowSize = { 0, 0, 99, 49 };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    location L;
    CursorSettings();
    GameStart(L);

    return 0;
}