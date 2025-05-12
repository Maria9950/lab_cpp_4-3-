#include "Gems.h"
#include <cstdlib>
#include <cmath>
#include <random>
#include <algorithm>

Board::Board() : _field(SIZE, std::vector<Cell>(SIZE)) {}

int Board::randColor() { return std::rand() % COLORS; }

bool Board::areNeighbors(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2) == 1;
}

sf::Color Board::toSFML(int c) {
    switch (c) {
    case 0: return sf::Color::Red;
    case 1: return sf::Color::Green;
    case 2: return sf::Color::Blue;
    case 3: return sf::Color::Yellow;
    case 4: return sf::Color::Magenta;
    default: return sf::Color::Black;
    }
}

void Board::initialize() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            int c;
            do {
                c = randColor();
                if (j >= 2 && _field[i][j - 1].color == c && _field[i][j - 2].color == c)
                    continue;
                if (i >= 2 && _field[i - 1][j].color == c && _field[i - 2][j].color == c)
                    continue;
                break;
            } while (true);

            auto& cell = _field[i][j];
            cell.color = c;
            cell.hasBonus = false;
            cell.bonusType = 0;
            cell.shape.setSize({ CELL_SIZE - 2.f, CELL_SIZE - 2.f });
            cell.shape.setFillColor(toSFML(c));
            cell.shape.setPosition(j * CELL_SIZE + 1, i * CELL_SIZE + 1);
        }
    }
    clearAllMatches();
}

bool Board::markMatches(std::vector<std::vector<bool>>& toDel) const {
    bool found = false;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE - 2; ++j) {
            int c = _field[i][j].color;
            if (c != -1 && _field[i][j + 1].color == c && _field[i][j + 2].color == c) {
                found = true;
                for (int k = j; k < SIZE && _field[i][k].color == c; ++k)
                    toDel[i][k] = true;
            }
        }
    }
    for (int j = 0; j < SIZE; ++j) {
        for (int i = 0; i < SIZE - 2; ++i) {
            int c = _field[i][j].color;
            if (c != -1 && _field[i + 1][j].color == c && _field[i + 2][j].color == c) {
                found = true;
                for (int k = i; k < SIZE && _field[k][j].color == c; ++k)
                    toDel[k][j] = true;
            }
        }
    }
    return found;
}

void Board::deleteMarked(const std::vector<std::vector<bool>>& toDel) {
    std::vector<sf::Vector2i> allDels;
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            if (toDel[i][j]) allDels.emplace_back(i, j);

    if (!allDels.empty() && std::rand() % 100 < 20) {
        int randIndex = std::rand() % allDels.size();
        auto [xi, xj] = allDels[randIndex];
        int bonusColor = _field[xi][xj].color;

        std::vector<sf::Vector2i> candidates;
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                if (std::abs(i - xi) + std::abs(j - xj) <= 3 && !toDel[i][j] && !_field[i][j].hasBonus) {
                    candidates.emplace_back(i, j);
                }
            }
        }

        if (!candidates.empty()) {
            auto [bx, by] = candidates[std::rand() % candidates.size()];
            auto& tgt = _field[bx][by];
            tgt.hasBonus = true;
            tgt.bonusType = (std::rand() % 2) + 1;
            tgt.bonusColor = bonusColor;
            _activeBonuses.push_back({ {bx, by}, tgt.bonusType, bonusColor, sf::Clock() });
        }
    }

    for (auto& p : allDels) {
        auto& c = _field[p.x][p.y];
        c.color = -1;
        c.shape.setFillColor(sf::Color::Black);
        c.hasBonus = false;
    }
}

void Board::collapseField() {
    for (int j = 0; j < SIZE; ++j) {
        int write = SIZE - 1;
        for (int i = SIZE - 1; i >= 0; --i) {
            if (_field[i][j].color != -1) {
                _field[write][j] = _field[i][j];
                _field[write][j].shape.setPosition(j * CELL_SIZE + 1, write * CELL_SIZE + 1);
                --write;
            }
        }
        for (int i = write; i >= 0; --i) {
            int c = randColor();
            auto& nc = _field[i][j];
            nc.color = c;
            nc.hasBonus = false;
            nc.bonusType = 0;
            nc.shape.setFillColor(toSFML(c));
            nc.shape.setPosition(j * CELL_SIZE + 1, i * CELL_SIZE + 1);
        }
    }
}

