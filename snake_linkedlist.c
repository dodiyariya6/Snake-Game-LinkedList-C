/* snake_linkedlist.c
   Snake Game — Queue (Doubly Linked List) + Levels + Obstacles + Bonus Food + High Score
   DS Edition: Queue implemented using Doubly Linked List
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 18
#define MAX_OBSTACLES 80

#define WALL '#'
#define FOOD '*'
#define BONUS '$'
#define SNAKE_HEAD '@'
#define SNAKE_BODY 'O'
#define OBSTACLE 'X'
#define EMPTY ' '

enum Direction { UP, DOWN, LEFT, RIGHT };

typedef struct Node {
    int x, y;
    struct Node *prev, *next;
} Node;

Node *head = NULL, *tail = NULL; // Doubly Linked List Queue

enum Direction dir;

int foodX, foodY;
int bonusX, bonusY;
int bonusActive = 0;
int bonusTimer = 0;
int bonusLifetime = 35;
int obstaclesX[MAX_OBSTACLES], obstaclesY[MAX_OBSTACLES];
int obstacleCount = 0;

int score = 0;
int level = 1;
int speed = 140;
int gameOver = 0;
int highScore = 0;

// ==== FUNCTION DECLARATIONS ====
void gotoxy(int x, int y);
void hideCursor();
void loadHighScore();
void saveHighScore();
void showIntro();
void selectDifficulty();
void setup();
void placeFood();
void placeBonus();
int isCellFree(int x, int y);
void addObstaclesForLevel();
void drawBoard();
void inputHandler();
void moveSnake();
void checkCollision();
void increaseSpeedOrLevel();
void gameOverScreen();

// Linked List (Queue) functions
void enqueueFront(int x, int y);
void dequeueRear();
int snakeLength();
void freeSnake();

// ==== CURSOR CONTROL ====
void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// ==== FILE HANDLING ====
void loadHighScore() {
    FILE *f = fopen("highscore.txt", "r");
    if (f) { fscanf(f, "%d", &highScore); fclose(f); }
    else highScore = 0;
}

void saveHighScore() {
    FILE *f = fopen("highscore.txt", "w");
    if (f) { fprintf(f, "%d", highScore); fclose(f); }
}

// ==== GAME INTRO ====
void showIntro() {
    system("cls");
    printf("\n\n\t====== SNAKE: DS EDITION (Linked List Queue) ======\n\n");
    printf("\tControls: Arrow Keys / W A S D\n");
    printf("\tQuit: X or Esc\n\n");
    printf("\tPress any key to continue...");
    _getch();
}

// ==== DIFFICULTY MENU ====
void selectDifficulty() {
    int choice;
    system("cls");
    printf("\n\n\t====== DIFFICULTY ======\n\n");
    printf("\t1. Easy   (Slow)\n");
    printf("\t2. Medium (Normal)\n");
    printf("\t3. Hard   (Fast)\n\n");
    printf("\tEnter choice: ");
    scanf("%d", &choice);
    switch (choice) {
        case 1: speed = 220; break;
        case 2: speed = 140; break;
        case 3: speed = 100; break;
        default: speed = 140; break;
    }
}

// ==== LINKED LIST QUEUE IMPLEMENTATION ====
void enqueueFront(int x, int y) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->x = x;
    newNode->y = y;
    newNode->prev = NULL;
    newNode->next = head;
    if (head) head->prev = newNode;
    head = newNode;
    if (!tail) tail = newNode;
}

void dequeueRear() {
    if (!tail) return;
    Node* temp = tail;
    tail = tail->prev;
    if (tail) tail->next = NULL;
    else head = NULL;
    free(temp);
}

int snakeLength() {
    int count = 0;
    for (Node* temp = head; temp; temp = temp->next)
        count++;
    return count;
}

void freeSnake() {
    while (head) dequeueRear();
}

// ==== INITIAL SETUP ====
void setup() {
    system("cls");
    srand(time(0));
    loadHighScore();
    freeSnake();

    int startX = WIDTH / 2, startY = HEIGHT / 2;

    // Correct start direction (right)
    for (int i = 3; i >= 0; --i)
        enqueueFront(startX - i, startY);

    dir = RIGHT;
    score = 0;
    level = 1;
    obstacleCount = 0;
    bonusActive = 0;
    bonusTimer = 0;
    gameOver = 0;
    placeFood();
}

// ==== UTILITY ====
int isCellFree(int x, int y) {
    for (Node* temp = head; temp; temp = temp->next)
        if (temp->x == x && temp->y == y) return 0;
    for (int o = 0; o < obstacleCount; ++o)
        if (obstaclesX[o] == x && obstaclesY[o] == y) return 0;
    if (x == foodX && y == foodY) return 0;
    if (bonusActive && x == bonusX && y == bonusY) return 0;
    return 1;
}

void placeFood() {
    int x, y;
    do {
        x = rand() % (WIDTH - 2) + 1;
        y = rand() % (HEIGHT - 2) + 1;
    } while (!isCellFree(x, y));
    foodX = x; foodY = y;
}

void placeBonus() {
    if (rand() % 8 == 0) {
        int x, y;
        do {
            x = rand() % (WIDTH - 2) + 1;
            y = rand() % (HEIGHT - 2) + 1;
        } while (!isCellFree(x, y));
        bonusX = x; bonusY = y;
        bonusActive = 1;
        bonusTimer = bonusLifetime;
    }
}

void addObstaclesForLevel() {
    if (level < 2) return;
    int toAdd = 2 + level;
    for (int k = 0; k < toAdd && obstacleCount < MAX_OBSTACLES; ++k) {
        int x, y;
        do {
            x = rand() % (WIDTH - 2) + 1;
            y = rand() % (HEIGHT - 2) + 1;
        } while (!isCellFree(x, y));
        obstaclesX[obstacleCount] = x;
        obstaclesY[obstacleCount] = y;
        obstacleCount++;
    }
}

// ==== DRAW BOARD ====
void drawBoard() {
    gotoxy(0, 0);
    for (int j = 0; j < WIDTH; ++j) putchar(WALL);
    putchar('\n');
    for (int i = 1; i < HEIGHT - 1; ++i) {
        putchar(WALL);
        for (int j = 1; j < WIDTH - 1; ++j) {
            int printed = 0;
            if (j == foodX && i == foodY) { putchar(FOOD); printed = 1; }
            else if (bonusActive && j == bonusX && i == bonusY) { putchar(BONUS); printed = 1; }
            else {
                for (int o = 0; o < obstacleCount; ++o)
                    if (obstaclesX[o] == j && obstaclesY[o] == i) { putchar(OBSTACLE); printed = 1; break; }
            }
            if (!printed) {
                Node* temp = head;
                int k = 0;
                for (; temp; temp = temp->next, ++k)
                    if (temp->x == j && temp->y == i) { putchar((k == 0) ? SNAKE_HEAD : SNAKE_BODY); printed = 1; break; }
            }
            if (!printed) putchar(EMPTY);
        }
        putchar(WALL);
        putchar('\n');
    }
    for (int j = 0; j < WIDTH; ++j) putchar(WALL);
    printf("\nScore: %d | Level: %d | High Score: %d\n", score, level, highScore);
}

// ==== INPUT HANDLER ====
void inputHandler() {
    if (_kbhit()) {
        int ch = _getch();
        if (ch == 224) ch = _getch();
        switch (ch) {
            case 72: case 'w': if (dir != DOWN) dir = UP; break;
            case 80: case 's': if (dir != UP) dir = DOWN; break;
            case 75: case 'a': if (dir != RIGHT) dir = LEFT; break;
            case 77: case 'd': if (dir != LEFT) dir = RIGHT; break;
            case 'x': case 'X': case 27: gameOver = 1; break;
        }
    }
}

// ==== MOVEMENT ====
void moveSnake() {
    int newX = head->x;
    int newY = head->y;

    if (dir == UP) newY--;
    if (dir == DOWN) newY++;
    if (dir == LEFT) newX--;
    if (dir == RIGHT) newX++;

    enqueueFront(newX, newY);

    if (newX == foodX && newY == foodY) {
        score += 10;
        placeFood();
        placeBonus();
        increaseSpeedOrLevel();
    }
    else if (bonusActive && newX == bonusX && newY == bonusY) {
        bonusActive = 0;
        bonusX = -1;
        bonusY = -1;
        score += 50;
        increaseSpeedOrLevel();
        // Do not dequeue => snake grows naturally
    }
    else {
        dequeueRear();
    }
}

// ==== COLLISION DETECTION ====
void checkCollision() {
    if (head->x == 0 || head->x == WIDTH - 1 || head->y == 0 || head->y == HEIGHT - 1)
        gameOver = 1;
    for (Node* temp = head->next; temp; temp = temp->next)
        if (head->x == temp->x && head->y == temp->y) gameOver = 1;
    for (int o = 0; o < obstacleCount; ++o)
        if (head->x == obstaclesX[o] && head->y == obstaclesY[o]) gameOver = 1;
}

// ==== LEVEL & SPEED ====
void increaseSpeedOrLevel() {
    int newLevel = score / 50 + 1;
    if (newLevel > level) {
        level = newLevel;
        addObstaclesForLevel();
        if (speed > 70) speed -= 10;
    }
}

// ==== GAME OVER SCREEN ====
void gameOverScreen() {
    system("cls");
    printf("\n\n\t====== GAME OVER ======\n");
    printf("\tFinal Score : %d\n", score);
    printf("\tLevel : %d\n", level);

    if (score > highScore) {
        highScore = score;
        saveHighScore();
        printf("\tNEW HIGH SCORE: %d!\n", highScore);
    } else {
        printf("\tHigh Score : %d\n", highScore);
    }

    Sleep(1000);

    printf("\nPress:\n");
    printf("\tR - Restart (same difficulty)\n");
    printf("\tD - Change Difficulty\n");
    printf("\tX - Quit\n");

    while (1) {
        char c = _getch();
        if (c == 'r' || c == 'R') {
            setup();
            break;
        } 
        else if (c == 'd' || c == 'D') {
            selectDifficulty();
            setup();
            break;
        } 
        else if (c == 'x' || c == 'X' || c == 27) {
            exit(0);
        }
    }
}

// ==== MAIN LOOP ====
int main() {
    showIntro();
    hideCursor();
    selectDifficulty();
    setup();

    Sleep(300); // Small delay before starting

    while (1) {
        if (gameOver) gameOverScreen();
        drawBoard();
        inputHandler();
        moveSnake();
        checkCollision();
        if (bonusActive && --bonusTimer <= 0) bonusActive = 0;
        Sleep(speed);
    }
    return 0;
}