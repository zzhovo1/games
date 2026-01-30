#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "game.h"
#include "recordmanager.h"

// 常量定义
const int TILE_DISPLAY_SIZE = 32; // 实际显示的方块大小
const float IMAGE_ORIGINAL_SIZE = 216.0f; // 你的图片原始像素
const float SCALE = TILE_DISPLAY_SIZE / IMAGE_ORIGINAL_SIZE;
const int HEADER_H = 60; // 顶部预留高度
const int FOOTER_H = 40; // 底部预留高度

struct Difficulty {
    std::string name;
    int rows, cols, mines;
};

std::vector<Difficulty> levels = {
    {"初级", 9, 9, 10},
    {"中级", 16, 16, 40},
    {"高级", 16, 30, 99} // 扫雷高级通常是30列16行
};

int main() {
    int currentLevel = 0;
    Game game(levels[currentLevel].cols, levels[currentLevel].rows, levels[currentLevel].mines);
    
    sf::RenderWindow window(sf::VideoMode(levels[currentLevel].cols * TILE_DISPLAY_SIZE, 
                            levels[currentLevel].rows * TILE_DISPLAY_SIZE + HEADER_H + FOOTER_H), 
                            "C++ Minesweeper", sf::Style::Titlebar | sf::Style::Close);

    // 加载资源
    sf::Texture tex[12]; // 0-8数字, F(9), M(10), X(11)
    for(int i=0; i<=8; ++i) tex[i].loadFromFile("./img/MINESWEEPER_" + std::to_string(i) + ".png");
    tex[9].loadFromFile("./img/MINESWEEPER_F.png");
    tex[10].loadFromFile("./img/MINESWEEPER_M.png");
    tex[11].loadFromFile("./img/MINESWEEPER_X.png");
    
    sf::Texture clockTex; clockTex.loadFromFile("./img/MINESWEEPER_C.png");
    sf::Font font; font.loadFromFile("msyh.ttc"); // 请确保路径下有微软雅黑或其它简中字体

    sf::Clock timerClock;
    int gameTime = 0;
    float timeAccumulator = 0;

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            if (e.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mPos = sf::Mouse::getPosition(window);
                
                // 1. 顶部难度切换区域点击 (左上角)
                if (mPos.y < HEADER_H && mPos.x < 150) {
                    currentLevel = (currentLevel + 1) % levels.size();
                    Difficulty d = levels[currentLevel];
                    game = Game(d.cols, d.rows, d.mines);
                    window.setSize(sf::Vector2u(d.cols * TILE_DISPLAY_SIZE, d.rows * TILE_DISPLAY_SIZE + HEADER_H + FOOTER_H));
                    window.setView(sf::View(sf::FloatRect(0, 0, d.cols * TILE_DISPLAY_SIZE, d.rows * TILE_DISPLAY_SIZE + HEADER_H + FOOTER_H)));
                    gameTime = 0;
                    continue;
                }

                // 2. 棋盘点击逻辑
                int gx = mPos.x / TILE_DISPLAY_SIZE;
                int gy = (mPos.y - HEADER_H) / TILE_DISPLAY_SIZE;

                if (gy >= 0 && gy < levels[currentLevel].rows && !game.getIsGameOver()) {
                    // 双键联动
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                        game.chordRevealCell(gy, gx);
                    } else if (e.mouseButton.button == sf::Mouse::Left) {
                        game.revealCell(gy, gx);
                    } else if (e.mouseButton.button == sf::Mouse::Right) {
                        game.flagCell(gy, gx);
                    }
                }
            }
        }

        // 计时逻辑：从第一次点击后开始
        if (!game.getIsFirstMove() && !game.getIsGameOver() && !game.checkWin()) {
            timeAccumulator += timerClock.restart().asSeconds();
            if (timeAccumulator >= 1.0f) {
                gameTime++;
                timeAccumulator -= 1.0f;
            }
        } else {
            timerClock.restart();
        }

        // 胜利处理
        if (game.checkWin()) {
            RecordManager::updateRecord(levels[currentLevel].name, gameTime);
        }

        window.clear(sf::Color(200, 200, 200));

        // 绘制顶部信息
        sf::Text headerText("难度: " + levels[currentLevel].name + " (点击切换)", font, 16);
        headerText.setFillColor(sf::Color::Black);
        headerText.setPosition(10, 10);
        window.draw(headerText);

        // 绘制棋盘
        for (int r = 0; r < levels[currentLevel].rows; ++r) {
            for (int c = 0; c < levels[currentLevel].cols; ++c) {
                sf::Sprite s;
                Block& b = game.getBoard().getBlock(r, c);
                if (!b.getIsRevealed()) {
                    s.setTexture(b.getIsFlagged() ? tex[9] : tex[11]);
                } else {
                    s.setTexture(b.getIsMine() ? tex[10] : tex[b.getAdjacentMines()]);
                }
                s.setScale(SCALE, SCALE);
                s.setPosition(c * TILE_DISPLAY_SIZE, r * TILE_DISPLAY_SIZE + HEADER_H);
                window.draw(s);
            }
        }

        // 绘制底部状态
        sf::Sprite cIcon(clockTex);
        cIcon.setScale(0.1f, 0.1f); // 假设计时器图标也很大
        cIcon.setPosition(10, window.getSize().y - 30);
        window.draw(cIcon);

        sf::Text timerText(std::to_string(gameTime) + "s", font, 18);
        timerText.setPosition(40, window.getSize().y - 32);
        timerText.setFillColor(sf::Color::Red);
        window.draw(timerText);

        sf::Text mineText("雷数: " + std::to_string(game.getRemainingMines()), font, 18);
        mineText.setPosition(window.getSize().x - 100, window.getSize().y - 32);
        mineText.setFillColor(sf::Color::Black);
        window.draw(mineText);

        window.display();
    }
    return 0;
}