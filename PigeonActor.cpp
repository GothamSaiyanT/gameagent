#include "PigeonActor.h"
#include "AnimSpriteComponent.h"
#include "Game.h"
#include "Texture.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>

PigeonActor::PigeonActor(Game* game) : Actor(game)
{
    // Pigeons draw at 89 — below NPCs (90) and student (91) so people
    // walk over them, which looks correct for ground-level birds.
    asc = new AnimSpriteComponent(this, 89);
    asc->SetAnimFPS(6.0f);   // slow shuffle

    // Randomise starting angle so pigeons don't all face the same way
    mAngle = (rand() % 1000) / 1000.0f * 6.28f;

    // Randomise radius slightly so the flock doesn't look mechanical
    mRadius       = 12.f + (rand() % 16);      // 12-28 px
    mAngularSpeed = 0.4f + (rand() % 100) / 200.0f;  // 0.4-0.9 rad/sec
}

void PigeonActor::SetWalkFrames(const std::vector<Texture*>& frames)
{
    if (asc && !frames.empty())
    {
        asc->SetAnimTextures("Walk", frames, true);
        asc->SetAnimation("Walk");
    }
}

void PigeonActor::UpdateActor(float dt)
{
    if (GetGame()->IsTrainingMode()) return;
    // Capture spawn position as the circle centre on first update
    // (constructor runs before Game::LoadData sets the position)
    if (!mCenterSet)
    {
        mCenter = GetPosition();
        mCenterSet = true;
        mLastX = mCenter.x;
    }

    mAngle += mAngularSpeed * dt;

    Vector2 newPos = mCenter + Vector2(
        std::cos(mAngle) * mRadius,
        std::sin(mAngle) * mRadius);

    // Flip sprite based on direction of motion (compare to last X position)
    if (asc)
    {
        float dx = newPos.x - mLastX;
        if      (dx >  0.01f) asc->SetFlipX(false);  // moving right
        else if (dx < -0.01f) asc->SetFlipX(true);   // moving left
    }
    mLastX = newPos.x;

    SetPosition(newPos);
}
