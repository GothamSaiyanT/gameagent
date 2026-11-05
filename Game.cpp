#include "Game.h"
#include <algorithm>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Actor.h"
#include "SpriteComponent.h"
#include "Ship.h"
#include "Shader.h"
#include "NPCActor.h"
#include "PigeonActor.h"
#include "BGSpriteComponent.h"
#include "VertexArray.h"
#include "Texture.h"
#include "TileMapComponent.h"
#include "TiledMapLoader.h"
#include "CSVConverter.h"
#include <vector>
#include "CameraActor.h"
#include "Renderer.h"
#include "StudentActor.h"
#include "StudentQState.h"
#include "BirdActor.h"

// World coordinates of the 5 locations (cluster centres on the tile map)
static const Vector2 WORLD_HOME       {  784.0f,  432.0f };
static const Vector2 WORLD_PARK       { -176.0f,  240.0f };
static const Vector2 WORLD_UNIVERSITY {  432.0f, -176.0f };
static const Vector2 WORLD_LIBRARY    {  784.0f, -176.0f };
static const Vector2 WORLD_SHOP       { -816.0f, -240.0f };

Game::Game()
    : mRenderer(nullptr)
    , mIsRunning(true)
    , mUpdatingActors(false)
    , mStudent(nullptr)
{
}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(this);
    if (!mRenderer->Initialize(1024.0f, 768.0f))
    {
        SDL_Log("Failed to initialize renderer");
        delete mRenderer;
        mRenderer = nullptr;
        return false;
    }

    // HUD initialisation (was previously commented out)
    if (!mHUD.Initialize(mRenderer->GetWindow(),
                          static_cast<int>(mRenderer->GetScreenWidth()),
                          static_cast<int>(mRenderer->GetScreenHeight())))
    {
        SDL_Log("HUD init failed - console log will still work");
    }
    else
    {
        SDL_Log("HUD initialised successfully");
    }

    mInputSystem = new InputSystem();
    mInputSystem->Initialize();

    LoadData();
    mTicksCount = SDL_GetTicks();
    return true;
}
   void Game::RunLoop()
   {
       int trainCounter = 0;
       int lastReportedEp = 0;

       while (mIsRunning)
       {
           ProcessInput();

           if (mTrainingMode)
           {
               // FAST TRAINING: many simulation ticks per real frame
               // No rendering, no frame cap. Use a fixed dt so physics
               // are deterministic, and run TICKS_PER_FRAME steps before
               // checking input again.
               const int  TICKS_PER_FRAME = 10000;
               const float FIXED_DT       = 0.016f;

               for (int i = 0; i < TICKS_PER_FRAME; ++i)
               {
                   mUpdatingActors = true;
                   for (auto actor : mActors) actor->Update(FIXED_DT);
                   mUpdatingActors = false;

                   for (auto pending : mPendingActors)
                   {
                       pending->ComputeWorldTransform();
                       mActors.emplace_back(pending);
                   }
                   mPendingActors.clear();
               }
               mTotalTicks += TICKS_PER_FRAME;
               trainCounter += TICKS_PER_FRAME;

               // Report progress to console every 50,000 ticks
               if (trainCounter >= 50000 && mStudent)
               {
                   trainCounter = 0;
                   int currentEp = mStudent->GetEpisode();
                   int delta     = currentEp - lastReportedEp;
                   lastReportedEp = currentEp;
                   SDL_Log("[TRAINING] Episode %d (+%d), epsilon=%.3f",
                           currentEp, delta, mStudent->GetEpsilon());
               }
           }
           else
           {
               //NORMAL PLAY: standard frame loop
               UpdateGame();
               GenerateOutput();
           }
       }
   }

void Game::ShutDown()
{
    UnloadData();
    for (auto layer : mTileMap) delete layer;
    mTileMap.clear();

    mHUD.Shutdown();

    if (mRenderer) mRenderer->Shutdown();
    mInputSystem->ShutDown();
    delete mInputSystem;
    SDL_Quit();
}


