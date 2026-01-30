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
    void revealCell(int x, int y);
    void flagCell(int x, int y);
    bool checkWin();

};
#endif // __GAME_H__