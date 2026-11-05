#ifndef SHIP_H_INCLUDED
#define SHIP_H_INCLUDED

#include "Actor.h"
#include "AnimSpriteComponent.h"
#include "BoxCollisionComponent.h"
#include "TileMapComponent.h"

class Ship:public Actor
{
public:
    Ship(class Game* game);
    ~Ship();
    void UpdateActor(float deltaTime) override;
    void ActorInput(const struct InputState& state) override;
    void ProcessKeyBoard(const struct InputState& state);
    float GetRightSpeed()const{return mRightSpeed;}
    float GetDownSpeed()const{return mDownSpeed;}

private:
    float mRightSpeed;
    float mDownSpeed;
        InputComponent* ic;
            AnimSpriteComponent* asc;
            BoxCollisionComponent* mCollision;
             TileMapComponent* mTileMap;
    float mLaserCooldown;
       float mVerticalSpeed = 0.0f;
    bool mOnGround = false;

};


#endif // SHIP_H_INCLUDED
