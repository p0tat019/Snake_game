#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <vector>

#include <unistd.h> // 디버깅에 사용

using namespace std;

const int FIELD_WIDTH = 50;
const int FIELD_HEIGHT = 21;
const int FIELD_INNER_WIDTH = 48;
const int FIELD_INNER_HEIGHT = 19;
const int INIT_SNAKE_LENGTH = 3;
const int TICK_RATE = 500; // ms
const int ITEM_DURATION = 5000; // ms


enum Direction { UP, DOWN, LEFT, RIGHT };

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

class SnakeGame {
public:
    SnakeGame();
    void run();

private:
    WINDOW *gameWin;
    vector<Point> snake;
    Direction direction;
    int growthItems;
    int poisonItems;
    Point growthItem;
    Point poisonItem;
    bool gameOver;
    clock_t itemSpawnTime;
    vector<Point> walls; // walls
    vector<Point> immuneWalls; // immune walls

    void init();
    void draw();
    void update();
    void generateItem();
    bool isCollision(Point p);
    bool isSnake(Point p);
    void removeItemIfExpired();
    void showGameOver();
};

SnakeGame::SnakeGame() :  growthItems(0), poisonItems(0), gameOver(false) {
    srand(time(0));
    initscr();  //화면에 띄울 스크린 초기화
    cbreak();
    noecho();
    curs_set(0);
    timeout(TICK_RATE);
    keypad(stdscr, TRUE);

    gameWin = newwin(FIELD_HEIGHT, FIELD_WIDTH, 0, 0);

    init();
}

void SnakeGame::init() {
    // Snake 초기화
    snake.clear();
    int startX = rand() % (FIELD_WIDTH - 4) + 2;  // 랜덤 X 좌표 (벽을 피하기 위해 범위 조정)
    int startY = rand() % (FIELD_HEIGHT - 4) + 2; // 랜덤 Y 좌표 (벽을 피하기 위해 범위 조정)
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
    for(int i = 1; i <= 5; ++i){
        walls.push_back(Point(centerX, centerY+i));
        walls.push_back(Point(centerX, centerY-i));
    }
    for(int i = 1; i <= 10; ++i){
        walls.push_back(Point(centerX+i, centerY));
        walls.push_back(Point(centerX-i, centerY));
    }
    


    // Immune Wall 초기화
    immuneWalls.clear();
    immuneWalls.push_back(Point(0, 0));
    immuneWalls.push_back(Point(FIELD_WIDTH - 1, 0));
    immuneWalls.push_back(Point(0, FIELD_HEIGHT - 1));
    immuneWalls.push_back(Point(FIELD_WIDTH - 1, FIELD_HEIGHT - 1));
    immuneWalls.push_back(Point(centerX, centerY));


    generateItem();
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
    
    wrefresh(gameWin);


}

void SnakeGame::update() {
    int ch = getch();
    Direction newDirection = direction; // 새로운 방향을 저장할 변수

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

    // 새로운 방향이 현재 방향과 반대되지 않을 때만 적용
    if (newDirection == direction){
        direction = direction;
    }else{
        direction = newDirection;
    }


    Point newHead = snake[0]; // 뱀의 머리 위치
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

    // 충돌 여부 확인
    if (isCollision(newHead)) {
        gameOver = true;
        showGameOver();
        return;
    }

    // 머리가 성장 아이템 위치에 도달했을 때 처리
    if (newHead.x == growthItem.x && newHead.y == growthItem.y) {
        growthItems++;
        snake.push_back(snake.back());
        generateItem();
    } 
    // 머리가 독 아이템 위치에 도달했을 때 처리
    else if (newHead.x == poisonItem.x && newHead.y == poisonItem.y) {
        poisonItems++;
        if (snake.size() > 3) {
            snake.pop_back();
        } else {
            gameOver = true;
            showGameOver();
            return;
        }
        generateItem();
    } 

    // 뱀의 몸통 이동
    for (int i = snake.size() - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = newHead;

    // 아이템 지속 시간이 만료되었는지 확인하여 새로 생성
    removeItemIfExpired();
}


void SnakeGame::generateItem() {
    do{
        growthItem = Point(rand() % FIELD_INNER_WIDTH + 1, rand() % FIELD_INNER_HEIGHT + 1);
        poisonItem = Point(rand() % FIELD_INNER_WIDTH + 1, rand() % FIELD_INNER_HEIGHT + 1);
        
    }while(isSnake(growthItem) || isSnake(poisonItem) || isCollision(growthItem) || isCollision(poisonItem) || (poisonItem.x == growthItem.x && poisonItem.y == growthItem.y));

    itemSpawnTime = clock();
}


bool SnakeGame::isCollision(Point p) {
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

void SnakeGame::removeItemIfExpired() {
    if ((clock() - itemSpawnTime) / CLOCKS_PER_SEC * 1000 >= ITEM_DURATION) {
        generateItem();
    }
}

void SnakeGame::showGameOver() {
    wclear(gameWin);
    mvwprintw(gameWin, FIELD_HEIGHT / 2, (FIELD_WIDTH - 10) / 2, "Game Over");
    wrefresh(gameWin);
    napms(2000); // Show "Game Over" for 2 seconds
    endwin();
}

void SnakeGame::run() {
    while (!gameOver) {
        draw();
        update();
    }
}

int main() {
    SnakeGame game;
    game.run();
    return 0;
}
