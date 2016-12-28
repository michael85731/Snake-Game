#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <locale.h>

#define Key_Up 'u'
#define Key_Down 'd'
#define Key_Left 'l'
#define Key_Right 'r'

typedef struct coordinateObject{
  int x;
  int y;
} CoordinateObject;

char direction = Key_Right;  // Initial direction is right
int score = 0;               // Player score
bool gameState = true;       // Game state
char *gameStateDescribe;     // Game state descirbe
int snakeLength = 12;        // Length include snake head
int appleNum = 15;            // How many apples will appear at screen
int stoneNum = 25;           // How many stones will appear at screen
int fenceNum = 0;            // How many fences will generate at screen(depends on terminal size)
int speed = 15;              // Snake speed
WINDOW *w;                   // Window object, use for ncurses
int wx = 0;
int wy = 0;
CoordinateObject *snake;     // Snake(actual snake)
CoordinateObject *apples;    // Apple(actual apples)
CoordinateObject *stones;    // Stone(actual stones)
CoordinateObject *fences;    // Fences(actual fences)
pthread_mutex_t lock;        // Mutex lock, global control variable

void userChangeDirection(void){
  while(1){
    pthread_mutex_lock(&lock);

    char *nowDirection = &direction;   //Loop to get user input
    int userInput = getch();

    switch(userInput){
      case KEY_UP:
        if(direction != Key_Down)
          *nowDirection = Key_Up;
        break;
      case KEY_DOWN:
        if(direction != Key_Up)
          *nowDirection = Key_Down;
        break;
      case KEY_LEFT:
        if(direction != Key_Right)
          *nowDirection = Key_Left;
        break;
      case KEY_RIGHT:
        if(direction != Key_Left)
          *nowDirection = Key_Right;
        break;
    }

    pthread_mutex_unlock(&lock);
  }
}

void updatePosition(int snakeLength){
  pthread_mutex_lock(&lock);

  CoordinateObject *snakeHead = &snake[snakeLength - 1];

  // Update snake body position, but not update snakeHead
  for(int i = 0;i < snakeLength - 1;i++){
    snake[i].x = snake[i + 1].x;
    snake[i].y = snake[i + 1].y;
  }

  // Update snakeHead position
  switch(direction){
    case Key_Up:
      snakeHead -> y -= 1;
      break;
    case Key_Down:
      snakeHead -> y += 1;
      break;
    case Key_Left:
      snakeHead -> x -= 1;
      break;
    case Key_Right:
      snakeHead -> x += 1;
      break;
  }

  pthread_mutex_unlock(&lock);
}

void updateGameState(void){
  pthread_mutex_lock(&lock);

  CoordinateObject snakeHead = snake[snakeLength - 1];

  // If snake eats apple, score + 1, hide the apple that has been eat
  for(int i = 0; i < appleNum; i++){
    if((snakeHead.x == apples[i].x || snakeHead.x == apples[i].x + 1) && snakeHead.y == apples[i].y){
      apples[i].x = -1;
      apples[i].y = -1;
      score++;
    }
  }

  memset(gameStateDescribe, '\0', sizeof(*gameStateDescribe));

  // If snake eats stone, game over
  for(int i = 0; i < stoneNum; i++){
    if(snakeHead.x == stones[i].x && snakeHead.y == stones[i].y){
      gameStateDescribe = "Snake don't eats stone";
      gameState = false;
      break;
    }
  }

  // If snake touchs fence, game over
  for(int i = 0; i < fenceNum; i++){
    if(snakeHead.x == fences[i].x && snakeHead.y == fences[i].y){
      gameStateDescribe = "Snake don't eats fence";
      gameState = false;
      break;
    }
  }

  // If player eats all apples on the screen, game over too
  if(appleNum == score){
    gameStateDescribe = "You Win !\nYou eat all the apples. Well Played !";
    gameState = false;
  }
  pthread_mutex_unlock(&lock);
}

