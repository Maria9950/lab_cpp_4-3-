#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

constexpr int SIZE = 8;
constexpr int CELL_SIZE = 64;
constexpr int COLORS = 5;

struct Cell {
    int color = -1;
    sf::RectangleShape shape;
    bool hasBonus = false;
    int bonusType = 0;
    int bonusColor = -1;
};

struct ActiveBonus {
    sf::Vector2i pos;
    int type;
    int bonusColor; 
    sf::Clock timer;
};

class Board {
public:
    Board();
    void initialize();
    bool trySwap(sf::Vector2i a, sf::Vector2i b);
    void update();
    void draw(sf::RenderWindow& w, sf::Sprite& paintSpr, sf::Sprite& bombSpr) const;

private:
    std::vector<std::vector<Cell>> _field;
    std::vector<ActiveBonus> _activeBonuses;
    bool _waitingForBonus = false;
    sf::Clock _bonusClock;

    static int randColor();
    static sf::Color toSFML(int code);
    static bool areNeighbors(int x1, int y1, int x2, int y2);

    bool markMatches(std::vector<std::vector<bool>>& toDel) const;
    void deleteMarked(const std::vector<std::vector<bool>>& toDel);
    void collapseField();
    void clearAllMatches();

    void applyPaintBonus(int x, int y, int color);
    void applyBombBonus(int x, int y);
};