void Board::clearAllMatches() {
    std::vector<std::vector<bool>> toDel(SIZE, std::vector<bool>(SIZE, false));
    while (true) {
        for (auto& row : toDel) std::fill(row.begin(), row.end(), false);
        if (!markMatches(toDel)) break;
        deleteMarked(toDel);
        collapseField();
    }
}

bool Board::trySwap(sf::Vector2i a, sf::Vector2i b) {
    if (!areNeighbors(a.x, a.y, b.x, b.y)) return false;

    std::swap(_field[a.x][a.y], _field[b.x][b.y]);
    _field[a.x][a.y].shape.setPosition(a.y * CELL_SIZE + 1, a.x * CELL_SIZE + 1);
    _field[b.x][b.y].shape.setPosition(b.y * CELL_SIZE + 1, b.x * CELL_SIZE + 1);

    std::vector<std::vector<bool>> td(SIZE, std::vector<bool>(SIZE));
    if (markMatches(td)) {
        deleteMarked(td);
        collapseField();
        clearAllMatches();
        _waitingForBonus = true;
        _bonusClock.restart();
        return true;
    }

    std::swap(_field[a.x][a.y], _field[b.x][b.y]);
    _field[a.x][a.y].shape.setPosition(a.y * CELL_SIZE + 1, a.x * CELL_SIZE + 1);
    _field[b.x][b.y].shape.setPosition(b.y * CELL_SIZE + 1, b.x * CELL_SIZE + 1);
    return false;
}

void Board::update() {
    if (!_waitingForBonus) return;
    if (_bonusClock.getElapsedTime().asSeconds() < 1.f) return;

    for (auto& b : _activeBonuses) {
        if (b.type == 1)
            applyPaintBonus(b.pos.x, b.pos.y, b.bonusColor);
        else
            applyBombBonus(b.pos.x, b.pos.y);
    }
    _activeBonuses.clear();
    collapseField();
    clearAllMatches();
    _waitingForBonus = false;
}

void Board::applyPaintBonus(int x, int y, int color) {
    sf::Vector2i origin = { x, y };
    std::vector<sf::Vector2i> nonNeighbors;

    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            if ((std::abs(i - origin.x) + std::abs(j - origin.y) <= 3) &&
                !(i == origin.x && j == origin.y) &&
                !areNeighbors(i, j, origin.x, origin.y))
                nonNeighbors.emplace_back(i, j);

    std::shuffle(nonNeighbors.begin(), nonNeighbors.end(), std::mt19937{ std::random_device{}() });

    if (nonNeighbors.size() >= 2) {
        _field[x][y].color = color;
        _field[x][y].shape.setFillColor(toSFML(color));
        _field[x][y].hasBonus = false;

        for (int i = 0; i < 2; ++i) {
            auto& c = _field[nonNeighbors[i].x][nonNeighbors[i].y];
            c.color = color;
            c.shape.setFillColor(toSFML(color));
            c.hasBonus = false;
        }
    }
}

void Board::applyBombBonus(int x, int y) {
    std::vector<sf::Vector2i> all;
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            all.emplace_back(i, j);

    std::shuffle(all.begin(), all.end(), std::mt19937{ std::random_device{}() });

    int destroyed = 0;
    for (const auto& p : all) {
        if (destroyed >= 5) break;
        auto& c = _field[p.x][p.y];
        if (c.color != -1) {
            c.color = -1;
            c.shape.setFillColor(sf::Color::Black);
            c.hasBonus = false;
            destroyed++;
        }
    }
}

void Board::draw(sf::RenderWindow& w, sf::Sprite& paintSpr, sf::Sprite& bombSpr) const {
    for (int i = 0; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            w.draw(_field[i][j].shape);

    if (_waitingForBonus) {
        for (auto& b : _activeBonuses) {
            sf::Sprite& s = (b.type == 1 ? paintSpr : bombSpr);
            s.setPosition(b.pos.y * CELL_SIZE + 8, b.pos.x * CELL_SIZE + 8);
            w.draw(s);
        }
    }
}
