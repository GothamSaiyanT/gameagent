#pragma once
#ifndef NPCACTOR_H
#define NPCACTOR_H

#include "Actor.h"
#include "Math.h"
#include "PathFinder.h"
#include <vector>
#include <string>

class AnimSpriteComponent;

//  NPCActor  —  Non-player character that roams the map randomly.
//
//  Behaviour:
//    1. Every few seconds, pick a random walkable tile within ROAM_RADIUS
//    2. Use A* to find a path to that tile (respects collision layer)
//    3. Walk the path waypoint by waypoint
//    4. When arrived (or path failed), wait a bit then pick a new target
//
//  Each NPC owns its own AnimSpriteComponent which you set up after
//  construction by calling AddIdleFrames() / AddWalkFrames() with your
//  sprite textures.
class NPCActor : public Actor
{
public:
    explicit NPCActor(class Game* game, const std::string& name = "NPC");

    void UpdateActor(float deltaTime) override;

    // Configuration
    // Call these AFTER construction to set up the sprite.
    // Pass any textures you want \u2014 the asc registers them under those names.
    void AddIdleFrames(const std::vector<class Texture*>& frames);
    void AddWalkFrames(const std::vector<class Texture*>& frames);

    void SetRoamRadius(int tiles) { mRoamRadiusTiles = tiles; }
    void SetRoamSpeed (float px)  { mWalkSpeed       = px; }
    void SetWaitTime  (float lo, float hi) { mWaitMin = lo; mWaitMax = hi; }

    const std::string& GetName() const { return mName; }

private:
    void PickNewDestination();
    void UpdateWalking(float dt);

    // Sprite
    AnimSpriteComponent* asc = nullptr;

    // Pathfinding
    PathFinder           mPathFinder;
    std::vector<Vector2> mWaypoints;
    int                  mCurrentWP = 0;

    // Roam state
    bool   mWalking      = false;
    float  mWaitTimer    = 0.f;     // counts down — when 0, pick new target
    float  mWaitMin      = 1.5f;    // seconds to idle between walks
    float  mWaitMax      = 3.5f;
    float  mWalkSpeed    = 120.f;   // slower than student so they don't dominate
    int    mRoamRadiusTiles = 6;    // pick new target within this many tiles
    Vector2 mSpawnPos    {0.f, 0.f};

    static constexpr float ARRIVE_RADIUS = 8.0f;

    std::string mName;
};

#endif
