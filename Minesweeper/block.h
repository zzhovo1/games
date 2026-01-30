#ifndef __BLOCK_H__
#define __BLOCK_H__

class Block {
private:
    bool isMine = false;
    bool isRevealed = false;
    bool isFlagged = false;
    int adjacentMines = 0;
    
public:
    Block() = default; 

    bool getIsMine() const {return isMine;}
    void setIsMine(bool mine) {isMine = mine;}
    bool getIsRevealed() const {return isRevealed;}
    void setIsRevealed(bool revealed) {isRevealed = revealed;}
    bool getIsFlagged() const {return isFlagged;}
    void setIsFlagged(bool flagged) {isFlagged = flagged;}
    int getAdjacentMines() const {return adjacentMines;}
    void setAdjacentMines(int count) {adjacentMines = count;}

    void reset() {
        isMine = false;
        isRevealed = false;
        isFlagged = false;
        adjacentMines = 0;
    }
};

#endif // __BLOCK_H__