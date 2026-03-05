#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "game.h"
#include "recordmanager.h"

// --- 全局配置 ---
const int TILE_SIZE = 45;
const float ORIG_IMG_SIZE = 216.0f; 
const float SCALE = TILE_SIZE / ORIG_IMG_SIZE;
const int HEADER_H = 100;
const int FOOTER_H = 60;
const unsigned int ACTIVE_FPS = 30;
const unsigned int BACKGROUND_FPS = 10;

struct Difficulty {
    std::string name;
    int rows, cols, mines;
};

std::vector<Difficulty> levels = {
    {"初级", 9, 9, 10},
    {"中级", 16, 16, 40},
    {"高级", 16, 30, 99},
    {"自定义", 0, 0, 0} // 占位符
};

sf::String toUtf8(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}


int main() {
    int curIdx = 0;
    Difficulty d = levels[curIdx];
    Game game(d.rows, d.cols, d.mines);
    
    sf::RenderWindow window(
        sf::VideoMode(d.cols * TILE_SIZE, d.rows * TILE_SIZE + HEADER_H + FOOTER_H),
        toUtf8(u8"Minesweeper --made by zzhovo"),
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    bool hasFocus = true;
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(ACTIVE_FPS);

    sf::Texture tex[12]; 
    for(int i=0; i<=8; ++i) tex[i].loadFromFile("./img/MINESWEEPER_" + std::to_string(i) + ".png");
    tex[9].loadFromFile("./img/MINESWEEPER_F.png");
    tex[10].loadFromFile("./img/MINESWEEPER_M.png");
    tex[11].loadFromFile("./img/MINESWEEPER_X.png");
    sf::Texture clockTex; clockTex.loadFromFile("./img/MINESWEEPER_C.png");

    sf::Font font;
    if (!font.loadFromFile("./font/MiSans-Regular.otf")) {
        std::cerr << "Error: Font file ./font/MiSans-Regular.otf not found!" << std::endl;
    }

    sf::Clock timer;
    int gameTime = 0;
    float acc = 0;
    bool showPopup = false;
    bool isWin = false;
    sf::FloatRect restartBtnRect;

    // 自定义难度界面变量
    bool showCustomPanel = false;
    std::string inputs[3] = {"", "", ""}; // 行, 列, 雷
    int inputIdx = 0;
    std::string errorMsg = "";

    while (window.isOpen()) {
        const int boardWidth = d.cols * TILE_SIZE;
        const int boardHeight = d.rows * TILE_SIZE;
        const int boardOffsetX = ((int)window.getSize().x - boardWidth) / 2;
        const int boardOffsetY = HEADER_H + (((int)window.getSize().y - HEADER_H - FOOTER_H - boardHeight) / 2);

        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)e.size.width, (float)e.size.height)));
            }
            if (e.type == sf::Event::LostFocus) {
                hasFocus = false;
                window.setFramerateLimit(BACKGROUND_FPS);
            }
            if (e.type == sf::Event::GainedFocus) {
                hasFocus = true;
                window.setFramerateLimit(ACTIVE_FPS);
            }

            // 文本输入逻辑 (仅在自定义面板显示时)
            if (showCustomPanel && e.type == sf::Event::TextEntered) {
                if (e.text.unicode >= '0' && e.text.unicode <= '9' && inputs[inputIdx].size() < 3) {
                    inputs[inputIdx] += static_cast<char>(e.text.unicode);
                } else if (e.text.unicode == 8 && !inputs[inputIdx].empty()) { // Backspace
                    inputs[inputIdx].pop_back();
                }
            }

            if (e.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f m = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                
                // 1. 自定义面板点击逻辑
                if (showCustomPanel) {
                    // 点击输入框切换焦点
                    for (int i = 0; i < 3; ++i) {
                        if (sf::FloatRect(window.getSize().x/2 - 50, 150 + i*50, 100, 30).contains(m.x, m.y)) inputIdx = i;
                    }
                    // 确定按钮
                    if (sf::FloatRect(window.getSize().x/2 - 40, 320, 80, 40).contains(m.x, m.y)) {
                        if (inputs[0].empty() || inputs[1].empty() || inputs[2].empty()) {
                            errorMsg = u8"设置不完整";
                        } else {
                            int r = std::stoi(inputs[0]), c = std::stoi(inputs[1]), n = std::stoi(inputs[2]);
                            if (r < 9 || c < 9) errorMsg = u8"边长需大于等于 9";
                            else if (n >= r * c) errorMsg = u8"雷数过多";
                            else {
                                d = {u8"自定义", r, c, n};
                                curIdx = 3;
                                game = Game(d.rows, d.cols, d.mines);
                                window.create(
                                    sf::VideoMode(d.cols * TILE_SIZE, d.rows * TILE_SIZE + HEADER_H + FOOTER_H),
                                    toUtf8(u8"Minesweeper --made by zzhovo"),
                                    sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
                                );
                                window.setView(sf::View(sf::FloatRect(0, 0, (float)window.getSize().x, (float)window.getSize().y)));
                                window.setVerticalSyncEnabled(true);
                                window.setFramerateLimit(hasFocus ? ACTIVE_FPS : BACKGROUND_FPS);
                                showCustomPanel = false; gameTime = 0;
                            }
                        }
                    }
                    // 关闭按钮
                    if (sf::FloatRect(window.getSize().x/2 + 100, 105, 30, 30).contains(m.x, m.y)) showCustomPanel = false;
                    continue; 
                }

                // 2. 难度切换栏点击
                if (m.y > 10 && m.y < 60) { 
                    float bWidth = (float)window.getSize().x / 4.0f;
                    int clickedIdx = (int)(m.x / bWidth);
                    if (clickedIdx == 3) { showCustomPanel = true; errorMsg = ""; }
                    else if (clickedIdx >= 0 && clickedIdx < 3 && clickedIdx != curIdx) {
                        curIdx = clickedIdx; d = levels[curIdx];
                        game = Game(d.rows, d.cols, d.mines);
                        window.create(
                            sf::VideoMode(d.cols * TILE_SIZE, d.rows * TILE_SIZE + HEADER_H + FOOTER_H),
                            toUtf8(u8"Minesweeper --made by zzhovo"),
                            sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
                        );
                        window.setView(sf::View(sf::FloatRect(0, 0, (float)window.getSize().x, (float)window.getSize().y)));
                        window.setVerticalSyncEnabled(true);
                        window.setFramerateLimit(hasFocus ? ACTIVE_FPS : BACKGROUND_FPS);
                        gameTime = 0; showPopup = false;
                    }
                    continue;
                }

                // 3. 弹窗重开点击
                if (showPopup && restartBtnRect.contains(m.x, m.y)) {
                    game = Game(d.rows, d.cols, d.mines);
                    gameTime = 0; showPopup = false; continue;
                }

                // 4. 棋盘点击 
                int gx = ((int)m.x - boardOffsetX) / TILE_SIZE;
                int gy = ((int)m.y - boardOffsetY) / TILE_SIZE;
                if (gy >= 0 && gy < d.rows && gx >= 0 && gx < d.cols && !game.getIsGameOver() && !showPopup) {
                    bool L = sf::Mouse::isButtonPressed(sf::Mouse::Left);
                    bool R = sf::Mouse::isButtonPressed(sf::Mouse::Right);
                    if (L && R) game.chordRevealCell(gy, gx);
                    else if (e.mouseButton.button == sf::Mouse::Left) game.revealCell(gy, gx);
                    else if (e.mouseButton.button == sf::Mouse::Right) game.flagCell(gy, gx);
                }
            }
        }

        // 逻辑更新
        if (!game.getIsFirstMove() && !game.getIsGameOver() && !showPopup && !showCustomPanel) {
            acc += timer.restart().asSeconds();
            if (acc >= 1.0f) { gameTime++; acc -= 1.0f; }
            if (game.checkWin()) { isWin = true; showPopup = true; RecordManager::updateRecord(d.name, gameTime); }
        } else { timer.restart(); }
        if (game.getIsGameOver() && !showPopup) { isWin = false; showPopup = true; }

        window.clear(sf::Color(230, 230, 230));

        // 绘制难度栏 (4个按钮)
        float bW = (float)window.getSize().x / 4.0f;
        for (int i = 0; i < 4; ++i) {
            sf::RectangleShape bRect(sf::Vector2f(bW - 6, 45));
            bRect.setPosition(i * bW + 3, 10);
            bRect.setFillColor(i == curIdx ? sf::Color(80, 80, 80) : sf::Color(180, 180, 180));
            window.draw(bRect);
            sf::Text bText(toUtf8(levels[i].name), font, 18);
            sf::FloatRect tr = bText.getLocalBounds();
            bText.setPosition(i * bW + bW/2 - tr.width/2, 20);
            window.draw(bText);
        }

        // 绘制棋盘
        for (int r = 0; r < d.rows; ++r) {
            for (int c = 0; c < d.cols; ++c) {
                sf::Sprite s; Block& b = game.getBoard().getBlock(r, c);
                if (!b.getIsRevealed()) s.setTexture(b.getIsFlagged() ? tex[9] : tex[11]);
                else s.setTexture(b.getIsMine() ? tex[10] : tex[b.getAdjacentMines()]);
                s.setScale(SCALE, SCALE);
                s.setPosition((float)(boardOffsetX + c * TILE_SIZE), (float)(boardOffsetY + r * TILE_SIZE));
                window.draw(s);
            }
        }

        // 底部状态栏
        float fy = (float)window.getSize().y - 45;
        sf::Text tv(std::to_string(gameTime), font, 26); tv.setFillColor(sf::Color::Red); tv.setPosition(65, fy); window.draw(tv);
        sf::Sprite cIcon(clockTex); cIcon.setScale(0.18f, 0.18f); cIcon.setPosition(15, fy - 5); window.draw(cIcon);
        
        sf::Text mv(std::to_string(game.getRemainingMines()), font, 26);
        mv.setFillColor(sf::Color::Black); sf::FloatRect mvr = mv.getLocalBounds();
        float mineX = (float)window.getSize().x - 30 - mvr.width;
        mv.setPosition(mineX, fy); window.draw(mv);
        sf::Sprite mIcon(tex[10]); mIcon.setScale(0.18f, 0.18f); mIcon.setPosition(mineX - 45, fy - 5); window.draw(mIcon);

        // --- 弹窗与自定义面板 ---
        if (showPopup || showCustomPanel) {
            sf::RectangleShape mask(sf::Vector2f(window.getSize().x, window.getSize().y));
            mask.setFillColor(sf::Color(0, 0, 0, 160)); window.draw(mask);

            if (showCustomPanel) { // 自定义面板
                sf::RectangleShape box(sf::Vector2f(260, 300));
                box.setPosition(window.getSize().x/2 - 130, 100); box.setFillColor(sf::Color::White); window.draw(box);
                
                sf::Text title(toUtf8(u8"自定义难度"), font, 20); title.setFillColor(sf::Color::Black);
                title.setPosition(window.getSize().x/2 - 50, 110); window.draw(title);

                std::string labels[3] = {u8"行数:", u8"列数:", u8"雷数:"};
                for(int i=0; i<3; ++i) {
                    sf::Text t(toUtf8(labels[i]), font, 18); t.setFillColor(sf::Color::Black);
                    t.setPosition(window.getSize().x/2 - 110, 155 + i*50); window.draw(t);
                    sf::RectangleShape field(sf::Vector2f(100, 30)); 
                    field.setPosition(window.getSize().x/2 - 50, 150 + i*50);
                    field.setOutlineThickness(inputIdx == i ? 2.0f : 1.0f);
                    field.setOutlineColor(inputIdx == i ? sf::Color::Blue : sf::Color::Black);
                    window.draw(field);
                    sf::Text val(inputs[i], font, 18); val.setFillColor(sf::Color::Black);
                    val.setPosition(window.getSize().x/2 - 45, 152 + i*50); window.draw(val);
                }
                if(!errorMsg.empty()) {
                    sf::Text err(toUtf8(errorMsg), font, 16); err.setFillColor(sf::Color::Red);
                    err.setPosition(window.getSize().x/2 - 50, 290); window.draw(err);
                }
                // 确定按钮
                sf::RectangleShape okBtn(sf::Vector2f(80, 40)); okBtn.setPosition(window.getSize().x/2 - 40, 320);
                okBtn.setFillColor(sf::Color(100, 200, 100)); window.draw(okBtn);
                sf::Text okT(toUtf8(u8"确定"), font, 18); okT.setPosition(window.getSize().x/2 - 18, 328); window.draw(okT);
            } else if (showPopup) { // 结果弹窗
                float boxW = 340, boxH = 260; float boxX = window.getSize().x/2 - boxW/2; float boxY = window.getSize().y/2 - boxH/2;
                sf::RectangleShape box(sf::Vector2f(boxW, boxH)); box.setFillColor(sf::Color::White); box.setPosition(boxX, boxY); window.draw(box);
                sf::Text msg(isWin ? toUtf8(u8"恭喜胜利！") : toUtf8(u8"遗憾失败"), font, 30);
                msg.setFillColor(isWin ? sf::Color(0,120,0) : sf::Color::Red); msg.setPosition(window.getSize().x/2 - msg.getLocalBounds().width/2, boxY + 20); window.draw(msg);
                std::string statsStr = u8"本局用时: " + std::to_string(gameTime) + "s\n" + u8"最快记录: " + std::to_string(RecordManager::getBestTime(d.name)) + "s";
                sf::Text stats(toUtf8(statsStr), font, 18); stats.setFillColor(sf::Color(50,50,50)); stats.setPosition(window.getSize().x/2 - stats.getLocalBounds().width/2, boxY + 85); window.draw(stats);
                restartBtnRect = sf::FloatRect(window.getSize().x/2 - 80, boxY + 180, 160, 50);
                sf::RectangleShape btn(sf::Vector2f(160, 50)); btn.setPosition(restartBtnRect.left, restartBtnRect.top); btn.setFillColor(sf::Color(240,240,240)); window.draw(btn);
                sf::Text bt(toUtf8(u8"再来一局"), font, 22); bt.setFillColor(sf::Color::Black); bt.setPosition(restartBtnRect.left + 80 - bt.getLocalBounds().width/2, restartBtnRect.top + 10); window.draw(bt);
            }
        }
        window.display();
        sf::sleep(sf::milliseconds(1));
    }
    return 0;
}
