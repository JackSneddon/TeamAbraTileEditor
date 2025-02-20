#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <queue>
#include <bitset>

const float TILESIZE = 32;
const float GRIDSIZE = 32;
const int WIDTH = 1920;
const int HEIGHT = 1080;
const int RULETILE = 7;   // The middle tile used when rule tiling
const int TILESWIDTH = 4; // width of tilesheet in tiles

std::vector<int> grid(GRIDSIZE *GRIDSIZE, 0);
int selectedTile = 0;
bool drawing = false;
sf::Vector2i hoveredTile(-1, -1);
sf::Texture sheet;
int sheetCols = 0;

bool loadSheet(int sheetNumber)
{
    std::string name = "./gfx/" + std::to_string(sheetNumber) + ".png";
    if (!sheet.loadFromFile(name))
    {
        std::cerr << "Couldnt load texture " << name << "\n";
        return false;
    }
    sheetCols = sheet.getSize().x / TILESIZE;
    return true;
}

void saveToCSV(const std::string &name)
{
    std::ofstream file(name);
    if (!file.is_open())
    {
        std::cerr << "couldn't open the csv, its already open or filename is wrong or smthn idk\n";
        return;
    }

    for (int y = 0; y < GRIDSIZE; y++)
    {
        for (int x = 0; x < GRIDSIZE; x++)
        {
            file << (grid[y * GRIDSIZE + x] == 1 ? 0 : grid[y * GRIDSIZE + x]);
            if (x < GRIDSIZE - 1)
                file << ",";
        }
        file << "\n";
    }
    file.close();
    std::cout << "grid saved " << name << "\n";
}

void loadCSV()
{
    std::string name;
    std::cout << "enter name of file to load in ('45632') DO NOT INCLUDE .csv!!!!!!!!\n";
    std::cin >> name;
    if (name == "" || name == " " || name == "\n")
    {
        return;
    }
    name += ".csv";
    std::ifstream file(name);
    if (!file.is_open())
    {
        std::cerr << "error loading the csv in\n";
        return;
    }

    std::string line;
    int row = 0;
    while (std::getline(file, line) && row < GRIDSIZE)
    {
        std::stringstream ss(line);
        std::string cell;
        int col = 0;
        while (std::getline(ss, cell, ',') && col < GRIDSIZE)
        {
            grid[row * GRIDSIZE + col] = std::stoi(cell) == 0 ? 1 : std::stoi(cell);
            col++;
        }
        row++;
    }
    file.close();
    std::cout << "island loaded from " << name << "\n";
}

void bucketFill(int x, int y, int newTile)
{
    int oldTile = grid[y * GRIDSIZE + x];
    if (oldTile == newTile)
        return;

    std::queue<sf::Vector2i> toFill;
    toFill.push({x, y});

    while (!toFill.empty())
    {
        sf::Vector2i pos = toFill.front();
        toFill.pop();

        int px = pos.x;
        int py = pos.y;
        if (px < 0 || px >= GRIDSIZE || py < 0 || py >= GRIDSIZE)
            continue;
        if (grid[py * GRIDSIZE + px] != oldTile)
            continue;

        grid[py * GRIDSIZE + px] = newTile;

        toFill.push({px + 1, py});
        toFill.push({px - 1, py});
        toFill.push({px, py + 1});
        toFill.push({px, py - 1});
    }
}

bool isValidForRuling(const int pos, int base)
{
    // if that position isn't the base and isn't the necessary tile
    return grid[pos] != base && (grid[pos] == 0 || grid[pos] == 1 || grid[pos] == RULETILE + (TILESWIDTH * 2) - 1 || grid[pos] == RULETILE + (TILESWIDTH * 2) + 1 || grid[pos] == RULETILE + (TILESWIDTH * 2));
}

