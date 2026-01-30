#ifndef __GAMEBOARD_H__
#define __GAMEBOARD_H__

#include "block.h"
#include <vector>

class GameBoard {
private:
    std::vector<std::vector<Block>> board;
    int rows;
    int cols;
    int mineCount;

public:
    GameBoard() = default;
    GameBoard(int r, int c, int mines);
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int getMineCount() const { return mineCount; }
    void initializeBoard();
    void placeMines(int firstRow, int firstCol);
    void countAdjacentMines();
    Block& getBlock(int row, int col);
    bool isMine(int row, int col);
    bool isRevealed(int row, int col);
    bool isFlagged(int row, int col);
    int getAdjacentMines(int row, int col);
};

#endif // __GAMEBOARD_H__