#ifndef TILEDMAPLOADER_H_INCLUDED
#define TILEDMAPLOADER_H_INCLUDED

#pragma once
#include <string>

class Game;
class Texture;
class TileMapComponent;

class TiledMapLoader
{
public:
    static void LoadObjectLayer(
        const std::string& filename,
        const std::string& layerName,
        Game* game
    );
        static void LoadTileLayer(
        const std::string& filename,
        const std::string& layerName,
        TileMapComponent* tileMap);
    static void LoadCollisionObjects(
    const std::string& file,
    const std::string& layerName,
    TileMapComponent* tileMap);

private:
    static Texture* TextureFromGid(int gid, Game* game);
};



#endif // TILEDMAPLOADER_H_INCLUDED
