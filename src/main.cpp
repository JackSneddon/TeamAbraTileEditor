#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>

const float TILESIZE = 32;
const float GRIDSIZE = 16;
const int WIDTH = 800;
const int HEIGHT = 512;

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

void handleDrawing(const sf::RenderWindow &window)
{
    sf::Vector2i mPos = sf::Mouse::getPosition(window);
    if (mPos.x < GRIDSIZE * TILESIZE && mPos.y < GRIDSIZE * TILESIZE)
    {
        int gridX = mPos.x / TILESIZE;
        int gridY = mPos.y / TILESIZE;
        grid[gridY * GRIDSIZE + gridX] = selectedTile;
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
                    handleDrawing(window);
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
                    handleDrawing(window);
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
        window.display();
    }
    return 0;
}
