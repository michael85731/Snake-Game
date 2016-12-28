# Homework 01 - Share Memory
These program were writen by only `C`
It's a simple snake game.
## How to compile
I use `GCC` to compile these program.
I also use a library call ncurses, so you have to install it on you computer and add additional argument when you compile.
Just type:
```terminal
gcc snakeGame.c -o  snakeGame -lncurses
```
## How to execute
In terminal, type the file that after compile.
Then you can play the game.
```terminal
./snakeGame
```
## Game rule
1. Use up, down, left, right key to contorl the snake.
2. You can't reverse snake  direction.
3. There's 15 apple on the map, you have to control the snake to eat them all.
4. You can't let snake eat a stone or go over the fence, that will cause game over.

There's few icon on the screen, here's the illustration
- `🍎`
  Apple, snake have to eat them all.
- `⊗`
  Stone, snake can't eats stone.
- `|`, `–`
  Fence, snake can't go out over the fence.
