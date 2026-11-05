#include "PathFinder.h"
#include <SDL2/SDL.h>

//
//  Coordinate conversion — matches TileMapComponent::Draw EXACTLY.
//
//  TileMapComponent::Draw renders tile (col, row) so its CENTRE is at:
//      posX =  col * tileW - (mapW * tileW) / 2
//      posY = -(row * tileH - (mapH * tileH) / 2)
//
//  This is because the engine's quad is -0.5..0.5 centred on origin,
//  scaled by (tileW, tileH), and translated by (posX, posY).
//  So (posX, posY) IS the tile's centre.
//
//  Inverting for WorldToGrid:
//      col = (worldX + halfW) / tileW
//      row = (halfH - worldY) / tileH
//
GridPos PathFinder::WorldToGrid(const Vector2& world,
                                 const TileMapComponent* tm) const
{
    float halfW = (tm->GetMapWidth()  * tm->GetTileWidth())  * 0.5f;
    float halfH = (tm->GetMapHeight() * tm->GetTileHeight()) * 0.5f;

    // Add 0.5 before flooring so points near a tile centre round to that tile
    int col = static_cast<int>((world.x + halfW) / tm->GetTileWidth()  + 0.5f);
    int row = static_cast<int>((halfH - world.y) / tm->GetTileHeight() + 0.5f);

    col = std::max(0, std::min(col, tm->GetMapWidth()  - 1));
    row = std::max(0, std::min(row, tm->GetMapHeight() - 1));
    return { col, row };
}

Vector2 PathFinder::GridToWorld(const GridPos& gp,
                                 const TileMapComponent* tm) const
{
    float halfW = (tm->GetMapWidth()  * tm->GetTileWidth())  * 0.5f;
    float halfH = (tm->GetMapHeight() * tm->GetTileHeight()) * 0.5f;

    // The CENTRE of tile (col, row) — matches TileMapComponent::Draw
    float x =  gp.col * tm->GetTileWidth()  - halfW;
    float y = -(gp.row * tm->GetTileHeight() - halfH);
    return Vector2(x, y);
}

//
bool PathFinder::IsWalkable(const GridPos& gp,
                             const TileMapComponent* tm) const
{
    if (gp.col < 0 || gp.row < 0 ||
        gp.col >= tm->GetMapWidth() ||
        gp.row >= tm->GetMapHeight())
        return false;

    return tm->GetTileID(gp.col, gp.row) < mWallTileID;
}


GridPos PathFinder::FindNearestWalkable(const GridPos& target,
                                         const TileMapComponent* tm,
                                         int maxRadius) const
{
    if (IsWalkable(target, tm)) return target;

    for (int r = 1; r <= maxRadius; ++r)
    {
        for (int dr = -r; dr <= r; ++dr)
        {
            for (int dc = -r; dc <= r; ++dc)
            {
                if (std::abs(dr) != r && std::abs(dc) != r) continue;
                GridPos candidate{ target.col + dc, target.row + dr };
                if (IsWalkable(candidate, tm))
                    return candidate;
            }
        }
    }
    return target;
}

//
float PathFinder::Heuristic(const GridPos& a, const GridPos& b) const
{
    int dc = std::abs(a.col - b.col);
    int dr = std::abs(a.row - b.row);
    return static_cast<float>(dc + dr)
         + (1.414f - 2.0f) * static_cast<float>(std::min(dc, dr));
}

std::vector<std::pair<GridPos, float>>
PathFinder::GetNeighbours(const GridPos& gp,
                           const TileMapComponent* tm) const
{
    std::vector<std::pair<GridPos, float>> result;
    result.reserve(8);

    const int dx[]    = { 0,  0,  1, -1,  1,  1, -1, -1 };
    const int dy[]    = { 1, -1,  0,  0,  1, -1,  1, -1 };
    const float cost[]= { 1.f,1.f,1.f, 1.f, 1.414f,1.414f,1.414f,1.414f };

    for (int i = 0; i < 8; ++i)
    {
        GridPos nb{ gp.col + dx[i], gp.row + dy[i] };
        if (!IsWalkable(nb, tm)) continue;

        if (dx[i] != 0 && dy[i] != 0)
        {
            GridPos cardA{ gp.col + dx[i], gp.row };
            GridPos cardB{ gp.col,          gp.row + dy[i] };
            if (!IsWalkable(cardA, tm) || !IsWalkable(cardB, tm))
                continue;
        }

        result.push_back({ nb, cost[i] });
    }
    return result;
}

