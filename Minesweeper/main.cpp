#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "game.h"
#include "recordmanager.h"

// --- 全局配置 ---
const int TILE_SIZE = 30;               // 方块尺寸
const float ORIG_IMG_SIZE = 216.0f; 
const float SCALE = TILE_SIZE / ORIG_IMG_SIZE;
const int HEADER_H = 80;                // 顶部按钮区高度
const int FOOTER_H = 50;                // 底部状态栏高度 (稍微加高防止被切)

struct Difficulty {
    std::string name;
    int rows, cols, mines;
};

std::vector<Difficulty> levels = {
    {"初级", 9, 9, 10},
    {"中级", 16, 16, 40},
    {"高级", 16, 30, 99} // 30列 x 16行
};

sf::String toUtf8(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

bool findFont(sf::Font& font) {
    std::string paths[] = {
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "msyh.ttc"
    };
    for (const std::string& p : paths) { if (font.loadFromFile(p)) return true; }
    return false;
}

int main() {
    int curIdx = 0;
    Difficulty d = levels[curIdx];
    Game game(d.cols, d.rows, d.mines);
    
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(d.cols * TILE_SIZE, d.rows * TILE_SIZE + HEADER_H + FOOTER_H), 
                            toUtf8(u8"扫雷专业版"), sf::Style::Titlebar | sf::Style::Close);

    // ★★★ 关键修复 1：初始化时设置视图，保证坐标系 1:1 ★★★
    window.setView(sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y)));

    // 加载资源
    sf::Texture tex[12]; 
    for(int i=0; i<=8; ++i) tex[i].loadFromFile("./img/MINESWEEPER_" + std::to_string(i) + ".png");
    tex[9].loadFromFile("./img/MINESWEEPER_F.png");
    tex[10].loadFromFile("./img/MINESWEEPER_M.png");
    tex[11].loadFromFile("./img/MINESWEEPER_X.png");
    sf::Texture clockTex; clockTex.loadFromFile("./img/MINESWEEPER_C.png");

    sf::Font font;
    if (!findFont(font)) std::cerr << "Warning: No Chinese font found." << std::endl;

    sf::Clock timer;
    int gameTime = 0;
    float acc = 0;
    bool showPopup = false;
    bool isWin = false;

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            if (e.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i m = sf::Mouse::getPosition(window);
                
                // 1. 顶部难度切换
                if (m.y > 10 && m.y < 50) { // 按钮区域
                    float bWidth = (float)window.getSize().x / 3.0f;
                    int clickedIdx = (int)(m.x / bWidth);
                    
                    if (clickedIdx >= 0 && clickedIdx < 3 && clickedIdx != curIdx) {
                        curIdx = clickedIdx;
                        d = levels[curIdx];
                        game = Game(d.cols, d.rows, d.mines);
                        
                        // ★★★ 关键修复 2：切换难度后，必须重置 View ★★★
                        // 1. 重建窗口
                        window.create(sf::VideoMode(d.cols * TILE_SIZE, d.rows * TILE_SIZE + HEADER_H + FOOTER_H), 
                                      toUtf8(u8"扫雷专业版"), sf::Style::Titlebar | sf::Style::Close);
                        // 2. 强制重置视图为新窗口大小 (解决点击错位和显示不全的核心)
                        window.setView(sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y)));
                        
                        gameTime = 0; 
                        showPopup = false;
                        continue; // 跳过当前帧剩余逻辑
                    }
                }

                if (showPopup) {
                    game = Game(d.cols, d.rows, d.mines);
                    gameTime = 0; showPopup = false;
                    continue;
                }

                // 2. 棋盘点击 (坐标转换)
                int gx = m.x / TILE_SIZE;
                int gy = (m.y - HEADER_H) / TILE_SIZE;

                if (gy >= 0 && gy < d.rows && gx >= 0 && gx < d.cols && !game.getIsGameOver()) {
                    // ★★★ 关键修复 3：优先检测双键 (Chord) ★★★
                    // SFML 的 MouseButtonPressed 是一次触发一个键，所以这里检测状态
                    bool leftDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
                    bool rightDown = sf::Mouse::isButtonPressed(sf::Mouse::Right);

                    if (leftDown && rightDown) {
                        game.chordRevealCell(gy, gx);
                    } 
                    else if (e.mouseButton.button == sf::Mouse::Left) {
                        // 只有左键按下且右键没按时，才算普通揭开
                        if (!rightDown) game.revealCell(gy, gx);
                    } 
                    else if (e.mouseButton.button == sf::Mouse::Right) {
                        // 只有右键按下且左键没按时，才算插旗
                        if (!leftDown) game.flagCell(gy, gx);
                    }
                }
            }
        }

        // 逻辑更新
        if (!game.getIsFirstMove() && !game.getIsGameOver() && !showPopup) {
            acc += timer.restart().asSeconds();
            if (acc >= 1.0f) { gameTime++; acc -= 1.0f; }
            if (game.checkWin()) { isWin = true; showPopup = true; RecordManager::updateRecord(d.name, gameTime); }
        } else { timer.restart(); }
        if (game.getIsGameOver() && !showPopup) { isWin = false; showPopup = true; }

        window.clear(sf::Color(220, 220, 220));

        // --- 绘制难度按钮 ---
        float bW = (float)window.getSize().x / 3.0f;
        for (int i = 0; i < 3; ++i) {
            sf::RectangleShape bRect(sf::Vector2f(bW - 4, 35));
            bRect.setPosition(i * bW + 2, 10);
            // 选中态深色，未选中浅色
            bRect.setFillColor(i == curIdx ? sf::Color(100, 100, 100) : sf::Color(190, 190, 190));
            bRect.setOutlineThickness(1);
            bRect.setOutlineColor(sf::Color::Black);
            window.draw(bRect);

            sf::Text bText(toUtf8(levels[i].name), font, 18);
            bText.setFillColor(i == curIdx ? sf::Color::White : sf::Color::Black);
            sf::FloatRect tr = bText.getLocalBounds();
            bText.setPosition(i * bW + bW/2 - tr.width/2, 16);
            window.draw(bText);
        }

        // --- 绘制棋盘 ---
        for (int r = 0; r < d.rows; ++r) {
            for (int c = 0; c < d.cols; ++c) {
                sf::Sprite s;
                Block& b = game.getBoard().getBlock(r, c);
                if (!b.getIsRevealed()) s.setTexture(b.getIsFlagged() ? tex[9] : tex[11]);
                else s.setTexture(b.getIsMine() ? tex[10] : tex[b.getAdjacentMines()]);
                s.setScale(SCALE, SCALE);
                s.setPosition((float)c * TILE_SIZE, (float)r * TILE_SIZE + HEADER_H);
                window.draw(s);
            }
        }

        // --- 绘制底部 ---
        float fy = (float)window.getSize().y - 40;
        sf::Sprite cIcon(clockTex); cIcon.setScale(0.12f, 0.12f); cIcon.setPosition(10, fy); window.draw(cIcon);
        sf::Text tv(std::to_string(gameTime), font, 20); tv.setFillColor(sf::Color::Red); tv.setPosition(45, fy - 2); window.draw(tv);
        
        // 动态计算右侧位置，确保贴边
        sf::Sprite mIcon(tex[10]); mIcon.setScale(SCALE, SCALE); mIcon.setPosition((float)window.getSize().x - 70, fy); window.draw(mIcon);
        sf::Text mv(std::to_string(game.getRemainingMines()), font, 20); mv.setFillColor(sf::Color::Black); mv.setPosition((float)window.getSize().x - 35, fy - 2); window.draw(mv);

        // --- 弹出框 ---
        if (showPopup) {
            sf::RectangleShape mask(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
            mask.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(mask);

            sf::RectangleShape box(sf::Vector2f(200, 100));
            box.setFillColor(sf::Color::White);
            box.setOutlineThickness(2);
            box.setOutlineColor(sf::Color::Black);
            box.setPosition((float)window.getSize().x/2 - 100, (float)window.getSize().y/2 - 50);
            window.draw(box);

            sf::Text msg(isWin ? toUtf8(u8"胜利！") : toUtf8(u8"炸了！"), font, 24);
            msg.setFillColor(isWin ? sf::Color::Blue : sf::Color::Red);
            sf::FloatRect mr = msg.getLocalBounds();
            msg.setPosition((float)window.getSize().x/2 - mr.width/2, (float)window.getSize().y/2 - 35);
            window.draw(msg);

            sf::Text sub(toUtf8(u8"点击任意处重开"), font, 14);
            sub.setFillColor(sf::Color::Black);
            sf::FloatRect sr = sub.getLocalBounds();
            sub.setPosition((float)window.getSize().x/2 - sr.width/2, (float)window.getSize().y/2 + 10);
            window.draw(sub);
        }

        window.display();
    }
    return 0;
}