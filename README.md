# Snake_game

국민대학교 C++ 수업에서 진행하는 과제인 스네이크 게임 만들기 입니다.


---

Snake Game은 터미널에서 동작하는 간단한 뱀 게임입니다. 이 프로젝트는 C++과 ncurses 라이브러리를 사용하여 구현되었습니다.

## 설치 및 실행 방법

### 요구 사항

- C++ 컴파일러 (C++11 이상)
- ncurses 라이브러리

### 설치

1. **C++ 컴파일러 설치**: 시스템에 맞는 C++ 컴파일러를 설치합니다.
   
2. **ncurses 라이브러리 설치**: 
   - Ubuntu 기반 시스템: `sudo apt-get install libncurses5-dev libncursesw5-dev`
   - Fedora 기반 시스템: `sudo dnf install ncurses-devel`
   - macOS: `brew install ncurses`

### 빌드 및 실행

1. 소스 코드를 클론합니다:
   ```bash
   git clone <repository-url>
   cd snake-game
   ```

2. 프로젝트를 빌드합니다:
   ```bash
   g++ -std=c++11 -o snake_game main.cpp -lncurses
   ```

3. 빌드된 실행 파일을 실행합니다:
   ```bash
   ./snake_game
   ```

## 게임 설명

이 뱀 게임은 터미널에서 실행되며, 사용자는 방향키를 사용하여 뱀을 조종합니다. 뱀은 맵을 가로질러 이동하며, 벽과 장애물에 충돌하거나 자신의 몸에 부딪히면 게임이 종료됩니다. 몸 길이를 늘리는 아이템을 먹거나 독을 피하는 것이 목표입니다. 또한, 게이트를 통해 길을 최단 경로로 이동할 수 있습니다.

## 기능 및 구현

- **Snake 클래스**: 뱀의 상태와 이동을 관리합니다.
- **GameMap 클래스**: 맵을 초기화하고 그리는 기능을 제공합니다. 벽, 장애물, 아이템, 게이트의 위치를 설정하고 충돌을 검사합니다.
- **Item 클래스**: 아이템의 종류와 위치를 관리하며, 뱀이 아이템을 먹었을 때 새로운 아이템을 생성합니다.
- **Gate 클래스**: 게이트의 입구와 출구를 설정하고, 뱀이 게이트를 통과하여 다른 위치로 이동할 수 있습니다.

## 추가 기능

- **점수 계산**: 아이템을 먹고 게이트를 통과하는 등의 동작에 따라 점수를 계산합니다.
- **난이도 설정**: 게임의 속도와 아이템 출현 빈도 등을 조절하여 난이도를 설정할 수 있습니다.

---

1 단계 

### Map에 대한 설계사항

#### 1. Map의 구성 요소

**Data 처리 방법**
- **맵 구조**: 맵은 2차원 그리드 형태로 구성되며, 각 셀은 특정 객체(벽, 장애물, 아이템 등)을 포함할 수 있습니다.
- **객체 정보**: 각 객체는 좌표와 종류 정보를 가지며, 벽, 장애물, 아이템, 게이트 등 다양한 종류가 있습니다.
- **데이터 구조**: Map 클래스는 벡터 또는 다른 적절한 자료구조를 사용하여 객체들의 위치와 종류를 관리합니다.

**Map Data Import/Export**
- **Export**: 맵 데이터를 파일로 내보낼 수 있는 기능을 제공합니다. 이는 텍스트 파일이나 JSON 파일 형식으로 저장할 수 있습니다.
- **Import**: 저장된 맵 데이터 파일을 읽어와서 게임에 로드하는 기능을 제공합니다. 이를 통해 사용자는 맵을 수정하고 저장할 수 있습니다.

#### 예시 코드 (간략화된 예시)

