#include "NPCActor.h"
#include "AnimSpriteComponent.h"
#include "Game.h"
#include "TileMapComponent.h"
#include <SDL2/SDL.h>
#include <cstdlib>
#include <cmath>


NPCActor::NPCActor(Game* game, const std::string& name)
    : Actor(game)
    , mPathFinder(0)   // tile ID >= 0 = wall (same as Student)
    , mName(name)
{
    asc = new AnimSpriteComponent(this, 134);  // draw under student (which is 91)
    asc->SetAnimFPS(8.0f);

    // Initial wait so all 3 NPCs don't move at once on spawn
    mWaitTimer = (rand() % 100) / 100.f * 2.f;
}


void NPCActor::AddIdleFrames(const std::vector<Texture*>& frames)
{
    if (asc && !frames.empty()) asc->SetAnimTextures("Idle", frames, true);
}

void NPCActor::AddWalkFrames(const std::vector<Texture*>& frames)
{
    if (asc && !frames.empty()) asc->SetAnimTextures("Walk", frames, true);
}

void NPCActor::UpdateActor(float dt)
{
    if (GetGame()->IsTrainingMode()) return;
    // Remember spawn position the first time we update (since constructor
    // runs before Game::LoadData sets the position)
    if (mSpawnPos.x == 0.f && mSpawnPos.y == 0.f)
        mSpawnPos = GetPosition();

    if (mWalking)
    {
        UpdateWalking(dt);
        return;
    }

    // Waiting between walks
    mWaitTimer -= dt;
    if (mWaitTimer <= 0.f)
        PickNewDestination();
}

//  PickNewDestination —> choose a random walkable tile within ROAM_RADIUS,
//                       run A*, start walking if a path was found
void NPCActor::PickNewDestination()
{
    TileMapComponent* tileMap = GetGame()->GetTileMap();
    if (!tileMap) return;

    // Convert current world position to tile coords
    int tileW = tileMap->GetTileWidth();
    int tileH = tileMap->GetTileHeight();
    float halfW = tileMap->GetMapWidth()  * tileW * 0.5f;
    float halfH = tileMap->GetMapHeight() * tileH * 0.5f;

    Vector2 pos = GetPosition();
    int currCol = static_cast<int>((pos.x + halfW) / tileW + 0.5f);
    int currRow = static_cast<int>((halfH - pos.y) / tileH + 0.5f);

    // Try up to 8 random nearby tiles
    for (int attempt = 0; attempt < 8; ++attempt)
    {
        int dCol = (rand() % (2 * mRoamRadiusTiles + 1)) - mRoamRadiusTiles;
        int dRow = (rand() % (2 * mRoamRadiusTiles + 1)) - mRoamRadiusTiles;
        if (dCol == 0 && dRow == 0) continue;

        int targetCol = currCol + dCol;
        int targetRow = currRow + dRow;

        // Skip out-of-bounds
        if (targetCol < 0 || targetCol >= tileMap->GetMapWidth())  continue;
        if (targetRow < 0 || targetRow >= tileMap->GetMapHeight()) continue;

        // Skip walls
        if (tileMap->GetTileID(targetCol, targetRow) >= 0) continue;

        // Convert target tile back to world coord
        float targetX =  targetCol * tileW - halfW;
        float targetY = -(targetRow * tileH - halfH);

        // Try A*
        mWaypoints = mPathFinder.FindPath(pos, Vector2(targetX, targetY), tileMap);
        if (!mWaypoints.empty())
        {
            mCurrentWP = 0;
            mWalking   = true;
            if (asc) asc->SetAnimation("Walk");
            return;
        }
    }

    // No valid target found — wait a bit then try again
    mWaitTimer = mWaitMin + (rand() % 1000) / 1000.f * (mWaitMax - mWaitMin);
    if (asc) asc->SetAnimation("Idle");
}

void NPCActor::UpdateWalking(float dt)
{
    if (mWaypoints.empty() || mCurrentWP >= static_cast<int>(mWaypoints.size()))
    {
        // Arrived — start waiting
        mWalking   = false;
        mWaitTimer = mWaitMin + (rand() % 1000) / 1000.f * (mWaitMax - mWaitMin);
        if (asc) asc->SetAnimation("Idle");
        return;
    }

    Vector2 pos    = GetPosition();
    Vector2 target = mWaypoints[mCurrentWP];
    Vector2 diff   = target - pos;
    float   dist   = diff.Length();

    if (dist <= ARRIVE_RADIUS)
    {
        SetPosition(target);
        mCurrentWP++;
        return;
    }

    Vector2 dir = diff;
    dir.Normalize();

    // Flip sprite based on horizontal movement
    if (asc)
    {
        if      (dir.x >  0.05f) asc->SetFlipX(false);
        else if (dir.x < -0.05f) asc->SetFlipX(true);
    }

    float step = mWalkSpeed * dt;
    if (step > dist) step = dist;
    SetPosition(pos + dir * step);
}
