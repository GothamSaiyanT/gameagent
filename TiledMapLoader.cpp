#include "TiledMapLoader.h"
#include "Game.h"
#include "Actor.h"
#include "SpriteComponent.h"
#include "Renderer.h"
#include "Texture.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <cstdio>

void TiledMapLoader::LoadObjectLayer(
    const std::string& filename,
    const std::string& layerName,
    Game* game)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
        return;

    char buffer[65536];
    rapidjson::FileReadStream stream(fp, buffer, sizeof(buffer));

    rapidjson::Document doc;
    doc.ParseStream(stream);
    fclose(fp);

    if (!doc.IsObject() || !doc.HasMember("layers"))
        return;


        /*int mapHeight = doc["height"].GetInt();
    int tileHeight = doc["tileheight"].GetInt();
    float mapPixelHeight = mapHeight * tileHeight;*/

    const auto& layers = doc["layers"].GetArray();

    for (const auto& layer : layers)
    {
        if (!layer.HasMember("name") ||
            layer["name"].GetString() != layerName)
            continue;

        if (std::string(layer["type"].GetString()) != "objectgroup")
            continue;

        const auto& objects = layer["objects"].GetArray();

        for (const auto& obj : objects)
        {
            if (!obj.HasMember("gid"))
                continue;

            int gid = obj["gid"].GetInt();
            float x = obj["x"].GetFloat();
            float y = obj["y"].GetFloat();
            float w = obj["width"].GetFloat();
            float h = obj["height"].GetFloat();

           /* Vector2 pos;
            pos.x = x + w * 0.5f;
            pos.y = mapPixelHeight - y - h * 0.5f;*/

            Texture* tex = TextureFromGid(gid, game);
            if (!tex)
                continue;

            Actor* a = new Actor(game);

            // Convert from Tiled space → your world space
            Vector2 pos(
                30.0f-(x - w * 0.5f),
                400.0f-(y - h * 0.5f)
            );
            a->SetPosition(pos);

            SpriteComponent* sc = new SpriteComponent(a, 50);
            sc->SetTexture(tex);
        }

        return;
    }
}
void TiledMapLoader::LoadCollisionObjects(
    const std::string& file,
    const std::string& layerName,
    TileMapComponent* tileMap)
{
    FILE* fp = fopen(file.c_str(), "rb");
    if (!fp) return;

    char buffer[65536];
    rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    int mapHeight = doc["height"].GetInt();
    int tileHeight = doc["tileheight"].GetInt();
    float mapPixelHeight = mapHeight * tileHeight;

    const auto& layers = doc["layers"];
    for (rapidjson::SizeType i = 0; i < layers.Size(); i++)
    {
        const auto& layer = layers[i];
        if (layer["name"].GetString() != layerName)
            continue;

        const auto& objects = layer["objects"];
        for (rapidjson::SizeType j = 0; j < objects.Size(); j++)
        {
            const auto& obj = objects[j];

            float x = obj["x"].GetFloat();
            float y = obj["y"].GetFloat();
            float w = obj["width"].GetFloat();
            float h = obj["height"].GetFloat();

            //  Tiled Y-down → engine Y-up

Vector2 origin = tileMap->GetMapOrigin();

float worldX = origin.x + x;
float worldY = origin.y + (mapPixelHeight - y);

tileMap->AddCollisionRect(worldX, worldY, w, h);
        }
        break;
    }
}
void TiledMapLoader::LoadTileLayer(
    const std::string& filename,
    const std::string& layerName,
    TileMapComponent* tileMap)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
        return;

    char buffer[65536];
    rapidjson::FileReadStream stream(fp, buffer, sizeof(buffer));
    rapidjson::Document doc;
    doc.ParseStream(stream);
    fclose(fp);

    if (!doc.HasMember("layers"))
        return;

    const auto& layers = doc["layers"].GetArray();

    for (const auto& layer : layers)
    {
        if (!layer.HasMember("type") ||
            std::string(layer["type"].GetString()) != "tilelayer")
            continue;

        if (layer["name"].GetString() != layerName)
            continue;

        int width  = layer["width"].GetInt();
        int height = layer["height"].GetInt();

        const auto& data = layer["data"].GetArray();

        std::vector<std::vector<int>> map(height,
            std::vector<int>(width));

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = y * width + x;
                int gid = data[index].GetInt();

                // Tiled: 0 = empty → convert to -1
                map[y][x] = (gid == 0) ? -1 : (gid - 1);
            }
        }

        tileMap->SetMapData(map);
        return;
    }
}


