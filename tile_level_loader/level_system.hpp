#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

class LevelSystem {
public:
    // Types of tiles we support in the level file
    enum Tile { EMPTY, START, END, WALL, ENEMY, WAYPOINT };

    // Load a level text file and build tiles/sprites
    static void load_level(const std::string& path, float tile_size = 100.f);

    // Draw all level tiles
    static void render(sf::RenderWindow& window);

    // Colour helpers for each tile type
    static sf::Color get_color(Tile t);
    static void set_color(Tile t, sf::Color c);

    // Query tile type by grid or world position
    static Tile get_tile(sf::Vector2i grid);
    static Tile get_tile_at(sf::Vector2f world);

    // Convert grid coords to world position (top-left of tile)
    static sf::Vector2f get_tile_position(sf::Vector2i grid);

    // Level dimensions and start position
    static int get_height();
    static int get_width();
    static sf::Vector2f get_start_position();

protected:
    // Raw tile data (row-major order)
    static std::unique_ptr<Tile[]> _tiles;
    static int _width;
    static int _height;

    // Global offset + tile size in pixels
    static sf::Vector2f _offset;
    static float _tile_size;

    // Per-tile colours and cached start position
    static std::map<Tile, sf::Color> _colors;
    static sf::Vector2f _start_position;

    // One rect per tile for drawing
    static std::vector<std::unique_ptr<sf::RectangleShape>> _sprites;
    static void build_sprites();

private:
    LevelSystem() = delete;
    ~LevelSystem() = delete;
};
