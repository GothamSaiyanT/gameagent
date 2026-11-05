#include "Ship.h"

#include "Game.h"
#include "InputComponent.h"
#include "BoxCollisionComponent.h"
#include "TileMapComponent.h"


Ship::Ship( Game* game)
:Actor(game)
{
    std::vector<Texture*> idleTexture= {
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle01.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle02.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle03.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle04.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle05.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle06.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle07.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle08.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/idle09.png")
    };
        std::vector<Texture*> walkTextures= {
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run01.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run02.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run03.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run04.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run05.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run06.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run07.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run08.png")
    };
    std::vector<Texture*> jumptexturs= {
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/jump_start01.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/jump_start02.png"),
    game->GetRenderer()->GetTexture("Assets/Idle, run, jump/jump_mid01.png")
    };
    asc = new AnimSpriteComponent(this,1200);
    asc->SetAnimTextures("Idle",idleTexture,true);
    asc->SetAnimTextures("Walk",walkTextures,true);
    asc->SetAnimTextures("Jump",jumptexturs,true);



    //asc->SetAnimation("Ship");

    //create input component and set keys
     ic = new InputComponent(this);
    ic->SetForwardkey(SDL_SCANCODE_D);
    ic->SetBackkey(SDL_SCANCODE_A);
    ic->SetJumpKey(SDL_SCANCODE_SPACE);

    ic->SetMaximuForwardSpeed(150.0f);
    ic->SetMaximuAngularSpeed(Math::TwoPi);

        //  COLLISION SETUP
    mCollision = new BoxCollisionComponent(this);
    mCollision->SetSize(32.0f, 48.0f); // match character size

    // Get tile map from game (main layer)
    mTileMap = game->GetTileMap();

}
Ship::~Ship()
{
    delete asc;
    asc = 0;
    delete ic;
    ic = 0;
}

void Ship::UpdateActor(float deltaTime){
        asc->UpdateAnimation(ic->GetState());
        if (ic->GetForwardSpeed() < 0.0f)
    asc->SetFlipX(true);
else if (ic->GetForwardSpeed() > 0.0f)
    asc->SetFlipX(false);

    //update position based on speeds and delta time
   /* Vector2 pos =  GetPosition();
    pos.x += mRightSpeed * deltaTime;
    pos.y += mDownSpeed * deltaTime;
        // Screen bounds
    float screenWidth = 1024.0f;
    float screenHeight = 768.0f;

float halfWidth = screenWidth * 0.5f;
float halfHeight = screenHeight * 0.5f;

if (pos.x < -halfWidth)
    pos.x = halfWidth;
else if (pos.x > halfWidth)
    pos.x = -halfWidth;

if (pos.y < -halfHeight)
    pos.y = halfHeight;
else if (pos.y > halfHeight)
    pos.y = -halfHeight;

    // Set wrapped position
    SetPosition(pos);*/

    /*Vector2 pos = GetPosition();

    // Define world bounds based on the PlaneActor grid
    float halfWorldWidth = 1024.0f * 0.5f;   // 512.0f
    float halfWorldHeight = 768.0f * 0.5f;   // 384.0f

        // Clamp position to stay within the floor
    pos.x = Math::Clamp(pos.x, -halfWorldWidth, halfWorldWidth);
    pos.y = Math::Clamp(pos.y, -halfWorldHeight, halfWorldHeight);

    // Apply wrapped position
    SetPosition(pos);*/


/*if(mOnGround)
{
ic->Jump(200);
}*/


    mLaserCooldown -= deltaTime;
}
    void Ship::ActorInput(const struct InputState& state){
        ic->ProcessInput(state); // Apply keys to movement
            if (mOnGround &&
        state.Keyboard.GetKeyValue(SDL_SCANCODE_SPACE))
    {
        mVerticalSpeed = 550.0f;
        mOnGround = false;
    }
    }