void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT: mIsRunning = false; break;
        }
    }
    const InputState& state = mInputSystem->GetState();
    if (state.Keyboard.GetKeyValue(SDL_SCANCODE_ESCAPE)) mIsRunning = false;

    float zoom = mRenderer->GetZoom();
    if (state.Keyboard.GetKeyValue(SDL_SCANCODE_EQUALS)) zoom += 0.005f;
    if (state.Keyboard.GetKeyValue(SDL_SCANCODE_MINUS))  zoom -= 0.005f;
    if (zoom < 0.1f) zoom = 0.1f;
    if (zoom > 2.0f) zoom = 2.0f;
    mRenderer->SetZoom(zoom);
    mRenderer->UpdateProjection();

    static bool prevTPressed = false;
   bool tPressed = state.Keyboard.GetKeyValue(SDL_SCANCODE_T);
   if (tPressed && !prevTPressed)
   {
       mTrainingMode = !mTrainingMode;
       if (mTrainingMode)
       {
           mTrainingStartEp = mStudent ? mStudent->GetEpisode() : 0;
           mHUDEpisode = 0;
           SDL_Log("=== TRAINING MODE ON ===");
           SDL_Log("    Start episode: %d", mTrainingStartEp);
           SDL_Log("    Target episode: %d", mTargetEpisode);
           SDL_Log("    Will auto-resume normal play when reached.");
       }
        else
        {
            SDL_Log("=== TRAINING MODE OFF (manual stop) ===");
            if (mStudent)
            {
                mStudent->StartFreshEpisode();
                mEpisodeDisplayOffset = mStudent->GetEpisode();
            }
        }
   }
   prevTPressed = tPressed;

    mUpdatingActors = true;
    for (auto actor : mActors) actor->ProcessInput(state);
    mUpdatingActors = false;
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));
    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f) deltaTime = 0.05f;
    mTicksCount = SDL_GetTicks();
    mTotalTicks++;

    mUpdatingActors = true;
    for (auto actor : mActors) actor->Update(deltaTime);
    mUpdatingActors = false;

    for (auto pending : mPendingActors)
    {
        pending->ComputeWorldTransform();
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();

    std::vector<Actor*> deadActors;
    for (auto actor : mActors)
        if (actor->GetState() == Actor::EDead)
            deadActors.emplace_back(actor);
    for (auto actor : deadActors) delete actor;

    // Debug log once per second
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer >= 1.0f && mStudent)
    {
        debugTimer = 0.0f;
        SDL_Log("[Student] Ep=%d  Loc=%-13s  E=%d H=%d S=%d M=%d P=%d  "
                "%s  U=%.2f  eps=%.3f",
                mStudent->GetEpisode(),
                mStudent->GetFSMStateName(),
                mStudent->GetEnergy(),
                mStudent->GetHunger(),
                mStudent->GetStress(),
                mStudent->GetMoney(),
                mStudent->GetProgress(),
                TimeOfDayName(mStudent->GetTimeOfDay()),
                mStudent->GetCompositeUtility(),
                mStudent->GetEpsilon());
    }

    // Camera follow student
    if (mStudent)
    {
        mRenderer->SetCameraPos(mStudent->GetPosition());
        mRenderer->UpdateProjection();
    }
}