std::vector<GridPos> PathFinder::BuildPath(
    const std::unordered_map<GridPos, GridPos, GridPosHash>& cameFrom,
    const GridPos& start, const GridPos& goal) const
{
    std::vector<GridPos> path;
    GridPos current = goal;

    while (!(current == start))
    {
        path.push_back(current);
        auto it = cameFrom.find(current);
        if (it == cameFrom.end()) break;
        current = it->second;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

//
std::vector<Vector2> PathFinder::FindPath(
    const Vector2& worldStart,
    const Vector2& worldGoal,
    const TileMapComponent* tileMap) const
{
    if (!tileMap) return {};

    GridPos startGP = WorldToGrid(worldStart, tileMap);
    GridPos goalGP  = WorldToGrid(worldGoal,  tileMap);

    // Debug log so we can verify the math
    SDL_Log("PathFinder: start world(%.0f,%.0f) -> tile (%d,%d) | "
            "goal world(%.0f,%.0f) -> tile (%d,%d)",
            worldStart.x, worldStart.y, startGP.col, startGP.row,
            worldGoal.x,  worldGoal.y,  goalGP.col,  goalGP.row);

    if (!IsWalkable(goalGP, tileMap))
    {
        GridPos nearby = FindNearestWalkable(goalGP, tileMap, 6);
        if (IsWalkable(nearby, tileMap))
        {
            SDL_Log("PathFinder: goal (%d,%d) is a wall, using (%d,%d) instead",
                    goalGP.col, goalGP.row, nearby.col, nearby.row);
            goalGP = nearby;
        }
        else
        {
            SDL_Log("PathFinder: no walkable tile near goal");
            return {};
        }
    }

    if (!IsWalkable(startGP, tileMap))
    {
        GridPos nearby = FindNearestWalkable(startGP, tileMap, 3);
        if (IsWalkable(nearby, tileMap))
            startGP = nearby;
        else
        {
            SDL_Log("PathFinder: start is stuck in a wall");
            return {};
        }
    }

    if (startGP == goalGP)
        return { worldGoal };

    std::priority_queue<AStarEntry,
                        std::vector<AStarEntry>,
                        std::greater<AStarEntry>> openList;

    std::unordered_map<GridPos, GridPos, GridPosHash> cameFrom;
    std::unordered_map<GridPos, float,   GridPosHash> gCost;

    gCost[startGP] = 0.f;
    openList.push({ startGP, Heuristic(startGP, goalGP), 0.f });

    int maxIterations = tileMap->GetMapWidth() * tileMap->GetMapHeight() * 2;
    int iterations = 0;

    while (!openList.empty())
    {
        if (++iterations > maxIterations)
        {
            SDL_Log("PathFinder: gave up after %d iterations", iterations);
            return {};
        }

        AStarEntry current = openList.top();
        openList.pop();

        if (current.node == goalGP)
        {
            std::vector<GridPos> gridPath = BuildPath(cameFrom, startGP, goalGP);
            std::vector<Vector2> worldPath;
            worldPath.reserve(gridPath.size());

            for (const GridPos& gp : gridPath)
                worldPath.push_back(GridToWorld(gp, tileMap));

            SDL_Log("PathFinder: path found, %d waypoints (%d iterations)",
                    static_cast<int>(worldPath.size()), iterations);
            return worldPath;
        }

        auto gIt = gCost.find(current.node);
        if (gIt != gCost.end() && current.gCost > gIt->second + 0.001f)
            continue;

        for (auto& [nb, moveCost] : GetNeighbours(current.node, tileMap))
        {
            float newG = current.gCost + moveCost;
            auto it = gCost.find(nb);
            if (it == gCost.end() || newG < it->second)
            {
                gCost[nb]    = newG;
                cameFrom[nb] = current.node;
                float f      = newG + Heuristic(nb, goalGP);
                openList.push({ nb, f, newG });
            }
        }
    }

    SDL_Log("PathFinder: no path from (%d,%d) to (%d,%d)",
            startGP.col, startGP.row, goalGP.col, goalGP.row);
    return {};
}
