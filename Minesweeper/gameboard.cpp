#include <cstdio>
#include <cstdlib>
#include "gameboard.h"

GameBoard::GameBoard(int r, int c, int mines) : rows(r), cols(c), mineCount(mines) {
    this->board.resize(rows, std::vector<Block>(cols));
}

void GameBoard::initializeBoard() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            board[i][j] = Block();
        }
    }
}

void GameBoard::placeMines(int firstRow, int firstCol) {
    int placedMines = 0;
    while (placedMines < mineCount) {
        int r = rand() % rows;
        int c = rand() % cols;
        if ((r == firstRow && c == firstCol) || this->board[r][c].getIsMine()) {
            continue; // Avoid placing mine on the first clicked block or on an existing mine
        }
        this->board[r][c].setIsMine(true);
        placedMines++;
    }
}

void GameBoard::countAdjacentMines() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (this->board[i][j].getIsMine()) {
                continue;
            }
            int mineCount = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = i + dr;
                    int nc = j + dc;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && this->board[nr][nc].getIsMine()) {
                        mineCount++;
                    }
                }
            }
            this->board[i][j].setAdjacentMines(mineCount);
        }
    }
}

Block& GameBoard::getBlock(int row, int col) {
    return this->board[row][col];
}

bool GameBoard::isMine(int row, int col) {
    return this->board[row][col].getIsMine();
}

bool GameBoard::isRevealed(int row, int col) {
    return this->board[row][col].getIsRevealed();
}

bool GameBoard::isFlagged(int row, int col) {
    return this->board[row][col].getIsFlagged();
}

int GameBoard::getAdjacentMines(int row, int col) {
    return this->board[row][col].getAdjacentMines();
}

