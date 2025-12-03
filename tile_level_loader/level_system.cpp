#include "level_system.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

// -------------------------
// Static member definitions
// -------------------------

// Raw tile data for the loaded level, stored in row-major order.
std::unique_ptr<LevelSystem::Tile[]> LevelSystem::_tiles;

// Level dimensions (in tiles).
int LevelSystem::_width = 0;
int LevelSystem::_height = 0;

// World-space offset for the top-left of the level.
// Lets us move the whole grid around if needed.
sf::Vector2f LevelSystem::_offset(0.f, 0.f);

// Size of a single tile (width/height in pixels).
float LevelSystem::_tile_size = 100.f;

// Cached world position of the "start" tile (for spawning the player).
sf::Vector2f LevelSystem::_start_position(0.f, 0.f);

// One drawable rect per tile, built from the tile data.
std::vector<std::unique_ptr<sf::RectangleShape>> LevelSystem::_sprites;

// Colour lookup table for each tile type.
std::map<LevelSystem::Tile, sf::Color> LevelSystem::_colors{
    { WALL,     sf::Color(200, 200, 200) },
    { END,      sf::Color(255,  80,  80) },
    { START,    sf::Color(80, 255,  80) },
    { EMPTY,    sf::Color(25,  25,  25) },
    { ENEMY,    sf::Color(255, 180,   0) },
    { WAYPOINT, sf::Color(80, 160, 255) }
};

// -------------------------
// Simple getters
// -------------------------

int LevelSystem::get_height() { return _height; }
int LevelSystem::get_width() { return _width; }
sf::Vector2f LevelSystem::get_start_position() { return _start_position; }

// Look up the colour for a specific tile type.
sf::Color LevelSystem::get_color(LevelSystem::Tile t) {
    auto it = _colors.find(t);
    if (it == _colors.end()) return sf::Color::Transparent;
    return it->second;
}

// Override the colour used when drawing a specific tile type.
void LevelSystem::set_color(LevelSystem::Tile t, sf::Color c) {
    _colors[t] = c;
}

// Convert from grid coordinates (tile x,y) to world coordinates (pixels).
sf::Vector2f LevelSystem::get_tile_position(sf::Vector2i p) {
    return _offset + sf::Vector2f(p.x * _tile_size, p.y * _tile_size);
}

// Get the tile type at a specific grid coordinate.
// Throws if the coordinates are out of range.
LevelSystem::Tile LevelSystem::get_tile(sf::Vector2i p) {
    if (p.x < 0 || p.y < 0 || p.x >= _width || p.y >= _height) {
        throw std::string("Tile out of range: ") + std::to_string(p.x) + "," + std::to_string(p.y);
    }
    return _tiles[(p.y * _width) + p.x];
}

// Get the tile type at a world-space position (pixels).
// This does a simple floor(v / tile_size) to map back into grid space.
LevelSystem::Tile LevelSystem::get_tile_at(sf::Vector2f v) {
    const sf::Vector2f a = v - _offset;
    if (a.x < 0 || a.y < 0) throw std::string("Tile out of range");
    const sf::Vector2i grid = sf::Vector2i(a / _tile_size);
    return get_tile(grid);
}

// -------------------------
// Sprite building
// -------------------------

// Build one rectangle sprite per tile so the level can be drawn quickly.
void LevelSystem::build_sprites() {
    _sprites.clear();
    _sprites.reserve(_width * _height);

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            auto rect = std::make_unique<sf::RectangleShape>();

            // Position the rect at the correct world-space tile position.
            rect->setPosition(get_tile_position({ x, y }));
            rect->setSize({ _tile_size, _tile_size });

            // Pick colour based on the tile type at this grid position.
            rect->setFillColor(get_color(get_tile({ x, y })));

            _sprites.push_back(std::move(rect));
        }
    }
}

// -------------------------
// Level loading
// -------------------------

// Load a level from a text file and build tile/sprite data.
// The file is treated as a grid of characters:
//   'w' = wall, 's' = start, 'e' = end, ' ' = empty,
//   '+' = waypoint, 'n' = enemy lane.
// Newlines mark the end of a row.
void LevelSystem::load_level(const std::string& path, float tile_size) {
    _tile_size = tile_size;
    _width = 0;
    _height = 0;
    _start_position = { 0.f, 0.f };
    _tiles.reset();
    _sprites.clear();

    // Read whole file into a single string buffer.
    std::string buffer;
    std::ifstream f(path);
    if (f.good()) {
        f.seekg(0, std::ios::end);
        buffer.resize(static_cast<size_t>(f.tellg()));
        f.seekg(0);
        f.read(&buffer[0], buffer.size());
        f.close();
    }
    else {
        throw std::string("Couldn't open level file: ") + path;
    }

    int x = 0;       // current column
    int w = 0, h = 0; // final width/height in tiles
    std::vector<Tile> temp; // temporary storage for parsed tiles

    // Parse each character and convert it to a Tile.
    for (size_t i = 0; i < buffer.size(); ++i) {
        const char c = buffer[i];
        switch (c) {
        case 'w':
            temp.push_back(WALL);
            break;
        case 's':
            temp.push_back(START);
            // When we see the start tile, cache its world position.
            _start_position = get_tile_position({ x, h });
            break;
        case 'e':
            temp.push_back(END);
            break;
        case ' ':
            temp.push_back(EMPTY);
            break;
        case '+':
            temp.push_back(WAYPOINT);
            break;
        case 'n':
            temp.push_back(ENEMY);
            break;

        case '\r':
            // Ignore carriage return (Windows line endings).
            continue;

        case '\n':
            // End of row.
            if (w == 0) w = x;  // First newline -> we now know the width.
            x = 0;
            ++h;
            continue;

        default:
            // Anything else is treated as an unknown tile but we don't abort;
            std::cout << "Unknown tile: '" << c << "'\n";
            continue;
        }
        ++x;
    }

    // If the last line didn't end with '\n', we still need to count it.
    if (x != 0) {
        ++h;
        if (w == 0) w = x;
    }

    // Check the number of parsed tiles must match width * height.
    if (static_cast<int>(temp.size()) != w * h) {
        throw std::string("Can't parse level file: wrong size (") +
            std::to_string(temp.size()) + " vs " + std::to_string(w * h) + ")";
    }

    // Copy the temporary vector into our contiguous tile array.
    _tiles = std::make_unique<Tile[]>(w * h);
    _width = w;
    _height = h;
    std::copy(temp.begin(), temp.end(), &_tiles[0]);

    // Build one drawable rect per tile.
    build_sprites();
    std::cout << "Level " << path << " Loaded: " << w << "x" << h << "\n";
}

// -------------------------
// Rendering
// -------------------------

// Draw every tile sprite to the window.
void LevelSystem::render(sf::RenderWindow& window) {
    const size_t N = static_cast<size_t>(_width) * static_cast<size_t>(_height);
    for (size_t i = 0; i < N; ++i) {
        window.draw(*_sprites[i]);
    }
}
