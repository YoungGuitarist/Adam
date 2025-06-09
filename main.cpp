#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std;

// Params

const int FRAME_DELAY_MS = 10;   // 10 мс задержки между кадрами
const int TARGET_FPS = 100;      // 100 FPS (1000 мс / 10 мс)
const int DAY_DURATION_SEC = 60; // 60 секунд в сутках
const int TARGET_FRAMES_PER_DAY = DAY_DURATION_SEC * TARGET_FPS; // 6000 кадров

int CURRENT_DAY = 0;
int COUNT_OF_DAYS = 10;
int HEIGHT = 30, WIDTH = 60;
int MAX_UNITS = 10;
char WORLD_ICON = '.';
double GLOBAL_TIME = 0;
int FRAME_DELAY = 10;

string GREEN = "\033[32m";
string RED = "\033[31m";
string YELLOW = "\033[33m";
string PURPLE = "\033[95m";
string RESET_COLOR = "\033[0m";

int min_count_of_food = HEIGHT * WIDTH * 0.05;
int min_count_of_poison = HEIGHT * WIDTH * 0.02;

int randFunc(int min, int max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  uniform_int_distribution<int> distrib(min, max);
  return distrib(gen);
}

struct Cell {
  char icon = WORLD_ICON;
  string color = "";
};
struct Food {
  string color = GREEN;
  char icon = '@';
};
struct Poison {
  string color = RED;
  char icon = '#';
};

struct Genome {
  int speed;
  double hp = 100;
};

class Unit {
public:
  int x, y, lastX, lastY;
  string color = PURPLE;
  char icon = 'O';
  Genome genome;
  double hp = genome.hp;
  bool isDead;
  int coolDown = genome.speed;

  Unit(int x, int y, Genome genome)
      : x(x), y(y), genome(genome), isDead(false) {};

  void move(int direction, int width, int height) {
    lastX = x;
    lastY = y;

    if (coolDown == 0) {
      switch (direction) {
      case 0:
        y = (y - 1 + height) % height;
        break;
      case 1:
        x = (x + 1) % width;
        y = (y - 1 + height) % height;
        break;
      case 2:
        x = (x + 1) % width;
        break;
      case 3:
        x = (x + 1) % width;
        y = (y + 1) % height;
        break;
      case 4:
        y = (y + 1) % height;
        break;
      case 5:
        x = (x - 1 + width) % width;
        y = (y + 1) % height;
        break;
      case 6:
        x = (x - 1 + width) % width;
        break;
      case 7:
        x = (x - 1 + width) % width;
        y = (y - 1 + height) % height;
        break;
      }
      coolDown = genome.speed;
    } else {
      coolDown--;
    }

    if (hp <= 0) {
      isDead = true;
      icon = 'X';
      color = YELLOW;
    }
  };
};

class World {
private:
  int height, width;
  vector<vector<Cell>> grid;
  string buffer;
  Poison poison;
  Food food;
  vector<Unit> units;

  void cleanBuffer() {
    cout << "\033[2J\033[H"; // Очистка экрана
  }

public:
  World(int w, int h) : width(w), height(h) {
    grid = vector<vector<Cell>>(height, vector<Cell>(width));
  }

  void setCell(int x, int y, char c, const string &color = "") {
    if (x >= 0 && x < width && y >= 0 && y < height) {
      grid[y][x] = {c, color};
    }
  }

  void clearCell(int x, int y) { setCell(x, y, '.', ""); }

  char getCell(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
      return grid[y][x].icon;
    }
    return WORLD_ICON;
  }

  void clearAll() {
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        setCell(x, y, WORLD_ICON, "");
      }
    }
  }

  void drawWorld() {
    // Спавн еды
    int foodCount = randFunc(min_count_of_food, min_count_of_food * 1.5);
    for (int i = 0; i < foodCount; i++) {
      int x = randFunc(0, WIDTH - 1);
      int y = randFunc(0, HEIGHT - 1);
      if (getCell(x, y) == WORLD_ICON) {
        setCell(x, y, food.icon, food.color);
      }
    }

    // Спавн яда
    int poisonCount = randFunc(min_count_of_poison, min_count_of_poison * 1.5);
    for (int i = 0; i < poisonCount; i++) {
      int x = randFunc(0, WIDTH - 1);
      int y = randFunc(0, HEIGHT - 1);
      if (getCell(x, y) == WORLD_ICON) {
        setCell(x, y, poison.icon, poison.color);
      }
    }
  }

  Genome initializeGenome() { return {randFunc(0, 99)}; }

  void initializeUnits() {
    units.clear();
    for (int i = 0; i < MAX_UNITS; i++) {
      units.push_back(Unit(randFunc(0, WIDTH - 1), randFunc(0, HEIGHT - 1),
                           initializeGenome()));
    }
  }

  void drawUnits() {
    for (int i = 0; i < units.size(); i++) {
      if (!units[i].isDead) {
        setCell(units[i].lastX, units[i].lastY, WORLD_ICON);
        setCell(units[i].x, units[i].y, units[i].icon, units[i].color);
      } else {
        setCell(units[i].lastX, units[i].lastY, WORLD_ICON);
        setCell(units[i].x, units[i].y, units[i].icon, units[i].color);
      }
    }
  }

  void processUnit(Unit &unit) {
    unit.hp = unit.hp;
    char c = getCell(unit.x, unit.y);
    if (c == food.icon) {
      unit.hp += 10;
      clearCell(unit.x, unit.y);
    } else if (c == poison.icon) {
      unit.hp -= 15;
      clearCell(unit.x, unit.y);
    }

    if (unit.hp <= 0) {
      unit.isDead = true;
      unit.icon = 'X';
      unit.color = YELLOW;
    }
  }

  void updateBuffer() {
    buffer.clear();
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        const Cell &cell = grid[y][x];
        if (cell.color.empty()) {
          buffer += cell.icon;
        } else {
          buffer += cell.color;
          buffer += cell.icon;
          buffer += RESET_COLOR;
        }
      }
      buffer += '\n';
    }
  }

  void render() {
    cleanBuffer();
    updateBuffer();
    cout << buffer << flush;
  }

  vector<Unit> &getUnits() { return units; }
  void action() {

    for (auto &unit : getUnits()) {
      if (!unit.isDead) {
        unit.move(randFunc(0, 7), WIDTH, HEIGHT);
        processUnit(unit);
      }
    }

    drawUnits();
    render();
    /*this_thread::sleep_for(chrono::milliseconds(FRAME_DELAY));
    GLOBAL_TIME++;*/
  }
};

int main() {
  World world(WIDTH, HEIGHT);

  while (CURRENT_DAY < COUNT_OF_DAYS) {
    world.initializeUnits();
    world.clearAll();
    world.drawWorld();

    GLOBAL_TIME = 0; // Сброс времени дня
    auto day_start = std::chrono::steady_clock::now();

    while (GLOBAL_TIME < TARGET_FRAMES_PER_DAY) {
      auto frame_start = std::chrono::steady_clock::now();
      world.action();
      GLOBAL_TIME++;

      auto frame_end = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                         frame_end - frame_start)
                         .count();
      int remaining_delay = FRAME_DELAY_MS - static_cast<int>(elapsed);

      // Корректировка задержки
      if (remaining_delay > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(remaining_delay));
      }
    }

    CURRENT_DAY++;
  }
  return 0;
}
