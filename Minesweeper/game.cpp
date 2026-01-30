#include "game.h"
#include <stdexcept>

Game::Game(int width, int height, int mines) {
    board = GameBoard(width, height, mines);
    isGameOver = false;
    isFirstMove = true;
}

void Game::revealCell(int x, int y) {
    if (isGameOver) {
        throw std::runtime_error("Game is over. Cannot reveal more cells.");
    }

    if (isFirstMove) {
        board.placeMines(x, y);
        board.countAdjacentMines();
        isFirstMove = false;
    }

    Block& block = board.getBlock(x, y);
    if (block.getIsRevealed() || block.getIsFlagged()) {
        return; // Do nothing if already revealed or flagged
    }

    block.setIsRevealed(true);

    if (block.getIsMine()) {
        isGameOver = true;
        // Handle game over logic here (e.g., reveal all mines)
    } else if (block.getAdjacentMines() == 0) {
        // Reveal adjacent cells recursively
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                int nr = x + dr;
                int nc = y + dc;
                if (nr >= 0 && nr < board.getRows() && nc >= 0 && nc < board.getCols()) {
                    revealCell(nr, nc);
                }
            }
        }
    }
}

void Game::flagCell(int x, int y) {
    if (isGameOver) {
        throw std::runtime_error("Game is over. Cannot flag cells.");
    }

    Block& block = board.getBlock(x, y);
    if (block.getIsRevealed()) {
        return; // Do nothing if already revealed
    }

    block.setIsFlagged(!block.getIsFlagged());
}

void Game::chordRevealCell(int x, int y) {
    if (isGameOver) {
        throw std::runtime_error("Game is over. Cannot chord reveal cells.");
    }

    Block& block = board.getBlock(x, y);
    if (!block.getIsRevealed() || block.getAdjacentMines() == 0) {
        return; // Do nothing if not revealed or has no adjacent mines
    }

    int flaggedCount = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int nr = x + dr;
            int nc = y + dc;
            if (nr >= 0 && nr < board.getRows() && nc >= 0 && nc < board.getCols()) {
                if (board.isFlagged(nr, nc)) {
                    flaggedCount++;
                }
            }
        }
    }

    if (flaggedCount == block.getAdjacentMines()) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                int nr = x + dr;
                int nc = y + dc;
                if (nr >= 0 && nr < board.getRows() && nc >= 0 && nc < board.getCols()) {
                    if (!board.isFlagged(nr, nc) && !board.isRevealed(nr, nc)) {
                        revealCell(nr, nc);
                    }
                }
            }
        }
    }
}

int Game::getRemainingMines() {
    int flaggedCount = 0;
    for (int i = 0; i < board.getRows(); ++i) {
        for (int j = 0; j < board.getCols(); ++j) {
            if (board.isFlagged(i, j)) {
                flaggedCount++;
            }
        }
    }
    return board.getMineCount() - flaggedCount;
}

bool Game::checkWin() {
    for (int i = 0; i < board.getRows(); ++i) {
        for (int j = 0; j < board.getCols(); ++j) {
            const Block& block = board.getBlock(i, j);
            if (!block.getIsMine() && !block.getIsRevealed()) {
                return false; // Found a non-mine block that is not revealed
            }
        }
    }
    return true; // All non-mine blocks are revealed
}