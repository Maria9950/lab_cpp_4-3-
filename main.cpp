#include <SFML/Graphics.hpp>
#include "Gems.h"
#include <ctime>

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    sf::RenderWindow window({ SIZE * CELL_SIZE, SIZE * CELL_SIZE }, "GEMS");

    sf::Texture paintTx, bombTx;
    paintTx.loadFromFile("paint.png");
    bombTx.loadFromFile("bomb.png");

    sf::Sprite paintSp(paintTx), bombSp(bombTx);
    float scale = (CELL_SIZE - 16) / float(paintTx.getSize().x);
    paintSp.setScale(scale, scale);
    bombSp.setScale(scale, scale);

    Board board;
    board.initialize();

    bool selecting = false;
    sf::Vector2i first;

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                window.close();
            if (e.type == sf::Event::MouseButtonPressed &&
                e.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i pos{ e.mouseButton.y / CELL_SIZE, e.mouseButton.x / CELL_SIZE };
                if (!selecting) {
                    first = pos;
                    selecting = true;
                }
                else {
                    board.trySwap(first, pos);
                    selecting = false;
                }
            }
        }

        board.update();
        window.clear(sf::Color::White);
        board.draw(window, paintSp, bombSp);
        window.display();
    }

    return 0;
}
