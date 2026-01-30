#ifndef __GAME_H__
#define __GAME_H__

#include "gameboard.h"

class Game {
private:
    GameBoard board;
    bool isGameOver;
    bool isFirstMove;
public:
    Game() = default;
    Game(int width, int height, int mines);
    GameBoard& getBoard() { return board; }
    bool getIsGameOver() const { return isGameOver; }
    bool getIsFirstMove() const { return isFirstMove; }
    void revealCell(int x, int y);
    void flagCell(int x, int y);
    void chordRevealCell(int x, int y);
    int getRemainingMines();
    bool checkWin();

};
#endif // __GAME_H__