void Game::GenerateOutput()
{
    //  Fill HUD data from the student
    if (mStudent)
    {
            // HUD shows episodes since training ended; learning continues underneath
        mHUDData.episode = mStudent->GetEpisode() - mEpisodeDisplayOffset ;
       // mHUDData.episode  = mStudent->GetEpisode() + 1;
        mHUDData.totalTicks = mTotalTicks;
        mHUDData.epTicks    = mStudent->GetEpTicks();
        mHUDData.epsilon    = mStudent->GetEpsilon();
        mHUDData.reward     = mStudent->GetEpReward();
        mHUDData.utility    = mStudent->GetCompositeUtility();
        mHUDData.location   = mStudent->GetFSMStateName();
        mHUDData.timeOfDay  = TimeOfDayName(mStudent->GetTimeOfDay());

        // Stats
        mHUDData.energy     = mStudent->GetEnergy();
        mHUDData.hunger     = mStudent->GetHunger();
        mHUDData.stress     = mStudent->GetStress();
        mHUDData.money      = mStudent->GetMoney();
        mHUDData.progress   = mStudent->GetProgress();

        // Max values
        mHUDData.maxEnergy   = StudentThreshold::MAX_ENERGY;
        mHUDData.maxHunger   = StudentThreshold::MAX_HUNGER;
        mHUDData.maxStress   = StudentThreshold::MAX_STRESS;
        mHUDData.maxMoney    = StudentThreshold::MAX_MONEY;
        mHUDData.maxProgress = StudentThreshold::MAX_PROGRESS;

        if (mStudent->GetLastEpisode() <= mEpisodeDisplayOffset)
        {
        // No "real" demo episode finished yet —> show blank
        mHUDData.lastOutcome = "—";
        mHUDData.lastReward  = 0.f;
        mHUDData.lastEpisode = 0;
        }
        else
        {
        mHUDData.lastEpisode = mStudent->GetLastEpisode() - mEpisodeDisplayOffset;
        mHUDData.lastReward  = mStudent->GetLastReward();
        mHUDData.lastOutcome = OutcomeName(mStudent->GetLastOutcome());
}

        // Episode result for HUD "Last Ep" line and flash colour
        mHUDData.lastEpisode = mStudent->GetLastEpisode();
        mHUDData.lastReward  = mStudent->GetLastReward();
        mHUDData.lastOutcome = OutcomeName(mStudent->GetLastOutcome());

        // Screen flash overlay (fades from 1.0 to 0.0)
        mHUDData.flashAlpha    = mStudent->GetFlashAlpha();
        mHUDData.flashOutcome  = OutcomeName(mStudent->GetFlashOutcome());

        // Inter-episode pause flag
        mHUDData.betweenEpisodes = mStudent->IsBetweenEpisodes();

        // Copy dialogue log
        mHUDData.log.clear();
        for (const auto& msg : mStudent->GetLog())
            mHUDData.log.push_back(msg);
    }

    // 1. Draw the 3D world
    mRenderer->Draw();

    // 2. Draw the HUD overlay on top of the world
    if (mHUD.IsReady())
        mHUD.Draw(mHUDData);

    // 3. Swap window — must be LAST
    SDL_GL_SwapWindow(mRenderer->GetWindow());
}

void Game::AddActor(Actor* actor)
{
    if (mUpdatingActors) mPendingActors.emplace_back(actor);
    else                 mActors.emplace_back(actor);
}

