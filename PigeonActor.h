#pragma once
#ifndef PIGEONACTOR_H
#define PIGEONACTOR_H

#include "Actor.h"
#include "Math.h"
#include <vector>

class AnimSpriteComponent;
class Texture;

// ─────────────────────────────────────────────────────────────────────────────
//  PigeonActor  —  hops in a tiny circle around its spawn point.
//
//  No pathfinding, no collision detection — pigeons are decoration.
//  They walk slowly in a small circle, with randomised radius and speed
//  so a flock of them doesn't look synchronised.
//
//  Spawn them anywhere on the map (e.g. around the park door) and they
//  add ambience without any CPU cost beyond a sine/cosine per frame.
// ─────────────────────────────────────────────────────────────────────────────
class PigeonActor : public Actor
{
public:
    explicit PigeonActor(class Game* game);

    void UpdateActor(float deltaTime) override;

    void SetWalkFrames(const std::vector<Texture*>& frames);

    // Override defaults if you want
    void SetCircleRadius(float r) { mRadius = r; }
    void SetCircleSpeed (float s) { mAngularSpeed = s; }   // radians per second

private:
    AnimSpriteComponent* asc = nullptr;

    Vector2 mCenter;             // captured on first update
    bool    mCenterSet = false;
    float   mAngle = 0.f;
    float   mRadius = 20.f;      // pixels — small "hopping" range
    float   mAngularSpeed = 0.6f; // radians per second

    // Last X position for flip detection
    float mLastX = 0.f;
};

#endif
