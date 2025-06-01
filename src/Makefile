CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -pthread

SRC = main.cpp keyboard.cpp screenStart.cpp screenRule.cpp screenGrade.cpp \
      countdown.cpp playGrade.cpp screenGraderesult.cpp screenEnd.cpp \
      dropGame.cpp

OBJ = $(SRC:.cpp=.o)
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

