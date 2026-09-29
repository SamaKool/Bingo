# Terminal Bingo

Classic 5x5 Bingo (numbers 1-25) against the computer, right in your terminal. Written in C++.

## How to play

Take turns calling numbers. Every called number is struck out on both grids. Complete 5 lines (rows, columns or diagonals) to spell **B-I-N-G-O** and win.

- Use a random grid or type in your own
- The computer's grid stays hidden until the game ends
- The computer picks numbers using only its own grid, so it never peeks at yours

## Run it

```
g++ -std=c++11 -O2 -o bingo bingo.cpp
./bingo        # Windows: .\bingo.exe
```

Needs any C++11-capable compiler (g++, clang, MSVC).

## Sample

```
  YOUR GRID
  +-------------------------+
  | [ 1] [ 2]   3   [ 4]  5 |
  |   6  [ 7]   8    9   10 |
  ...
  You: BI___ (2 lines)   Computer: B____ (1 lines)
```