void render(){
  while(1){
    refresh();
    erase();

    for(int i = 0; i < ((wx / 2) - 45); i++){
      printw(" ");
    }

    printw("Your goal is to eat all the apples on the screen, there are ");
    printw("%d apples, ", appleNum);
    printw("you eat %d apples.", score);

    //Render snake
    for(int i = 0;i <= snakeLength;i++){
      char snakePic;
      if(i == snakeLength - 1)
        snakePic = '@';
      else
        snakePic = 'o';

      mvaddch(snake[i].y, snake[i].x, snakePic);
    }

    //Render apples
    for(int i = 0; i < appleNum;i++){
      char *applePic = "🍎";
      mvprintw(apples[i].y, apples[i].x, applePic);
    }

    //Render stones
    for(int i = 0; i < appleNum;i++){
      char *stonePic = "⊗";
      mvprintw(stones[i].y, stones[i].x, stonePic);
    }

    // Render fences
    CoordinateObject temp;
    int temp2 = 0;
    for(int i = 0; i < fenceNum - 2; i++){
      char *columnFencePic = "|";
      char *rowFencePic = "–";
      if(fences[i].x == 0 || fences[i].x == wx){
        mvprintw(fences[i].y, fences[i].x, columnFencePic);
      }else{
        mvprintw(fences[i].y, fences[i].x, rowFencePic);
      }
    }

    usleep((1000 / speed) * 1000);
  }
}

int main(void){
  //Setting init argument
  srand((unsigned) time(NULL) + getpid());                     // Guarantee every rand() will be different
  pthread_t userChangeDirectionThreadId; // Pthread function init
  pthread_t renderThreadId;
  setlocale(LC_ALL, "");                 // Set Locale to correctly show unicode char
  w = initscr();                         // Ncurses init setting
  getmaxyx(w, wy, wx); wy--; wx--;
  keypad(stdscr, true);
  noecho();
  curs_set(0);
  nodelay(w, true);
  fenceNum = 2 * (wx + wy);
  gameStateDescribe = malloc(sizeof(char) * 2048);
  snake = malloc(sizeof(CoordinateObject) * snakeLength);  // Snake init memory
  apples = malloc(sizeof(CoordinateObject) * appleNum);    // Apple init memory
  stones = malloc(sizeof(CoordinateObject) * stoneNum);    // Stone init memory
  fences = malloc(sizeof(CoordinateObject) * fenceNum);    // Fence init memory

  //宣告蛇的起始位置
  for(int i = 0;i < snakeLength;i++){
    snake[i].x = i + 1;
    snake[i].y = 2;
  }

  //宣告蘋果的起始位置
  for(int i = 0;i < appleNum;i++){
    apples[i].x = rand() % (wx - 2) + 1;
    apples[i].y = rand() % (wy - 2) + 1;
  }

  //宣告石頭的起始位置，並且不能跟apples的位置相同
  for(int i = 0;i < stoneNum;i++){
    bool repeatFlag = false;
    do{
      repeatFlag = false;
      stones[i].x = rand() % (wx - 2) + 1;
      stones[i].y = rand() % (wy - 2) + 1;
      for(int j = 0; j < appleNum ; j++){
        if(stones[i].x == apples[j].x && stones[i].y == apples[j].y){
          repeatFlag = true;
          break;
        }
      }
    }while(repeatFlag);
  }

  //宣告每個柵欄的起始位置
  int fenceIterator = 0;
  for(int i = 1; i <= wx; i++){
    fences[fenceIterator].x = i;
    fences[fenceIterator].y = 1;
    fenceIterator++;
  }

  for(int i = 0; i < wx; i++){
    fences[fenceIterator].x = i;
    fences[fenceIterator].y = wy;
    fenceIterator++;
  }

  for(int i = 2; i <= wy; i++){
    fences[fenceIterator].x = wx;
    fences[fenceIterator].y = i;
    fenceIterator++;
  }

  for(int i = 1; i < wy; i++){
    fences[fenceIterator].x = 0;
    fences[fenceIterator].y = i;
    fenceIterator++;
  }

  //建立偵測方向的thread
  if(pthread_create(&userChangeDirectionThreadId, NULL, (void *)userChangeDirection, NULL) != 0)
    perror("Create updatePosition thread error");

  //建立render的thread
  if(pthread_create(&renderThreadId, NULL, (void *)render, NULL) != 0)
    perror("Create render thread error");

  while(gameState){
    updatePosition(snakeLength);
    updateGameState();
    usleep((1000 / speed) * 1000);
  }

  //遊戲結束
  pthread_cancel(userChangeDirectionThreadId);
  pthread_cancel(renderThreadId);

  //印出遊戲結束畫面
  for(int i = 0; i < 10; i++){
    refresh();
    erase();
    if(strstr(gameStateDescribe, "Win")){
      printw("%s\n", gameStateDescribe);
    }else{
      printw("Game over\n");
      printw("%s\n", gameStateDescribe);
    }
  }

  return 0;
}