```cpp
#include <ncurses.h>
#include <unistd.h>
#include <vector>

using namespace std;

const int FIELD_WIDTH = 21;
const int FIELD_HEIGHT = 21;

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

class SnakeGameMap {
public:
    SnakeGameMap();
    void draw();

private:
    WINDOW *gameWin;
    vector<Point> snake;
    Point growthItem;
    Point poisonItem;
    Point gateIn;
    Point gateOut;
    vector<Point> walls;

    void init();
    bool isSnake(Point p);
};

SnakeGameMap::SnakeGameMap() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    gameWin = newwin(FIELD_HEIGHT, FIELD_WIDTH, 0, 0);
    init();
}

void SnakeGameMap::init() {
    snake.push_back(Point(FIELD_WIDTH / 2, FIELD_HEIGHT / 2));
    snake.push_back(Point(FIELD_WIDTH / 2, FIELD_HEIGHT / 2 + 1));
    snake.push_back(Point(FIELD_WIDTH / 2, FIELD_HEIGHT / 2 + 2));
    growthItem = Point(2, 2);
    poisonItem = Point(4, 4);
    gateIn = Point(6, 6);
    gateOut = Point(8, 8);

    // 가로 및 세로 벽
    for (int i = 0; i < FIELD_WIDTH; ++i) {
        walls.push_back(Point(i, 0));
        walls.push_back(Point(i, FIELD_HEIGHT - 1));
    }
    for (int i = 1; i < FIELD_HEIGHT - 1; ++i) {
        walls.push_back(Point(0, i));
        walls.push_back(Point(FIELD_WIDTH - 1, i));
    }

    // 겉 테두리의 꼭짓점에 Immune Wall 추가
    walls.push_back(Point(0, 0));                      // 좌상단
    walls.push_back(Point(FIELD_WIDTH - 1, 0));         // 우상단
    walls.push_back(Point(0, FIELD_HEIGHT - 1));       // 좌하단
    walls.push_back(Point(FIELD_WIDTH - 1, FIELD_HEIGHT - 1));  // 우하단
}

void SnakeGameMap::draw() {
    wclear(gameWin);
    box(gameWin, '#', '#');

    // 벽 그리기
    for (Point p : walls) {
        if ((p.x == 0 && p.y == 0) ||
            (p.x == FIELD_WIDTH - 1 && p.y == 0) ||
            (p.x == 0 && p.y == FIELD_HEIGHT - 1) ||
            (p.x == FIELD_WIDTH - 1 && p.y == FIELD_HEIGHT - 1)) {
            mvwprintw(gameWin, p.y, p.x, "@"); // 겉 테두리의 꼭짓점에 Immune Wall
        } else if (p.x == 0 || p.x == FIELD_WIDTH - 1) {
            mvwprintw(gameWin, p.y, p.x, "#"); // 세로 벽
        } else {
            mvwprintw(gameWin, p.y, p.x, "#"); // 가로 벽
        }
    }

    wrefresh(gameWin);
}

bool SnakeGameMap::isSnake(Point p) {
    for (Point s : snake) {
        if (s.x == p.x && s.y == p.y) return true;
    }
    return false;
}

int main() {
    SnakeGameMap gameMap;
    while (true) {
        gameMap.draw();
        usleep(500000); // 0.5초 대기
    }
    endwin();
    return 0;
}

```

### Map과 다른 객체와의 상호작용

#### 1. Snake (뱀)
- **뱀의 이동과 충돌 검사**: 뱀의 위치를 맵 상의 객체들과 비교하여 충돌 여부를 검사하고, 적절히 처리합니다.
- **아이템 획득**: 뱀이 아이템 위치에 도달했을 때, 해당 아이템을 획득하고 새로운 아이템을 맵에 배치합니다.
- **게이트 통과**: 뱀이 게이트 입구에 도달했을 때, 출구로 이동할 수 있게 합니다.

#### 2. Items (아이템)
- **아이템 위치 설정**: 초기에 맵에 랜덤하게 아이템을 배치하고, 뱀이 아이템을 획득했을 때 새로운 아이템을 생성합니다.
- **아이템 획득 여부 검사**: 뱀이 아이템 위치에 도달했을 때, 해당 아이템을 획득하도록 합니다.

#### 3. 기타 객체 (추가적인 요소들)
- **장애물 충돌 검사**: 뱀이나 다른 객체들이 장애물에 충돌했을 때의 처리를 구현합니다.
- **게이트 상호작용**: 게이트의 입구와 출구 사이의 이동을 처리하여 게임 플레이를 다양화합니다.

---

