#include "TileMapComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Texture.h"
#include "CSVConverter.h"

TileMapComponent::TileMapComponent(Actor* owner, int drawOrder)
    : SpriteComponent(owner, drawOrder)
    , mTileWidth(32)
    , mTileHeight(32)
    , mMapWidth(0)
    , mMapHeight(0)
{
}

void TileMapComponent::SetTexture(Texture* texture)
{
    SpriteComponent::SetTexture(texture);
}

void TileMapComponent::SetTileSize(int width, int height)
{
    mTileWidth = width;
    mTileHeight = height;
}

void TileMapComponent::SetMapData(const std::vector<std::vector<int>>& data)
{
    mMapChips = data; // Direct assignment now works
    UpdateMapDimensions();
}

bool TileMapComponent::LoadFromCSV(const std::string& filename)
{
    mMapChips = CSVConverter::Convert(filename); // Direct assignment
    if(mMapChips.empty()) return false;

    UpdateMapDimensions();
    return true;
}

void TileMapComponent::UpdateMapDimensions()
{
    if(!mMapChips.empty())
    {
        mMapHeight = mMapChips.size();
        if(mMapHeight > 0)
        {
            mMapWidth = mMapChips[0].size();
        }
    }
}

void TileMapComponent::Draw(Shader* shader)
{
    if (mTexture && !mMapChips.empty())
    {
        mTexture->SetActive();

        int tilesPerRow = mTexture->GetWidth() / mTileWidth;
        int tilesPerCol = mTexture->GetHeight() / mTileHeight;
        float uvTileWidth = 1.0f / tilesPerRow;
        float uvTileHeight = 1.0f / tilesPerCol;

        for(int y = 0; y < mMapHeight; y++)
        {
            for(int x = 0; x < mMapWidth; x++)
            {
                int tileID = mMapChips[y][x]; // Access 2D vector
                if(tileID < 0) continue;

                Matrix4 scaleMat = Matrix4::CreateScale(
                    static_cast<float>(mTileWidth),
                    static_cast<float>(mTileHeight),
                    1.0f);

                float posX = x * mTileWidth - (mMapWidth * mTileWidth)/2.0f;
                float posY = y * mTileHeight - (mMapHeight * mTileHeight)/2.0f;

                Matrix4 transMat = Matrix4::CreateTranslation(
                    Vector3(posX, -posY, 0.0f));

                Matrix4 world = scaleMat * transMat * mOwner->GetWorldTransform();
                shader->SetMatrixUniform("uWorldTransform", world);

                int tileX = tileID % tilesPerRow;
                int tileY = tileID / tilesPerRow;

                Vector2 uvOffset(tileX * uvTileWidth, tileY * uvTileHeight);
                Vector2 uvScale(uvTileWidth, uvTileHeight);
                shader->SetIntUniform("uUseUVTransform", 1);
                shader->SetVector2Uniform("uUVOffset", uvOffset);
                shader->SetVector2Uniform("uUVScale", uvScale);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            }
        }
    }
}
