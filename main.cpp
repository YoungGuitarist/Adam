#include <chrono>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <fstream>

using namespace std;

// Params
const int FRAME_DELAY_MS = 10;   // 10 мс задержки между кадрами
const int TARGET_FPS = 100;      // 100 FPS (1000 мс / 10 мс)
const int DAY_DURATION_SEC = 10; // 60 секунд в сутках
const int TARGET_FRAMES_PER_DAY = DAY_DURATION_SEC * TARGET_FPS; // 6000 кадров
int TICK_PER_SECOND = TARGET_FPS;

int COUNT_OF_DEAD_UNITS = 0;
int REDUCE_DELAY = 0;
int CURRENT_DAY = 0;
int COUNT_OF_DAYS = 10;
int HEIGHT = 30, WIDTH = 60;
int MAX_UNITS = 3;
char WORLD_ICON = '.';
int GLOBAL_TIME = 0;

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
  int hp;

  string genomeInString = " ";
};

//Genome makeGenome(Genome genome = {}) {
//    string stringGenome;
//    if (genome.genomeInString == " ") {
//        string stringGenome = to_string(randFunc(1111, 9999));
//    }
//    genome.hp = stoi(stringGenome.substr(0, 2));
//    genome.speed = stoi(stringGenome.substr(3, 4));
//
//    return genome;
//}


class Unit {
public:
  int x, y, lastX, lastY, id;
  string color = PURPLE;
  char icon = 'O';
  Genome genome;
  double hp = genome.hp;
  bool isDead;
  int coolDown = genome.speed;

  Unit(int x, int y, Genome genome, int id)
      : x(x), y(y), genome(genome), isDead(false), id(id) {};

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
   vector<Unit> deadUnits;
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
    deadUnits.clear();
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

  Genome initializeGenome() { return {100, 100}; };

  void initializeUnits() {
    units.clear();
    for (int i = 0; i < MAX_UNITS; i++) {
      units.push_back(Unit(randFunc(0, WIDTH - 1), randFunc(0, HEIGHT - 1),
                           initializeGenome(), i));
    }
  }

  void drawUnits() {
    for (int i = 0; i < units.size(); i++) {
      if (!units[i].isDead) {
        setCell(units[i].lastX, units[i].lastY, WORLD_ICON);
        setCell(units[i].x, units[i].y, units[i].icon, units[i].color);
      } 
    }
    for (Unit unit : deadUnits) {
      setCell(unit.x, unit.y, unit.icon, unit.color);
    }
  }

  void processUnit(Unit &unit) {
    if (REDUCE_DELAY == TICK_PER_SECOND - 1) {
      unit.hp = unit.hp - 10;
      REDUCE_DELAY = 0;
    } else {
      REDUCE_DELAY++;
    }
    char c = getCell(unit.x, unit.y);
    if (c == food.icon) {
      unit.hp += 12;
      clearCell(unit.x, unit.y);
    } else if (c == poison.icon) {
      unit.hp -= 15;
      clearCell(unit.x, unit.y);
    }

    if (unit.hp <= 0) {
      unit.isDead = true;
      unit.icon = 'X';
      unit.color = YELLOW;
      deadUnits.push_back(unit);
    }

    for (int i = 0; i < units.size(); i++) {
        if (units[i].isDead) {
            units.erase(units.begin() + i);
        }
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
  }

  // Logging functions
  void Logging(vector<Unit> MassiveForLogging) {
      ofstream outFile("Log.txt", ios::app);

      if (outFile.is_open()) {
          outFile << "Current day: " << CURRENT_DAY << "\n";
          for (auto &unit : MassiveForLogging) {
              outFile << "Unit " << unit.id << ":" << "\n" << "   "
                  << "Day of death: " << CURRENT_DAY << "\n" << "   "
                  << "Current HP: " << unit.hp<< "\n" << "   " 
                  << "Genome: " << "\n" << "      "
                  << "HP:" << unit.genome.hp << "\n" << "      "
                  << "SPEED: " << unit.genome.speed << "\n";
          };
          outFile << "------------------------------------------------------- \n";
      }
      outFile.close();
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

    world.Logging(world.deadUnits);

    CURRENT_DAY++;
  }

    world.clearAll();
    cout << "\nSimulation ended, logs are ready!";
  return 0;
}
