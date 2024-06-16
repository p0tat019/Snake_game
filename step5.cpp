#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm> // std::remove 사용을 위해 포함

using namespace std;

const int FIELD_WIDTH = 50;
const int FIELD_HEIGHT = 21;
const int FIELD_INNER_WIDTH = 48;
const int FIELD_INNER_HEIGHT = 19;
const int TICK_RATE = 500; // ms
const int ITEM_DURATION = 15000; // ms
const int GATE_DURATION = 15000; // ms
const int INIT_SNAKE_LENGTH = 3;
const int SCORE_WIDTH = 20; // Width of the scoreboard
const int SCORE_HEIGHT = FIELD_HEIGHT; // Height of the scoreboard



enum Direction { UP, DOWN, LEFT, RIGHT };

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& p) const {
        return x == p.x && y == p.y;
    }
};

class SnakeGame {
public:
    SnakeGame();
    void run();
    static int stage;
private:
    WINDOW *gameWin;
    WINDOW *scoreWin;
    vector<Point> snake;
    Direction direction;
    Point growthItem;
    Point poisonItem;
    Point gate1;
    Point gate2;
    bool gameOver;
    time_t itemSpawnTime;
    time_t gateSpawnTime;
    vector<Point> walls; // walls
    vector<Point> immuneWalls; // immune walls
    int gateUses;
    int score;
    int maxLength;
    int growthItems;
    int poisonItems;
    time_t start = time(nullptr);



    void init();
    void draw();
    void update();
    void generateItem();
    void generateGate();
    bool isCollision(Point p);
    bool isSnake(Point p);
    bool isWall(Point p);
    bool isImmune(Point p);
    bool wallsContain(Point p);
    void removeItemIfExpired();
    void removeGateIfExpired();
    void showGameOver();
};

int SnakeGame::stage = 1;

SnakeGame::SnakeGame() : score(0), growthItems(0), poisonItems(0), gateUses(0), maxLength(INIT_SNAKE_LENGTH), gameOver(false) {
    srand(time(0));
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    timeout(TICK_RATE);
    keypad(stdscr, TRUE);

    gameWin = newwin(FIELD_HEIGHT, FIELD_WIDTH, 0, 0);
    scoreWin = newwin(SCORE_HEIGHT, SCORE_WIDTH, 0, FIELD_WIDTH);

    init();
}

void SnakeGame::init() {
    // Snake 초기화
    snake.clear();
    
    
    // 고정된 시작 위치 설정
    int startX = FIELD_WIDTH / 2+ 11;
    int startY = FIELD_HEIGHT / 2;

    // 뱀의 초기 세그먼트 추가
    snake.push_back(Point(startX, startY));
    snake.push_back(Point(startX, startY + 1));
    snake.push_back(Point(startX, startY + 2));

    direction = UP;

    // Wall 초기화
    walls.clear();
    int centerX = FIELD_WIDTH / 2;
    int centerY = FIELD_HEIGHT / 2;
    for (int i = 1; i < FIELD_WIDTH; ++i) {
        walls.push_back(Point(i, 0));
        walls.push_back(Point(i, FIELD_HEIGHT - 1));
    }
    for (int i = 1; i < FIELD_HEIGHT; ++i) {
        walls.push_back(Point(0, i));
        walls.push_back(Point(FIELD_WIDTH - 1, i));
    }
    for (int i = 1; i <= 5; ++i) {
        walls.push_back(Point(centerX, centerY + i));
        walls.push_back(Point(centerX, centerY - i));
    }
    for (int i = 1; i <= 10; ++i) {
        walls.push_back(Point(centerX + i, centerY));
        walls.push_back(Point(centerX - i, centerY));
    }

    // Immune Wall 초기화
    immuneWalls.clear();
    immuneWalls.push_back(Point(0, 0));
    immuneWalls.push_back(Point(FIELD_WIDTH - 1, 0));
    immuneWalls.push_back(Point(0, FIELD_HEIGHT - 1));
    immuneWalls.push_back(Point(FIELD_WIDTH - 1, FIELD_HEIGHT - 1));
    immuneWalls.push_back(Point(centerX, centerY));

    generateItem();
    generateGate();
}