void ruleIt()
{
    for (int x = 0; x < GRIDSIZE; x++)
    {
        for (int y = 0; y < GRIDSIZE; y++)
        {
            const int current = y * GRIDSIZE + x;

            if (grid[current] == RULETILE)
            {
                // foreach tile get its surrounding tils
                const int up = ((y - 1) * GRIDSIZE) + x;
                const int down = ((y + 1) * GRIDSIZE) + x;
                const int left = current - 1;
                const int right = current + 1;
                /*                 const int topLeft = up - 1;
                                const int topRight = up + 1;
                                const int bottomLeft = down - 1;
                                const int bottomRight = down + 1; */

                // bitmask for surrounding tiles
                int8_t surrounding = 0;
                // 0    0    0    0    0    0    0    0
                // TL  TR  BL  BR  U    D    L    R
                // 2     4    10  12  3    11  6    8

                // set bit flags
                surrounding |= isValidForRuling(right, RULETILE) ? 0b00000001 : 0; // right
                surrounding |= isValidForRuling(left, RULETILE) ? 0b00000010 : 0;  // left
                surrounding |= isValidForRuling(down, RULETILE) ? 0b00000100 : 0;  // down
                surrounding |= isValidForRuling(up, RULETILE) ? 0b00001000 : 0;    // up
                                                                                   /*                 surrounding |= isValid(bottomRight, 7, 12) ? 0b00010000 : 0; // bottomright
                                                                                                   surrounding |= isValid(bottomLeft, 7, 10) ? 0b00100000 : 0;  // bottomleft
                                                                                                   surrounding |= isValid(topRight, 7, 4) ? 0b01000000 : 0;     // topright
                                                                                                   surrounding |= isValid(topLeft, 7, 2) ? 0b10000000 : 0;      // topleft */

                // switch statement from flags
                switch (surrounding)
                {
                case 0b00000001: // right
                    grid[current] = RULETILE + 1;
                    break;
                case 0b00000010: // left
                    grid[current] = RULETILE - 1;
                    break;
                case 0b00000100: // down
                    grid[current] = RULETILE + TILESWIDTH;
                    grid[down] = RULETILE + (TILESWIDTH * 2);
                    break;
                case 0b00001000: // up
                    grid[current] = RULETILE - TILESWIDTH;
                    break;
                case 0b00000101: // bottom right
                    grid[current] = RULETILE + TILESWIDTH + 1;
                    grid[down] = RULETILE + (TILESWIDTH * 2) + 1;
                    break;
                case 0b00000110: // bottom left
                    grid[current] = RULETILE + TILESWIDTH - 1;
                    grid[down] = RULETILE + (TILESWIDTH * 2) - 1;
                    break;
                case 0b00001001: // top right
                    grid[current] = RULETILE - TILESWIDTH + 1;
                    break;
                case 0b00001010: // top left
                    grid[current] = RULETILE - TILESWIDTH - 1;
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void handleDrawing(const sf::RenderWindow &window, bool bucket)
{
    sf::Vector2i mPos = sf::Mouse::getPosition(window);
    if (mPos.x < GRIDSIZE * TILESIZE && mPos.y < GRIDSIZE * TILESIZE)
    {
        int gridX = mPos.x / TILESIZE;
        int gridY = mPos.y / TILESIZE;

        if (bucket)
        {
            bucketFill(gridX, gridY, selectedTile);
        }
        else
        {
            grid[gridY * GRIDSIZE + gridX] = selectedTile;
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Team Abra Tile Editor Of DOOM 6000", sf::Style::Default);
    loadSheet(0);

    sf::Sprite palette(sheet);
    palette.setPosition({GRIDSIZE * TILESIZE + 10, 0});

    sf::Vector2u sheetSize = sheet.getSize();
    int sheetCols = sheetSize.x / TILESIZE;

    sf::Font font("./gfx/font.ttf");
    sf::Text credit(font, "Made for CMP105 team abra\n Jack Sneddon - 2309340", 22);
    sf::Text controls(font, "CONTROLS:\n LeftClick - place tile \n RightClick - bucketFill\n R - apply ruling\n Escape - clear grid\n Enter - load from file (console)\n Space - save to working dir\n 1-9 - load that sheet from ./gfx/", 26);
    credit.setFillColor(sf::Color::White);
    credit.setPosition({WIDTH * 0.67f, HEIGHT * 0.88f});

    controls.setFillColor(sf::Color::White);
    controls.setPosition({WIDTH * 0.55f, HEIGHT * 0.15f});

    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2i mPos = sf::Mouse::getPosition(window);
                if (mPos.x > GRIDSIZE * TILESIZE + 10)
                { // Click on tilesheet
                    int tileX = (mPos.x - GRIDSIZE * TILESIZE - 10) / TILESIZE;
                    int tileY = mPos.y / TILESIZE;
                    selectedTile = tileY * sheetCols + tileX + 1;
                }
                else
                { // Click on grid
                    drawing = true;
                    handleDrawing(window, sf::Mouse::isButtonPressed(sf::Mouse::Button::Right));
                }
            }
            if (event->is<sf::Event::MouseButtonReleased>())
            {
                drawing = false;
            }
            if (event->is<sf::Event::MouseMoved>())
            {
                sf::Vector2i mPos = sf::Mouse::getPosition(window);
                if (mPos.x < GRIDSIZE * TILESIZE && mPos.y < GRIDSIZE * TILESIZE)
                {
                    hoveredTile.x = mPos.x / TILESIZE;
                    hoveredTile.y = mPos.y / TILESIZE;
                }
                else
                {
                    hoveredTile = {-1, -1};
                }
                if (event->is<sf::Event::MouseMoved>() && drawing)
                {
                    handleDrawing(window, sf::Mouse::isButtonPressed(sf::Mouse::Button::Right));
                }
            }
            if (event->is<sf::Event::KeyPressed>())
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
                {
                    const time_t now = time(0);
                    std::string name = std::to_string(now);
                    name += ".csv";
                    saveToCSV(name);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
                {
                    loadCSV();
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
                {
                    ruleIt();
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                {
                    for (int i = 0; i < GRIDSIZE * GRIDSIZE; i++)
                    {
                        grid[i] = 0;
                    }
                }

                // this is evil and I hate this but idk/can't be bothered doing the proper thing
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num0))
                {
                    loadSheet(0);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))
                {
                    loadSheet(1);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2))
                {
                    loadSheet(2);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
                {
                    loadSheet(3);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4))
                {
                    loadSheet(4);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5))
                {
                    loadSheet(5);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num6))
                {
                    loadSheet(6);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num7))
                {
                    loadSheet(7);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num8))
                {
                    loadSheet(8);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num9))
                {
                    loadSheet(9);
                }
            }
        }

        window.clear(sf::Color::Black);

        // Draw grid
        sf::RectangleShape cell(sf::Vector2f(TILESIZE, TILESIZE));
        cell.setOutlineThickness(1);
        cell.setOutlineColor(sf::Color(255, 255, 255, 75));
        cell.setFillColor(sf::Color::Black);

        sf::Sprite tile(sheet);
        for (int y = 0; y < GRIDSIZE; y++)
        {
            for (int x = 0; x < GRIDSIZE; x++)
            {
                cell.setPosition({x * TILESIZE, y * TILESIZE});
                window.draw(cell);
                int i = grid[y * GRIDSIZE + x];
                if (i > 0)
                {
                    int tileX = (i - 1) % sheetCols;
                    int tileY = (i - 1) / sheetCols;
                    tile.setTextureRect(sf::IntRect({tileX * TILESIZE, tileY * TILESIZE}, {TILESIZE, TILESIZE}));
                    tile.setPosition({x * TILESIZE, y * TILESIZE});
                    window.draw(tile);
                }
            }
        }

        // Draw hovered tile preview
        if (hoveredTile.x != -1 && hoveredTile.y != -1 && selectedTile > 0)
        {
            int tileX = (selectedTile - 1) % sheetCols;
            int tileY = (selectedTile - 1) / sheetCols;
            tile.setTextureRect(sf::IntRect({tileX * TILESIZE, tileY * TILESIZE}, {TILESIZE, TILESIZE}));
            tile.setPosition({hoveredTile.x * TILESIZE, hoveredTile.y * TILESIZE});
            tile.setColor(sf::Color(255, 255, 255, 150)); // Semi-transparent preview
            window.draw(tile);
        }

        // Draw tilesheet
        window.draw(palette);
        window.draw(credit);
        window.draw(controls);
        window.display();
    }
    return 0;
}
