#include "StudentActor.h"
#include "StudentStates.h"
#include "Game.h"
#include "Renderer.h"
#include "SpriteComponent.h"
#include "AnimSpriteComponent.h"
#include "Texture.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <sstream>
#include <cmath>

static const char* QTABLE_FILE = "qtable_student.csv";

StudentActor::StudentActor(Game* game) : Actor(game)
{
    // ── Animated sprite — replaces the old plain SpriteComponent
    // Only ONE sprite component on this actor. AnimSpriteComponent IS a
    // SpriteComponent so it slots in correctly.
    asc = new AnimSpriteComponent(this, 133);

    // Idle frames
    std::vector<Texture*> idleTexture = {
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
    // Walk frames
    std::vector<Texture*> walkTextures = {
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run01.png"),
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run02.png"),
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run03.png"),
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run04.png"),
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run05.png"),
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run06.png"),
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run07.png"),
        game->GetRenderer()->GetTexture("Assets/Idle, run, jump/run08.png")
    };

    // Register both animations. First call auto-plays "Idle" thanks to the
    // SetAnimTextures auto-play logic.
    asc->SetAnimTextures("Idle", idleTexture,  true);
    asc->SetAnimTextures("Walk", walkTextures, true);

    // Speed up the animation a bit so walking looks natural at 250 px/sec
    asc->SetAnimFPS(24.0f);

    BuildStates();

    bool loaded = mQLearner.LoadCSV(QTABLE_FILE);
    if (loaded) AddLog("Q-table loaded - continuing previous learning.");
    else        AddLog("No prior Q-table - starting fresh.");

    SetPosition(mHomePos);

    AddLog("==========================================");
    AddLog("===   NEW EPISODE 0 STARTING           ===");
    AddLog("==========================================");

    TransitionTo("AtHome");
}

StudentActor::~StudentActor()
{
    mQLearner.SaveCSV(QTABLE_FILE);
    if (mCurrentState) mCurrentState->Exit(this);
    for (auto& kv : mStates) delete kv.second;
}

void StudentActor::BuildStates()
{
    mStates["AtHome"]       = new StateAtHome();
    mStates["AtUniversity"] = new StateAtUniversity();
    mStates["AtLibrary"]    = new StateAtLibrary();
    mStates["AtShop"]       = new StateAtShop();
    mStates["AtPark"]       = new StateAtPark();
}

void StudentActor::TransitionTo(const char* stateName)
{

    std::string name(stateName);
    auto it = mStates.find(name);
    if (it == mStates.end())
    {
        SDL_Log("StudentActor: unknown state '%s'", name.c_str());
        return;
    }

    if (mCurrentState) mCurrentState->Exit(this);

           //ALWAYS unhide and stop roaming when leaving any state
       if (asc) asc->SetVisible(true);
       mRoamingInPark = false;

    Vector2 goal = GetPosForState(name.c_str());
    TileMapComponent* tileMap = GetGame()->GetTileMap();
    if (GetGame()->IsTrainingMode())
{
    // Skip A* — just teleport
    SetPosition(goal);
    mWalking = false;
    mCurrentState = mStates[name];
    mPendingState = "";
    mCurrentState->Enter(this);
    return;
}

    if (tileMap)
    {
        mWaypoints = mPathFinder.FindPath(GetPosition(), goal, tileMap);

        if (mWaypoints.empty())
        {
            SDL_Log("StudentActor: no A* path found, teleporting");
            SetPosition(goal);
            mWalking      = false;
            mCurrentState = mStates[name];
            mPendingState = "";
            mCurrentState->Enter(this);

            // Standing still now — play Idle animation
            if (asc) asc->SetAnimation("Idle");
            return;
        }

        mCurrentWP    = 0;
        mWalking      = true;
        mPendingState = name;
        mCurrentState = nullptr;

        // Starting to walk — play Walk animation
        if (asc) asc->SetAnimation("Walk");


        AddLog(std::string("Student: A* path to ") + name
               + " (" + std::to_string(mWaypoints.size()) + " steps)");
        return;
    }

    // No tile map — direct teleport
    SetPosition(goal);
    mWalking      = false;
    mCurrentState = mStates[name];
    mPendingState = "";
    mCurrentState->Enter(this);
    if (asc) asc->SetAnimation("Idle");
}

void StudentActor::UpdateWalking(float dt)
{
    if (mWaypoints.empty() || mCurrentWP >= static_cast<int>(mWaypoints.size()))
    {
        mWalking = false;
        if (!mPendingState.empty())
        {
            // Save destination name BEFORE clearing mPendingState
            std::string arrived = mPendingState;
            mPendingState = "";

            // Set the FSM state
            mCurrentState = mStates[arrived];
            mCurrentState->Enter(this);

            // Switch to Idle animation
            if (asc) asc->SetAnimation("Idle");

            // Hide sprite if inside a building (not Park)
            if (arrived == "AtHome"     || arrived == "AtUniversity" ||
                arrived == "AtLibrary"  || arrived == "AtShop")
            {
                if (asc) asc->SetVisible(false);
                AddLog(std::string("Student: Entered ") + arrived + " (out of view)");
            }
            else if (arrived == "AtPark")
            {
                // Start park roaming
                mRoamingInPark = true;
                mRoamCenter    = GetPosition();
                mRoamAngle     = 0.f;
                mRoamTimer     = 0.f;
                if (asc) asc->SetAnimation("Walk");
                AddLog("Student: Strolling around the park.");
            }
        }
        return;
    }

    // Normal waypoint walking
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

       // Flip sprite to face the direction of horizontal movement
   if (asc)
   {
       if (dir.x >  0.05f) asc->SetFlipX(false);  // facing right (default)
       else if (dir.x < -0.05f) asc->SetFlipX(true);   // facing left
       // If dir.x is near zero (vertical movement), keep current facing
   }

    float step = mWalkSpeed * dt;
    if (step > dist) step = dist;
    SetPosition(pos + dir * step);
}

void StudentActor::UpdateActor(float dt)
{
    if (mFlashTimer > 0.f)
    {
        mFlashTimer -= dt;
        if (mFlashTimer < 0.f) mFlashTimer = 0.f;
    }

    if (mPauseTimer > 0.f)
    {
        mPauseTimer -= dt;
        if (mPauseTimer <= 0.f)
        {
            mPauseTimer = 0.f;
            ResetStats();
            std::ostringstream banner;
            banner << "===   NEW EPISODE " << mEpisode << " STARTING           ===";
            AddLog("==========================================");
            AddLog(banner.str());
            AddLog("==========================================");
        }
        return;
    }

    mEpTicks++;

    if (mWalking)
    {
        UpdateWalking(dt);
        return;
    }

       //  Park roam: walk in a slow circle around mRoamCenter
   if (mRoamingInPark)
   {
       mRoamAngle += ROAM_SPEED * dt;
       Vector2 newPos = mRoamCenter + Vector2(
           std::cos(mRoamAngle) * ROAM_RADIUS,
           std::sin(mRoamAngle) * ROAM_RADIUS);
       SetPosition(newPos);
   }

    if (mCurrentState)
        mCurrentState->Execute(this, dt);

    CheckEpisodeEnd();
}

void StudentActor::CheckEpisodeEnd()
{
    using namespace StudentThreshold;

    bool collapsed = (mEnergy <= 0);
    bool starving  = (mHunger >= MAX_HUNGER);
    bool breakdown = (mStress >= MAX_STRESS);
    bool goalMet   = (mProgress >= MAX_PROGRESS);
    bool timesUp   = (mEpTicks  >= MAX_EP_TICKS);

    if (!collapsed && !starving && !breakdown && !goalMet && !timesUp)
        return;

    EpisodeOutcome outcome;
    if      (collapsed) outcome = EpisodeOutcome::LostCollapse;
    else if (starving)  outcome = EpisodeOutcome::LostStarving;
    else if (breakdown) outcome = EpisodeOutcome::LostBreakdown;
    else if (goalMet)   outcome = EpisodeOutcome::Won;
    else                outcome = EpisodeOutcome::TimeUp;

    EndEpisode(outcome);
}

void StudentActor::EndEpisode(EpisodeOutcome outcome)
{
    bool lost = (outcome == EpisodeOutcome::LostCollapse ||
                 outcome == EpisodeOutcome::LostStarving ||
                 outcome == EpisodeOutcome::LostBreakdown);

    if (lost) mEpReward -= 50.0f;

    std::ostringstream banner;
    banner << "=== EP " << mEpisode << " " << OutcomeName(outcome)
           << "  Reward: " << mEpReward
           << "  Util: " << GetCompositeUtility() << " ===";
    AddLog("==========================================");
    AddLog(banner.str());
    AddLog("==========================================");

    mLastOutcome = outcome;
    mLastReward  = mEpReward;
    mLastEpisode = mEpisode;

    mFlashTimer   = FLASH_DURATION;
    mFlashOutcome = outcome;
    // Save less frequently during training
    if (!GetGame()->IsTrainingMode() || (mEpisode % 1000 == 0))
        mQLearner.SaveCSV(QTABLE_FILE);

    mEpisode++;
    mPauseTimer = PAUSE_DURATION;
}

void StudentActor::ResetStats()
{
    mEnergy   = StudentThreshold::MAX_ENERGY;
    mHunger   = 0;
    mStress   = 0;
    mMoney    = 3;
    mProgress = 0;
    mEpReward = 0.f;
    mEpTicks  = 0;
    SetPosition(mHomePos);
    mWaypoints.clear();
    mCurrentWP = 0;
    mWalking = false;
    if (asc) asc->SetAnimation("Idle");
    TransitionTo("AtHome");
}

void StudentActor::AddEnergy  (int v){ mEnergy   = std::max(0, std::min(mEnergy   + v, StudentThreshold::MAX_ENERGY));   }
void StudentActor::AddHunger  (int v){ mHunger   = std::max(0, std::min(mHunger   + v, StudentThreshold::MAX_HUNGER));   }
void StudentActor::AddStress  (int v){ mStress   = std::max(0, std::min(mStress   + v, StudentThreshold::MAX_STRESS));   }
void StudentActor::AddMoney   (int v){ mMoney    = std::max(0, std::min(mMoney    + v, StudentThreshold::MAX_MONEY));    }
void StudentActor::AddProgress(int v){ mProgress = std::max(0, std::min(mProgress + v, StudentThreshold::MAX_PROGRESS)); }

StudentQState StudentActor::BuildQState() const
{
    return { mEnergy, mHunger, mStress, mMoney, mProgress,
             static_cast<int>(TickToTimeOfDay(mEpTicks)) };
}

float StudentActor::GetCompositeUtility() const
{
    return 0.20f * GetUEnergy()   +
           0.15f * GetUHunger()   +
           0.15f * GetUStress()   +
           0.10f * GetUMoney()    +
           0.40f * GetUProgress();
}

Vector2 StudentActor::GetPosForState(const char* name) const
{
    std::string n(name);
    if (n == "AtUniversity") return mUniversityPos;
    if (n == "AtLibrary")    return mLibraryPos;
    if (n == "AtHome")       return mHomePos;
    if (n == "AtShop")       return mShopPos;
    if (n == "AtPark")       return mParkPos;
    return mHomePos;
}

const char* StudentActor::GetFSMStateName() const
{
    if (mPauseTimer > 0.f) return "[Reset]";
    if (mWalking && !mPendingState.empty()) return mPendingState.c_str();
    return mCurrentState ? mCurrentState->GetName() : "Walking";
}

void StudentActor::AddLog(const std::string& msg)
{
    mLog.push_back(msg);
    if (static_cast<int>(mLog.size()) > MAX_LOG) mLog.pop_front();
    if (!GetGame()->IsTrainingMode())
    SDL_Log("%s", msg.c_str());
}

   void StudentActor::StartFreshEpisode()
   {
       // Save the trained Q-table to disk (just in case)
       mQLearner.SaveCSV(QTABLE_FILE);

       // Reset all stats and counters for a clean episode
       mEnergy   = StudentThreshold::MAX_ENERGY;
       mHunger   = 0;
       mStress   = 0;
       mMoney    = 3;
       mProgress = 0;
       mEpReward = 0.f;
       mEpTicks  = 0;

       // Clear walking state
       mWaypoints.clear();
       mCurrentWP = 0;
       mWalking = false;
       mPendingState = "";

       // Clear roaming
       mRoamingInPark = false;

       // Clear pause/flash
       mPauseTimer = 0.f;
       mFlashTimer = 0.f;

    //NEW: clear the "last episode" history so the HUD top line is clean
    mLastOutcome  = EpisodeOutcome::None;   // or whatever your "neutral" value is
    mLastReward   = 0.f;
    mLastEpisode  = 0;
    mFlashOutcome = EpisodeOutcome::None;

       // Make sprite visible
       if (asc)
       {
           asc->SetVisible(true);
           asc->SetAnimation("Idle");
       }

       // Place at home, start fresh
       SetPosition(mHomePos);

       AddLog("==========================================");
       AddLog(std::string("===   RESUMING WITH TRAINED AGENT      ===").c_str());
       AddLog(std::string("===   Episode " + std::to_string(mEpisode) +
                           "   eps=" +
                           std::to_string(mQLearner.GetEpsilon()).substr(0, 5) +
                           "   ===").c_str());
       AddLog("==========================================");

       TransitionTo("AtHome");
   }