void Game::RemoveActor(Actor* actor)
{
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {
        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }
    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::UnloadData()
{
    while (!mActors.empty()) delete mActors.back();
    if (mRenderer) mRenderer->UnloadData();
}

void Game::LoadData()
{
    Texture* tilesTexture = mRenderer->GetTexture(
        "Assets/map/Slates v.2 [32x32px orthogonal tileset by Ivan Voirol].png");
    if (!tilesTexture) { SDL_Log("Failed to load tiles texture"); return; }

    Actor* mapActor = new Actor(this);
    mapActor->SetPosition(Vector2(0.0f, 0.0f));

    auto AddLayer = [&](int order, const char* csv) {
        TileMapComponent* layer = new TileMapComponent(mapActor, order);
        layer->SetTexture(tilesTexture);
        layer->SetTileSize(32, 32);
        if (!layer->LoadFromCSV(csv))
            SDL_Log("Failed to load layer: %s", csv);
        return layer;
    };
    TileMapComponent* mapCollisions = new TileMapComponent(mapActor, 0);
mapCollisions->SetTexture(tilesTexture);
mapCollisions->SetTileSize(32, 32);
if (!mapCollisions->LoadFromCSV("Assets/map/tilesetmapdemo_collisions.csv"))
    SDL_Log("Failed to load collisions layer");

    mTileMap.push_back(AddLayer( 10, "Assets/map/tilesetmapdemo_Water.csv"));
    mTileMap.push_back(AddLayer( 20, "Assets/map/tilesetmapdemo_Ground.csv"));
    mTileMap.push_back(AddLayer( 30, "Assets/map/tilesetmapdemo_Grass.csv"));
    mTileMap.push_back(AddLayer( 40, "Assets/map/tilesetmapdemo_path.csv"));
    mTileMap.push_back(AddLayer( 50, "Assets/map/tilesetmapdemo_Level.csv"));
    mTileMap.push_back(AddLayer( 60, "Assets/map/tilesetmapdemo_park.csv"));
    mTileMap.push_back(AddLayer( 70, "Assets/map/tilesetmapdemo_shipandstuff.csv"));
    mTileMap.push_back(AddLayer( 80, "Assets/map/tilesetmapdemo_shipstuff.csv"));
    mTileMap.push_back(AddLayer( 90, "Assets/map/tilesetmapdemo_out.csv"));
    mTileMap.push_back(AddLayer(100, "Assets/map/tilesetmapdemo_stuff.csv"));
    mTileMap.push_back(AddLayer(110, "Assets/map/tilesetmapdemo_treesdown.csv"));
    mTileMap.push_back(AddLayer(120, "Assets/map/tilesetmapdemo_otherstuffs.csv"));
    mTileMap.push_back(AddLayer(130, "Assets/map/tilesetmapdemo_theme.csv"));
    mTileMap.push_back(AddLayer(140, "Assets/map/tilesetmapdemo_treesup.csv"));
    mTileMap.push_back(mapCollisions);

    mStudent = new StudentActor(this);
    mStudent->SetScale(1.0f);
    mStudent->SetUniversityPos(WORLD_UNIVERSITY);
    mStudent->SetLibraryPos   (WORLD_LIBRARY);
    mStudent->SetHomePos      (WORLD_HOME);
    mStudent->SetShopPos      (WORLD_SHOP);
    mStudent->SetParkPos      (WORLD_PARK);

    SDL_Log("Student Agent ready.");

     auto LoadTex = [&](const char* path) -> Texture* {
       return mRenderer->GetTexture(path);
   };

   // ── NPC 1: spawns near the centre
   NPCActor* npc1 = new NPCActor(this, "Alice");
   npc1->SetPosition(Vector2(0.0f, 0.0f));
   npc1->SetScale(1.0f);
   npc1->SetRoamSpeed(120.f);
   npc1->SetRoamRadius(6);
   npc1->SetWaitTime(1.5f, 3.5f);
   // Replace these paths with your sprite frames for this NPC
   std::vector<Texture*> npc1Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc1Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc1->AddIdleFrames(npc1Idle);
   npc1->AddWalkFrames(npc1Walk);

   // ── NPC 2: spawns near the university
   NPCActor* npc2 = new NPCActor(this, "Bob");
   npc2->SetPosition(Vector2(400.0f, -100.0f));
   npc2->SetScale(1.0f);
   npc2->SetRoamSpeed(100.f);
   std::vector<Texture*> npc2Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc2Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc2->AddIdleFrames(npc2Idle);
   npc2->AddWalkFrames(npc2Walk);

   // ── NPC 3: spawns near home
   NPCActor* npc3 = new NPCActor(this, "Charlie");
   npc3->SetPosition(Vector2(-400.0f, 200.0f));
   npc3->SetScale(1.0f);
   npc3->SetRoamSpeed(140.f);
   std::vector<Texture*> npc3Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc3Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc3->AddIdleFrames(npc3Idle);
   npc3->AddWalkFrames(npc3Walk);

   SDL_Log("3 NPCs spawned and roaming.");

      // ── NPC 3: spawns near home
   NPCActor* npc4 = new NPCActor(this, "Charlie");
   npc4->SetPosition(Vector2(-480.0f, 260.0f));
   npc4->SetScale(1.0f);
   npc4->SetRoamSpeed(140.f);
   std::vector<Texture*> npc4Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc4Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc4->AddIdleFrames(npc4Idle);
   npc4->AddWalkFrames(npc4Walk);

   SDL_Log("4 NPCs spawned and roaming.");

   NPCActor* npc5 = new NPCActor(this, "Charlie");
   npc5->SetPosition(Vector2(-380.0f, 260.0f));
   npc5->SetScale(1.0f);
   npc5->SetRoamSpeed(140.f);
   std::vector<Texture*> npc5Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc5Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc5->AddIdleFrames(npc5Idle);
   npc5->AddWalkFrames(npc5Walk);

   SDL_Log("5 NPCs spawned and roaming.");

   NPCActor* npc6 = new NPCActor(this, "Charlie");
   npc6->SetPosition(Vector2(-200.0f, 200.0f));
   npc6->SetScale(1.0f);
   npc6->SetRoamSpeed(140.f);
   std::vector<Texture*> npc6Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc6Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc6->AddIdleFrames(npc6Idle);
   npc6->AddWalkFrames(npc6Walk);

   SDL_Log("6 NPCs spawned and roaming.");

   NPCActor* npc7 = new NPCActor(this, "Charlie");
   npc7->SetPosition(Vector2(-150.0f, 180.0f));
   npc7->SetScale(1.0f);
   npc7->SetRoamSpeed(140.f);
   std::vector<Texture*> npc7Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc7Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc7->AddIdleFrames(npc7Idle);
   npc7->AddWalkFrames(npc7Walk);

   SDL_Log("7 NPCs spawned and roaming.");

   NPCActor* npc8 = new NPCActor(this, "Charlie");
   npc8->SetPosition(Vector2(-150.0f, 300.0f));
   npc8->SetScale(1.0f);
   npc8->SetRoamSpeed(140.f);
   std::vector<Texture*> npc8Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc8Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc8->AddIdleFrames(npc8Idle);
   npc8->AddWalkFrames(npc8Walk);

   SDL_Log("8 NPCs spawned and roaming.");

   NPCActor* npc9 = new NPCActor(this, "Charlie");
   npc9->SetPosition(Vector2(380.0f, 260.0f));
   npc9->SetScale(1.0f);
   npc9->SetRoamSpeed(140.f);
   std::vector<Texture*> npc9Idle = {
       LoadTex("Assets/Idle, run, jump/idle01.png"),
       LoadTex("Assets/Idle, run, jump/idle02.png")
   };
   std::vector<Texture*> npc9Walk = {
       LoadTex("Assets/Idle, run, jump/run01.png"),
       LoadTex("Assets/Idle, run, jump/run02.png"),
       LoadTex("Assets/Idle, run, jump/run03.png")
   };
   npc9->AddIdleFrames(npc9Idle);
   npc9->AddWalkFrames(npc9Walk);

   SDL_Log("9 NPCs spawned and roaming.");

std::vector<Texture*> birdFrames = {
    mRenderer->GetTexture("Assets/bird/parrot01.png"),
    mRenderer->GetTexture("Assets/bird/parrot02.png"),
    mRenderer->GetTexture("Assets/bird/parrot03.png"),
    mRenderer->GetTexture("Assets/bird/parrot04.png"),
    mRenderer->GetTexture("Assets/bird/parrot05.png"),
    mRenderer->GetTexture("Assets/bird/parrot06.png"),
    mRenderer->GetTexture("Assets/bird/parrot07.png"),
    mRenderer->GetTexture("Assets/bird/parrot08.png"),
    mRenderer->GetTexture("Assets/bird/parrot09.png"),
    mRenderer->GetTexture("Assets/bird/parrot10.png")
};

std::vector<Texture*> birdFrames1 = {
    mRenderer->GetTexture("Assets/bird/bluebird01.png"),
    mRenderer->GetTexture("Assets/bird/bluebird02.png"),
    mRenderer->GetTexture("Assets/bird/bluebird03.png"),
    mRenderer->GetTexture("Assets/bird/bluebird04.png"),
    mRenderer->GetTexture("Assets/bird/bluebird05.png"),
    mRenderer->GetTexture("Assets/bird/bluebird06.png")
};

      // Bird 1 — slow, flies right across the top of the map
   BirdActor* bird1 = new BirdActor(this);
   bird1->SetFrames(birdFrames);
   bird1->SetSpeed(70.f);           // pixels/sec horizontal
   bird1->SetFlightLevel(224.f);    // high in the sky
   bird1->SetMapExtents(-1100.f, 1100.f);
   bird1->SetPosition(Vector2(-1100.f, 550.f));
   bird1->SetScale(0.15f);

   // Bird 2 — slightly faster, mid-altitude
   BirdActor* bird2 = new BirdActor(this);
   bird2->SetFrames(birdFrames);
   bird2->SetSpeed(95.f);
   bird2->SetFlightLevel(400.f);
   bird2->SetMapExtents(-1100.f, 1100.f);
   bird2->SetPosition(Vector2(-300.f, 400.f));
   bird2->SetScale(0.12f);

   // Bird 3 — flies LEFT (negative speed) across the bottom of the sky
   BirdActor* bird3 = new BirdActor(this);
   bird3->SetFrames(birdFrames);
   bird3->SetSpeed(-80.f);          // negative = flying left
   bird3->SetFlightLevel(-176.f);
   bird3->SetMapExtents(-1100.f, 1100.f);
   bird3->SetPosition(Vector2(-1100.f, -176.f));
   bird3->SetScale(0.13f);

   SDL_Log("3 birds flying.");

      std::vector<Texture*> pigeonFrames = {
       mRenderer->GetTexture("Assets/pigeon/pigeon_walk01.png"),
       mRenderer->GetTexture("Assets/pigeon/pigeon_walk02.png"),
       mRenderer->GetTexture("Assets/pigeon/pigeon_walk03.png"),
       mRenderer->GetTexture("Assets/pigeon/pigeon_walk04.png")
   };

   BirdActor* bird4 = new BirdActor(this);
bird4->SetFrames(birdFrames1);
bird4->SetSpeed(70.f);                              // right
bird4->SetFlightLevel(550.f);                       // above Home
bird4->SetMapExtents(-1100.f, 1100.f);
bird4->SetPosition(Vector2(-1100.f, 550.f));        // starts on LEFT edge
bird4->SetScale(0.6f);

// Bird 2 — flies LEFT over the Park area (middle of map)
BirdActor* bird5 = new BirdActor(this);
bird5->SetFrames(birdFrames);
bird5->SetSpeed(-90.f);                             // left
bird5->SetFlightLevel(240.f);                       // park height
bird5->SetMapExtents(-1100.f, 1100.f);
bird5->SetPosition(Vector2(1100.f, 240.f));         // starts on RIGHT edge
bird5->SetScale(0.5f);

// Bird 3 — flies RIGHT over the Library (bottom-right area)
BirdActor* bird6 = new BirdActor(this);
bird6->SetFrames(birdFrames1);
bird6->SetSpeed(85.f);                              // right
bird6->SetFlightLevel(-176.f);                      // library height
bird6->SetMapExtents(-1100.f, 1100.f);
bird6->SetPosition(Vector2(-1100.f, -176.f));       // starts on LEFT edge
bird6->SetScale(0.55f);

// Bird 4 — flies LEFT over the Shop area (bottom-left)
BirdActor* bird7 = new BirdActor(this);
bird7->SetFrames(birdFrames1);
bird7->SetSpeed(-75.f);                             // left
bird7->SetFlightLevel(-400.f);                      // below shop
bird7->SetMapExtents(-1100.f, 1100.f);
bird7->SetPosition(Vector2(1100.f, -400.f));        // starts on RIGHT edge
bird7->SetScale(0.6f);

// Bird 5 — fast flyer, near top of the sky
BirdActor* bird8 = new BirdActor(this);
bird8->SetFrames(birdFrames1);
bird8->SetSpeed(120.f);                             // fast right
bird8->SetFlightLevel(650.f);                       // very high                       // faster bob
bird8->SetMapExtents(-1100.f, 1100.f);
bird8->SetPosition(Vector2(-1100.f, 650.f));
bird8->SetScale(0.45f);                             // smaller = looks farther

SDL_Log("5 bluebirds flying across the map.");

      // Spawn pigeons around the park (park is at world -176, 240)
   // Spread them out so they're not all stacked on the same tile
   struct PigeonSpawn { float x, y, scale; };
   PigeonSpawn pigeonSpots[] = {
       { -200.f,  280.f, 1.0f },   // top-left of park
       {  -90.f,  290.f, 0.9f },   // top-right
       { -150.f,  220.f, 1.1f },   // middle
       { -240.f,  190.f, 1.0f },   // bottom-left
       {  -50.f,  200.f, 0.95f },  // bottom-right
       { -120.f,  160.f, 1.05f },  // far bottom
       { -290.f,  260.f, 0.9f },   // far left
       {  -10.f,  250.f, 1.0f },    // far right

        //  University (around 432, -176)
    {  380.f, -140.f, 1.0f  },
    {  470.f, -120.f, 0.95f },
    {  420.f, -220.f, 1.05f },
    {  350.f, -200.f, 0.9f  },

    //  Library (around 784, -176)
    {  730.f, -140.f, 1.0f  },
    {  830.f, -130.f, 0.95f },
    {  780.f, -220.f, 1.05f },

    //  Shop (around -816, -240)
    { -870.f, -200.f, 1.0f  },
    { -760.f, -210.f, 0.95f },
    { -820.f, -290.f, 1.05f },
    { -880.f, -280.f, 0.9f  },

    //  Home (around 784, 432)
    {  730.f,  470.f, 1.0f  },
    {  830.f,  460.f, 0.95f },
    {  780.f,  390.f, 1.05f },
    {  860.f,  410.f, 0.9f  }
   };

      for (auto& spot : pigeonSpots)
   {
       PigeonActor* pig = new PigeonActor(this);
       pig->SetPosition(Vector2(spot.x, spot.y));
       pig->SetScale(spot.scale);
       pig->SetWalkFrames(pigeonFrames);
   }

   SDL_Log("8 pigeons cooing around the park.");

   // Reuse BirdActor — clouds are essentially "very slow birds" with one

// Helper: create one cloud actor with a single texture
auto MakeCloud = [&](const char* texturePath, float startX, float y,
                     float speed, float scale)
{
    BirdActor* cloud = new BirdActor(this);

    // BirdActor expects a vector of frames — a "single frame" animation
    // is just one texture repeated. Still loops fine.
    Texture* cloudTex = mRenderer->GetTexture(texturePath);
    std::vector<Texture*> cloudFrames = { cloudTex };
    cloud->SetFrames(cloudFrames);

    cloud->SetSpeed(speed);                     // slow drift (10-30 px/s)
    cloud->SetFlightLevel(y);                   // altitude
    cloud->SetMapExtents(-1400.f, 1400.f);      // wrap past the visible map
    cloud->SetPosition(Vector2(startX, y));
    cloud->SetScale(scale);
};

// 4 clouds at different altitudes drifting in different directions
MakeCloud("Assets/clouds/cloud01.png", -1400.f,  600.f,  18.f,  0.40f);
MakeCloud("Assets/clouds/cloud02.png",  1400.f,  450.f, -22.f,  0.35f);
MakeCloud("Assets/clouds/cloud04.png", -1400.f,  250.f,  15.f,  0.30f);
MakeCloud("Assets/clouds/cloud05.png",  1400.f,  700.f, -20.f,  0.45f);

SDL_Log("4 clouds drifting.");

}

TileMapComponent* Game::GetTileMap() const
{
         if (!mTileMap.empty()) return mTileMap.back();   // last = collisions
       return nullptr;
}
