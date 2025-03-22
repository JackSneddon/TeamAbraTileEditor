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
int RULETILE = 7;         // The middle tile used when rule tiling
const int TILESWIDTH = 4; // width of tilesheet in tiles

std::vector<int> gridLayer1(GRIDSIZE *GRIDSIZE, 0);
std::vector<int> gridLayer2(GRIDSIZE *GRIDSIZE, 0);
sf::Texture sheetLayer1, sheetLayer2;
int sheetColsLayer1 = 0, sheetColsLayer2 = 0;
int currentLayer = 1; // 1 for layer 1, 2 for layer 2
int selectedTile = 0;
bool drawing = false;
sf::Vector2i hoveredTile(-1, -1);
sf::RectangleShape palette; // Use RectangleShape for the palette

bool loadSheet(int sheetNumber, int layer)
{
    std::string name = "./gfx/" + std::to_string(sheetNumber) + ".png";
    sf::Texture &sheet = (layer == 1) ? sheetLayer1 : sheetLayer2;
    int &sheetCols = (layer == 1) ? sheetColsLayer1 : sheetColsLayer2;

    if (!sheet.loadFromFile(name))
    {
        std::cerr << "Couldnt load texture " << name << "\n";
        return false;
    }
    sheetCols = sheet.getSize().x / TILESIZE;

    // Update palette size if the current layer is being modified
    if (layer == currentLayer)
    {
        palette.setSize({static_cast<float>(sheet.getSize().x), static_cast<float>(sheet.getSize().y)});
        palette.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(palette.getSize())));
        palette.setTexture(&sheet);
    }

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
            file << (gridLayer1[y * GRIDSIZE + x] == 1 ? 0 : gridLayer1[y * GRIDSIZE + x]);
            if (x < GRIDSIZE - 1)
                file << ",";
        }
        file << "\n";
    }
    file << "\n";
    for (int y = 0; y < GRIDSIZE; y++)
    {
        for (int x = 0; x < GRIDSIZE; x++)
        {
            file << (gridLayer2[y * GRIDSIZE + x] == 1 ? 0 : gridLayer2[y * GRIDSIZE + x]);
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
            gridLayer1[row * GRIDSIZE + col] = std::stoi(cell) == 0 ? 1 : std::stoi(cell);
            col++;
        }
        row++;
    }
    row = 0;
    while (std::getline(file, line) && row < GRIDSIZE)
    {
        std::stringstream ss(line);
        std::string cell;
        int col = 0;
        while (std::getline(ss, cell, ',') && col < GRIDSIZE)
        {
            gridLayer2[row * GRIDSIZE + col] = std::stoi(cell) == 0 ? 1 : std::stoi(cell);
            col++;
        }
        row++;
    }
    file.close();
    std::cout << "island loaded from " << name << "\n";
}

void bucketFill(int x, int y, int newTile)
{
    std::vector<int> &grid = (currentLayer == 1) ? gridLayer1 : gridLayer2;
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
    return gridLayer1[pos] != base && (gridLayer1[pos] == 0 || gridLayer1[pos] == 1 || gridLayer1[pos] == RULETILE + (TILESWIDTH * 2) - 1 || gridLayer1[pos] == RULETILE + (TILESWIDTH * 2) + 1 || gridLayer1[pos] == RULETILE + (TILESWIDTH * 2));
}

