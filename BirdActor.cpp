#include "BirdActor.h"
#include "AnimSpriteComponent.h"
#include "Game.h"
#include "Renderer.h"
#include "Texture.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>

BirdActor::BirdActor(Game* game) : Actor(game)
{
    // Birds draw above buildings but below HUD
    asc = new AnimSpriteComponent(this, 150);
    asc->SetAnimFPS(10.0f);
}

void BirdActor::SetFrames(const std::vector<Texture*>& frames)
{
    if (asc && !frames.empty())
    {
        asc->SetAnimTextures("Fly", frames, true);
        asc->SetAnimation("Fly");
    }
}

void BirdActor::UpdateActor(float dt)
{
    if (GetGame()->IsTrainingMode()) return;

    Vector2 pos = GetPosition();

    // Move horizontally
    pos.x += mSpeed * dt;

    // Wrap around — when off-screen right, teleport to off-screen left
    if (mSpeed > 0.f && pos.x > mMapRight)
        pos.x = mMapLeft;
    else if (mSpeed < 0.f && pos.x < mMapLeft)
        pos.x = mMapRight;

    SetPosition(pos);

    // Flip sprite based on flight direction
    if (asc)
    {
        if (mSpeed > 0.f) asc->SetFlipX(false);  // facing right
        else              asc->SetFlipX(true);   // facing left
    }
}
