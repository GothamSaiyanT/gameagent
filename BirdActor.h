#pragma once
#ifndef BIRDACTOR_H
#define BIRDACTOR_H

#include "Actor.h"
#include "Math.h"
#include <vector>

class AnimSpriteComponent;
class Texture;

class BirdActor : public Actor
{
public:
    explicit BirdActor(class Game* game);

    void UpdateActor(float deltaTime) override;

    void SetFrames(const std::vector<Texture*>& frames);
    void SetSpeed(float pxPerSec)           { mSpeed = pxPerSec; }
    void SetFlightLevel(float worldY)       { mFlightY = worldY; }
    void SetMapExtents(float left, float right) {
        mMapLeft = left; mMapRight = right;
    }

private:
    AnimSpriteComponent* asc = nullptr;

    float mSpeed   = 80.f;      // pixels per second (horizontal)
    float mFlightY = 400.f;     // base Y altitude (above the map)

    // Map wrap bounds (so the bird flies off-screen left and reappears right)
    float mMapLeft  = -1100.f;
    float mMapRight =  1100.f;
};

#endif