void ruleIt()
{
    for (int x = 0; x < GRIDSIZE; x++)
    {
        for (int y = 0; y < GRIDSIZE; y++)
        {
            const int current = y * GRIDSIZE + x;

            if (gridLayer1[current] == RULETILE)
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
                    gridLayer1[current] = RULETILE + 1;
                    break;
                case 0b00000010: // left
                    gridLayer1[current] = RULETILE - 1;
                    break;
                case 0b00000100: // down
                    gridLayer1[current] = RULETILE + TILESWIDTH;
                    gridLayer1[down] = gridLayer1[current] + TILESWIDTH;
                    break;
                case 0b00001000: // up
                    gridLayer1[current] = RULETILE - TILESWIDTH;
                    break;
                case 0b00000101: // bottom right
                    gridLayer1[current] = RULETILE + TILESWIDTH + 1;
                    gridLayer1[down] = gridLayer1[current] + TILESWIDTH;
                    break;
                case 0b00000110: // bottom left
                    gridLayer1[current] = RULETILE + TILESWIDTH - 1;
                    gridLayer1[down] = gridLayer1[current] + TILESWIDTH;
                    break;
                case 0b00001001: // top right
                    gridLayer1[current] = RULETILE - TILESWIDTH + 1;
                    break;
                case 0b00001010: // top left
                    gridLayer1[current] = RULETILE - TILESWIDTH - 1;
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void handleDrawing(const sf::RenderWindow &window, bool bucket, bool erase)
{
    sf::Vector2i mPos = sf::Mouse::getPosition(window);
    if (mPos.x < GRIDSIZE * TILESIZE && mPos.y < GRIDSIZE * TILESIZE)
    {
        int gridX = mPos.x / TILESIZE;
        int gridY = mPos.y / TILESIZE;
        std::vector<int> &grid = (currentLayer == 1) ? gridLayer1 : gridLayer2;

        if (erase)
        {
            grid[gridY * GRIDSIZE + gridX] = 0; // Set tile to 0 (delete)
        }
        else if (bucket)
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
    loadSheet(0, 1);
    loadSheet(0, 2);

    sf::Texture &initialSheet = sheetLayer1; // Start with layer 1's texture
    sf::Vector2u initialSize = initialSheet.getSize();
    palette.setSize({static_cast<float>(initialSize.x), static_cast<float>(initialSize.y)});
    palette.setTexture(&initialSheet);
    palette.setPosition({GRIDSIZE * TILESIZE + 10, 0});

    sf::Font font("./gfx/font.ttf");
    sf::Text credit(font, "Made for CMP105 team abra\n Jack Sneddon - 2309340", 22);
    sf::Text controls(font, "CONTROLS:\n LeftClick - place tile \n RightClick - bucketFill\n MiddleClick - delete tile\n R - apply ruling\n K - set RULETILE to selected tile\n Escape - clear grid\n Enter - load from file (console)\n Space - save to working dir\n 1-9 - load that sheet from ./gfx/\n Tab - switch layer\n Current Layer: 1", 26);
    credit.setFillColor(sf::Color::White);
    credit.setPosition({WIDTH * 0.67f, HEIGHT * 0.88f});

    controls.setFillColor(sf::Color::White);
    controls.setPosition({WIDTH * 0.55f, HEIGHT * 0.35f});

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

                    // Calculate TilesHeight dynamically based on texture height
                    int sheetRows = ((currentLayer == 1) ? sheetLayer1 : sheetLayer2).getSize().y / TILESIZE;
                    selectedTile = tileY * sheetColsLayer1 + tileX + 1;

                    if (tileY >= sheetRows || tileX >= sheetColsLayer1)
                    {
                        selectedTile = 0; // Prevent selecting invalid tiles
                    }
                }
                else
                { // Click on grid
                    drawing = true;
                    handleDrawing(window, sf::Mouse::isButtonPressed(sf::Mouse::Button::Right), sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle));
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
                    handleDrawing(window, sf::Mouse::isButtonPressed(sf::Mouse::Button::Right), sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle));
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
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K))
                {
                    RULETILE = selectedTile;
                    std::cout << "RULETILE set to " << RULETILE << "\n";
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                {
                    for (int i = 0; i < GRIDSIZE * GRIDSIZE; i++)
                    {
                        gridLayer1[i] = 0;
                        gridLayer2[i] = 0;
                    }
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab))
                {
                    currentLayer = (currentLayer == 1) ? 2 : 1;

                    // Update palette size and texture
                    sf::Texture &currentSheet = (currentLayer == 1) ? sheetLayer1 : sheetLayer2;
                    sf::Vector2u textureSize = currentSheet.getSize();
                    palette.setSize({static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)});
                    palette.setTexture(&currentSheet);

                    controls.setString("CONTROLS:\n LeftClick - place tile \n RightClick - bucketFill\n MiddleClick - delete tile\n R - apply ruling\n K - set RULETILE to selected tile\n Escape - clear grid\n Enter - load from file (console)\n Space - save to working dir\n 1-9 - load that sheet from ./gfx/\n Tab - switch layer\n Current Layer: " + std::to_string(currentLayer)); // Update controls text
                    std::cout << "Switched to layer " << currentLayer << "\n";
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num0))
                {
                    loadSheet(0, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))
                {
                    loadSheet(1, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2))
                {
                    loadSheet(2, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
                {
                    loadSheet(3, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4))
                {
                    loadSheet(4, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5))
                {
                    loadSheet(5, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num6))
                {
                    loadSheet(6, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num7))
                {
                    loadSheet(7, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num8))
                {
                    loadSheet(8, currentLayer);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num9))
                {
                    loadSheet(9, currentLayer);
                }
            }
        }

        window.clear(sf::Color::Black);

        // Draw grid
        sf::RectangleShape cell(sf::Vector2f(TILESIZE, TILESIZE));
        cell.setOutlineThickness(1);
        cell.setOutlineColor(sf::Color(255, 255, 255, 75));
        cell.setFillColor(sf::Color::Black);

        sf::Sprite tile(sheetLayer1);
        for (int y = 0; y < GRIDSIZE; y++)
        {
            for (int x = 0; x < GRIDSIZE; x++)
            {
                cell.setPosition({x * TILESIZE, y * TILESIZE});
                window.draw(cell);
                int i = gridLayer1[y * GRIDSIZE + x];
                if (i > 0)
                {
                    int tileX = (i - 1) % sheetColsLayer1;
                    int tileY = (i - 1) / sheetColsLayer1;
                    tile.setTextureRect(sf::IntRect({tileX * TILESIZE, tileY * TILESIZE}, {TILESIZE, TILESIZE}));
                    tile.setPosition({x * TILESIZE, y * TILESIZE});
                    window.draw(tile);
                }
            }
        }

        tile.setTexture(sheetLayer2);
        for (int y = 0; y < GRIDSIZE; y++)
        {
            for (int x = 0; x < GRIDSIZE; x++)
            {
                int i = gridLayer2[y * GRIDSIZE + x];
                if (i > 0)
                {
                    int tileX = (i - 1) % sheetColsLayer2;
                    int tileY = (i - 1) / sheetColsLayer2;
                    tile.setTextureRect(sf::IntRect({tileX * TILESIZE, tileY * TILESIZE}, {TILESIZE, TILESIZE}));
                    tile.setPosition({x * TILESIZE, y * TILESIZE});
                    window.draw(tile);
                }
            }
        }

        // Draw hovered tile preview
        if (hoveredTile.x != -1 && hoveredTile.y != -1 && selectedTile > 0)
        {
            tile.setTexture((currentLayer == 1) ? sheetLayer1 : sheetLayer2); // Use correct layer's texture
            int tileX = (selectedTile - 1) % ((currentLayer == 1) ? sheetColsLayer1 : sheetColsLayer2);
            int tileY = (selectedTile - 1) / ((currentLayer == 1) ? sheetColsLayer1 : sheetColsLayer2);
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