void SnakeGame::draw() {
    wclear(gameWin);
    box(gameWin, 0, 0);

    // snake 그리기
    for (Point p : snake) {
        mvwprintw(gameWin, p.y, p.x, "O");
    }

    // wall 그리기
    for (Point p : walls) {
        mvwprintw(gameWin, p.y, p.x, "#");
    }

    // immune wall 그리기
    for (Point p : immuneWalls) {
        mvwprintw(gameWin, p.y, p.x, "@");
    }

    // Items 그리기
    mvwprintw(gameWin, growthItem.y, growthItem.x, "+");
    mvwprintw(gameWin, poisonItem.y, poisonItem.x, "-");

    // Gates 그리기
    mvwprintw(gameWin, gate1.y, gate1.x, "G");
    mvwprintw(gameWin, gate2.y, gate2.x, "G");

    wrefresh(gameWin);

    // scoreboard 그리기
    wclear(scoreWin);
    box(scoreWin, 0, 0);
    mvwprintw(scoreWin, 1, 1, "Time: %dsec", (int)(time(nullptr)-start));
    mvwprintw(scoreWin, 2, 1, "Score Board");
    mvwprintw(scoreWin, 3, 1, "B: %d / %d", static_cast<int>(snake.size()), maxLength);
    mvwprintw(scoreWin, 4, 1, "+: %d", growthItems);
    mvwprintw(scoreWin, 5, 1, "-: %d", poisonItems);
    mvwprintw(scoreWin, 6, 1, "G: %d", gateUses);
    
    mvwprintw(scoreWin, 8, 1, "Mission");
    mvwprintw(scoreWin, 9, 1, "B: 10 (%c)", (snake.size() >= 10 ? 'v' : ' '));
    mvwprintw(scoreWin, 10, 1, "+: 5 (%c)", (growthItems >= 5 ? 'v' : ' '));
    mvwprintw(scoreWin, 11, 1, "-: 2 (%c)", (poisonItems >= 2 ? 'v' : ' '));
    mvwprintw(scoreWin, 12, 1, "G: 1 (%c)", (gateUses >= 1 ? 'v' : ' '));

    if (snake.size() >= 10 && growthItems >= 5 && poisonItems >= 2 && gateUses >= 1) {
        mvwprintw(scoreWin, 10, 1, "All missions completed! Game Over!");
        gameOver = true;
    }

    wrefresh(scoreWin);


}

