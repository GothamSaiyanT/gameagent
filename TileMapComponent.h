#ifndef TILEMAPCOMPONENT_H_INCLUDED
#define TILEMAPCOMPONENT_H_INCLUDED

#pragma once
#include "SpriteComponent.h"
#include "Texture.h"
#include <vector>
#include <string>
#include "VertexArray.h"

class TileMapComponent : public SpriteComponent
{
public:
    TileMapComponent(class Actor* owner, int drawOrder = 100);

    void SetTexture(class Texture* texture) override;
    void SetTileSize(int width, int height);

    // Changed to accept 2D vector directly
    void SetMapData(const std::vector<std::vector<int>>& data);

    bool LoadFromCSV(const std::string& filename);
    void Draw(class Shader* shader) override;

    int GetMapWidth() const { return mMapWidth; }
    int GetMapHeight() const { return mMapHeight; }
    int GetTileWidth() const { return mTileWidth; }
    int GetTileHeight() const { return mTileHeight; }
        int GetTileID(int col, int row) const
    {
        if (row < 0 || row >= mMapHeight) return -1;
        if (col < 0 || col >= mMapWidth)  return -1;
        return mMapChips[row][col];
    }

private:
    std::vector<std::vector<int>> mMapChips; // Changed to 2D vector
    int mTileWidth;
    int mTileHeight;
    int mMapWidth;
    int mMapHeight;

    void UpdateMapDimensions();
};



#endif // TILEMAPCOMPONENT_H_INCLUDED
