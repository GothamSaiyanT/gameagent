#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED
#pragma once
#include "SDL2/SDL.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "Texture.h"
#include "TileMapComponent.h"
#include "InputSystem.h"
#include "HUDManager.h"
#include "StudentActor.h"

class NPCActor;
class Game{
public:
    Game();
    bool Initialize();
    void RunLoop();
    void ShutDown();
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);
    class Renderer* GetRenderer() { return mRenderer; }
    class TileMapComponent* GetTileMap() const;
    bool IsTrainingMode() const { return mTrainingMode; }

       // Call this instead of SDL_Log for Bob's lines — shows on screen
    void AddLog(const std::string& msg) { mHUD.AddLog(msg); }
    class StudentActor* GetStudentActor() const { return mStudent; }


private:
    //helper functions
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void LoadData();
    void UnloadData();


    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;
    class Renderer* mRenderer;

        //const struct InputState& state = SDL_GetKeyboardState(nullptr);
    bool mIsRunning;
    Uint32 mTicksCount;
    bool mUpdatingActors;
    //game specific
    class Ship* mShip;
    Matrix4 mView;

     class StudentActor* mStudent;

      // Add TileMapComponent to the game
    std::vector<TileMapComponent*> mTileMap;

    InputSystem* mInputSystem;
      	//Renderer

      	    // HUD
    HUDManager mHUD;
    HUDData    mHUDData;
    int        mTotalTicks = 0;

    //training
    bool mTrainingMode = false;
   int  mTargetEpisode = 5000;     // auto-exit at this episode
   int  mTrainingStartEp = 0;       // episode when training started
   int mHUDEpisode = 0;
   int mEpisodeDisplayOffset = 0;



};

#endif // GAME_H_INCLUDED
