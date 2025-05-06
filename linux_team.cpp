#include <iostream>
#include <windows.h>
#include <conio.h>

using namespace std;

constexpr int KEY_ESC = 27;           // ESC 키
constexpr int KEY_SPACE = 32;         // SpaceBar 키
constexpr int MAX_JUMP = 6;           // 최대 점프 높이
constexpr int X_BASE = 10;            // 초기 Y축 위치
constexpr int Y_BASE = 60;            // 초기 Y축 위치


int GetKeyDown()
{
	if (_kbhit() != 0)
		return _getch();

	return 0;
}

void CursorSettings()
{
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void SetKeyCursor(int _nX, int _nY)
{
	COORD cursorPos = { (SHORT)_nX, (SHORT)_nY };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
}

void DrawDino(int nx, int nY)
{
	SetKeyCursor(X_BASE + nx, Y_BASE - nY);       // 플레이어의 초기 위치에서 이동
	cout << "@";
}

int GameStart()
{
	bool IsJumped = false;
	DrawDino(0, 0);

	int key = -1;
	while (1)
	{
		key = GetKeyDown();
		switch (key)
		{
		case KEY_ESC:
			return -1;
		case KEY_SPACE:
			IsJumped = true;
			break;
		case 'a':
			DrawDino(3, 0);
			break;
		case 'd':
			DrawDino(-3, 0);
			break;
		case 'w' :
			DrawDino(0, 3);
			break;
		case 's' :
			DrawDino(0, -3);
			break;
		default:
			DrawDino(0, 0);
			break;
		}
	}

	return 0;
}


int main() {
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// 1. 버퍼 크기 설정
	COORD bufferSize = { 100, 50 };
	SetConsoleScreenBufferSize(hConsole, bufferSize);
	// 2. 콘솔 창 크기 설정
	SMALL_RECT windowSize = { 0, 0, 99, 49 }; 
	SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

	CursorSettings();
	GameStart();

	return 0;
}