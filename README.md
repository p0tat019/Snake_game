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
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

enum class ObjectType { Wall, Obstacle, Item, Gate };

struct MapObject {
    Point position;
    ObjectType type;
    // 추가적인 필드들 (예: 아이템 종류, 게이트 입구와 출구 등)
};

class GameMap {
private:
    int width;
    int height;
    vector<MapObject> objects;

public:
    GameMap(int w, int h) : width(w), height(h) {}

    void addObject(Point pos, ObjectType type) {
        objects.push_back({pos, type});
    }

    void exportMap(string filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Failed to open file for export." << endl;
            return;
        }

        for (auto obj : objects) {
            file << obj.position.x << " " << obj.position.y << " ";
            switch (obj.type) {
                case ObjectType::Wall:
                    file << "Wall";
                    break;
                case ObjectType::Obstacle:
                    file << "Obstacle";
                    break;
                case ObjectType::Item:
                    file << "Item";
                    break;
                case ObjectType::Gate:
                    file << "Gate";
                    break;
            }
            file << endl;
        }

        file.close();
    }

    void importMap(string filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Failed to open file for import." << endl;
            return;
        }

        objects.clear();
        int x, y;
        string typeStr;
        while (file >> x >> y >> typeStr) {
            ObjectType type;
            if (typeStr == "Wall")
                type = ObjectType::Wall;
            else if (typeStr == "Obstacle")
                type = ObjectType::Obstacle;
            else if (typeStr == "Item")
                type = ObjectType::Item;
            else if (typeStr == "Gate")
                type = ObjectType::Gate;
            else {
                cerr << "Unknown object type found in file." << endl;
                continue;
            }

            objects.push_back({{x, y}, type});
        }

        file.close();
    }
};
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

