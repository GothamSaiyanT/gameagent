#pragma once
#ifndef PATHFINDER_H
#define PATHFINDER_H

//
//  PathFinder  —  A* pathfinding for the Student Agent
//
//  Used by StudentActor::TransitionTo() to find a walkable route to a
//  destination, navigating around buildings.
//
//  Treats tiles with ID >= 0 in the COLLISION layer as walls.
//  Tiles with ID = -1 (empty in that layer) are walkable.
//
//  Approach to "the goal IS a building":
//    Buildings are walls — A* can never path INTO a building tile.
//    If the goal is a wall (which it always is for our 5 locations),
//    we find the nearest walkable tile near the goal and use that instead.
//    That way the student walks UP TO the building entrance.
//

#include "Math.h"
#include "TileMapComponent.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

//  Tile grid coordinate
struct GridPos
{
    int col = 0;
    int row = 0;

    bool operator==(const GridPos& o) const
    { return col == o.col && row == o.row; }
};

// Hash for unordered_map keys
struct GridPosHash
{
    std::size_t operator()(const GridPos& g) const
    {
        return std::hash<int>()(g.col) ^ (std::hash<int>()(g.row) << 16);
    }
};

// Open-list entry — sorted by fCost in the priority queue
struct AStarEntry
{
    GridPos node;
    float   fCost = 0.f;
    float   gCost = 0.f;

    bool operator>(const AStarEntry& o) const { return fCost > o.fCost; }
};

//
class PathFinder
{
public:
    // wallTileID: any tile with this ID OR HIGHER counts as a wall
    // For us, the collision layer has either -1 (empty/walkable) or >=0 (building)
    // so wallTileID = 0 makes any non-empty tile blocking
    explicit PathFinder(int wallTileID = 0) : mWallTileID(wallTileID) {}

    //  Main entry point
    // Returns world-space waypoints from start to goal.
    // Empty vector means no path found.
    std::vector<Vector2> FindPath(
        const Vector2& worldStart,
        const Vector2& worldGoal,
        const TileMapComponent* tileMap) const;

    //  Helpers
    GridPos WorldToGrid(const Vector2& world,
                        const TileMapComponent* tm) const;

    Vector2 GridToWorld(const GridPos& gp,
                        const TileMapComponent* tm) const;

    bool IsWalkable(const GridPos& gp,
                    const TileMapComponent* tm) const;

    // Find the nearest walkable tile to the given position (used when the
    // goal is inside a building — we find the closest accessible tile)
    GridPos FindNearestWalkable(const GridPos& target,
                                 const TileMapComponent* tm,
                                 int maxRadius = 6) const;

private:
    int mWallTileID;

    // Octile distance heuristic (admissible for 8-directional grids)
    float Heuristic(const GridPos& a, const GridPos& b) const;

    std::vector<GridPos> BuildPath(
        const std::unordered_map<GridPos, GridPos, GridPosHash>& cameFrom,
        const GridPos& start, const GridPos& goal) const;

    // 8-directional neighbours with movement costs
    std::vector<std::pair<GridPos, float>> GetNeighbours(
        const GridPos& gp, const TileMapComponent* tm) const;
};

#endif
