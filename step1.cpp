#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <vector>

#include <unistd.h> // 디버깅에 사용

using namespace std;

const int FIELD_WIDTH = 50;
const int FIELD_HEIGHT = 21;


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
    vector<Point> walls; // walls
    vector<Point> immuneWalls; // immune walls

    void init();
    void draw();
};

SnakeGame::SnakeGame(){
    initscr();  //화면에 띄울 스크린 초기화

    gameWin = newwin(FIELD_HEIGHT, FIELD_WIDTH, 0, 0);

    init();
}

void SnakeGame::init() {

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

}

void SnakeGame::draw() {
    wclear(gameWin);
    box(gameWin, 0, 0);
    
    // wall 그리기
    for (Point p : walls) {
        mvwprintw(gameWin, p.y, p.x, "#");
    }
    
    // immune wall 그리기
    for (Point p : immuneWalls) {
        mvwprintw(gameWin, p.y, p.x, "@");
    }

    wrefresh(gameWin);
    
}



void SnakeGame::run() {
    draw();
    sleep(10);  // 디버깅용
    endwin();
}

int main() {
    SnakeGame game;
    game.run();
    return 0;
}