void SnakeGame::update() {
    int ch = getch();
    Direction newDirection = direction;

    switch (ch) {
    case KEY_UP:
        newDirection = UP;
        break;
    case KEY_DOWN:
        newDirection = DOWN;
        break;
    case KEY_LEFT:
        newDirection = LEFT;
        break;
    case KEY_RIGHT:
        newDirection = RIGHT;
        break;
    }

    if (newDirection == direction) {
        direction = direction;
    } else {
        direction = newDirection;
    }

    Point newHead = snake[0];
    switch (direction) {
    case UP:
        newHead.y--;
        break;
    case DOWN:
        newHead.y++;
        break;
    case LEFT:
        newHead.x--;
        break;
    case RIGHT:
        newHead.x++;
        break;
    }
    // 게이트가 벽을 통과할 수 있는 로직 추가
    if (newHead.x == gate1.x && newHead.y == gate1.y && isWall(gate1)) {
        newHead = gate2; // gate1을 통과하여 gate2로 이동
        gateUses++;
        direction = (gate2.y > gate1.y) ? DOWN : UP; // 게이트가 세로로 생성된 경우 방향 설정
    } else if (newHead.x == gate2.x && newHead.y == gate2.y && isWall(gate2)) {
        newHead = gate1; // gate2를 통과하여 gate1로 이동
        gateUses++;
        direction = (gate1.x > gate2.x) ? RIGHT : LEFT; // 게이트가 가로로 생성된 경우 방향 설정
    }
    if (isCollision(newHead)) {
        gameOver = true;
        showGameOver();
        return;
    }

    if (newHead.x == growthItem.x && newHead.y == growthItem.y) {
        growthItems++;
        snake.push_back(snake.back());
        generateItem();
    } else if (newHead.x == poisonItem.x && newHead.y == poisonItem.y) {
        poisonItems++;
        if (snake.size() > 3) {
            snake.pop_back();
        } else {
            gameOver = true;
            showGameOver();
            return;
        }
        generateItem();
    } else if (newHead.x == gate1.x && newHead.y == gate1.y) {
        if (gate2.y == 0) {
            direction = DOWN;
        } else if (gate2.y == FIELD_HEIGHT - 1) {
            direction = UP;
        } else if (gate2.x == 0) {
            direction = RIGHT;
        } else if (gate2.x == FIELD_WIDTH - 1) {
            direction = LEFT;
        }
        gateUses++;
        newHead = gate2;
    } else if (newHead.x == gate2.x && newHead.y == gate2.y) {
        if (gate1.y == 0) {
            direction = DOWN;
        } else if (gate1.y == FIELD_HEIGHT - 1) {
            direction = UP;
        } else if (gate1.x == 0) {
            direction = RIGHT;
        } else if (gate1.x == FIELD_WIDTH - 1) {
            direction = LEFT;
        }
        gateUses++;
        newHead = gate1;
    }

    for (int i = snake.size() - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = newHead;

    removeItemIfExpired();
    removeGateIfExpired();
}

void SnakeGame::generateItem() {
    do {
        growthItem = Point(rand() % FIELD_INNER_WIDTH + 1, rand() % FIELD_INNER_HEIGHT + 1);
        poisonItem = Point(rand() % FIELD_INNER_WIDTH + 1, rand() % FIELD_INNER_HEIGHT + 1);
    } while (isSnake(growthItem) || isSnake(poisonItem) || isCollision(growthItem) || isCollision(poisonItem) || (poisonItem.x == growthItem.x && poisonItem.y == growthItem.y));

    itemSpawnTime = time(0);
}
void SnakeGame::generateGate() {
    bool gate1OnWall = isWall(gate1);
    bool gate2OnWall = isWall(gate2);

    // 게이트 위치 임시로 제거
    if (gate1OnWall) {
        walls.erase(remove(walls.begin(), walls.end(), gate1), walls.end());
    }
    if (gate2OnWall) {
        walls.erase(remove(walls.begin(), walls.end(), gate2), walls.end());
    }

    // 게이트 위치 임의 생성 및 immune 벡터 확인
    do {
        gate1 = Point(rand() % (FIELD_WIDTH - 2) + 1, rand() % (FIELD_HEIGHT - 2) + 1);
        gate2 = Point(rand() % (FIELD_WIDTH - 2) + 1, rand() % (FIELD_HEIGHT - 2) + 1);
    } while (isWall(gate1) || isWall(gate2) || isImmune(gate1) || isImmune(gate2));

    // 생성한 게이트 위치 벽에 다시 추가
    if (gate1OnWall) {
        walls.push_back(gate1);
    }
    if (gate2OnWall) {
        walls.push_back(gate2);
    }

    gateSpawnTime = time(0);
}

bool SnakeGame::isImmune(Point p) {
    for (Point imm : immuneWalls) {
        if (imm.x == p.x && imm.y == p.y) {
            return true;
        }
    }
    return false;
}


bool SnakeGame::isCollision(Point p) {
    if ((p.x == gate1.x && p.y == gate1.y) || (p.x == gate2.x && p.y == gate2.y)) {
        return false;
    }
    if (p.x <= 0 || p.x >= FIELD_WIDTH - 1 || p.y <= 0 || p.y >= FIELD_HEIGHT - 1) {
        return true;
    }
    if (isSnake(p)) return true;
    for (Point w : walls) {
        if (w.x == p.x && w.y == p.y) return true;
    }
    return false;
}

bool SnakeGame::isSnake(Point p) {
    for (Point s : snake) {
        if (s.x == p.x && s.y == p.y) return true;
    }
    return false;
}

bool SnakeGame::isWall(Point p) {
    for (Point wall : walls) {
        if (wall.x == p.x && wall.y == p.y) {
            return true;
        }
    }
    return false;
}

void SnakeGame::removeItemIfExpired() {
    if (time(0) - itemSpawnTime > ITEM_DURATION / 1000) {
        generateItem();
    }
}

void SnakeGame::removeGateIfExpired() {
    if (time(0) - gateSpawnTime > GATE_DURATION / 1000) {
        // 게이트가 벽에 있을 때만 벽을 복원
        if (isWall(gate1) && !wallsContain(gate1)) {
            walls.push_back(gate1);
        }
        if (isWall(gate2) && !wallsContain(gate2)) {
            walls.push_back(gate2);
        }

        generateGate();
    }
}

bool SnakeGame::wallsContain(Point p) {
    return find(walls.begin(), walls.end(), p) != walls.end();
}


void SnakeGame::showGameOver() {
    wclear(gameWin);
    mvwprintw(gameWin, FIELD_HEIGHT / 2, FIELD_WIDTH / 2 - 5, "Game Over");
    wrefresh(gameWin);
    napms(2000); // 2초 대기
    endwin();
    exit(0);
}

void SnakeGame::run() {
    while (!gameOver) {
        update();
        draw();
    }
}

int main() {
    SnakeGame game;
    game.run();
    return 0;
}

