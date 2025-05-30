#include "playGrade.hpp"
#include "countdown.hpp"
#include "screenGrade.hpp"
#include <string>

int playGrade(sf::RenderWindow& window) {
    for (int year = 1; year <= 4 && window.isOpen(); ++year) {
        showCountdown(window, std::to_string(year) + " grade");
        int res = screenGrade(window, year);
        if (res == -1) {
            window.close();
            return -1;
        }
    }
    return 0;
}
