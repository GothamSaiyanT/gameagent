#pragma once
#ifndef STUDENTACTOR_H
#define STUDENTACTOR_H

#include "Actor.h"
#include "Math.h"
#include "StudentQState.h"
#include "QLearner.h"
#include "PathFinder.h"
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>

class StudentFSMState;
class AnimSpriteComponent;   // forward declaration

enum class EpisodeOutcome
{
    None,
    Won,
    LostCollapse,
    LostStarving,
    LostBreakdown,
    TimeUp
};

inline const char* OutcomeName(EpisodeOutcome o)
{
    switch (o)
    {
        case EpisodeOutcome::None:          return "None";
        case EpisodeOutcome::Won:           return "WON";
        case EpisodeOutcome::LostCollapse:  return "LOST [COLLAPSED]";
        case EpisodeOutcome::LostStarving:  return "LOST [STARVING]";
        case EpisodeOutcome::LostBreakdown: return "LOST [BREAKDOWN]";
        case EpisodeOutcome::TimeUp:        return "TIME UP";
    }
    return "?";
}

class StudentActor : public Actor
{
public:
    explicit StudentActor(class Game* game);
    ~StudentActor() override;

    void UpdateActor(float deltaTime) override;
    void TransitionTo(const char* stateName);

    void SetUniversityPos(const Vector2& p) { mUniversityPos = p; }
    void SetLibraryPos   (const Vector2& p) { mLibraryPos    = p; }
    void SetHomePos      (const Vector2& p) { mHomePos       = p; }
    void SetShopPos      (const Vector2& p) { mShopPos       = p; }
    void SetParkPos      (const Vector2& p) { mParkPos       = p; }
    Vector2 GetPosForState(const char* stateName) const;

    int    GetEnergy()   const { return mEnergy;   }
    int    GetHunger()   const { return mHunger;   }
    int    GetStress()   const { return mStress;   }
    int    GetMoney()    const { return mMoney;    }
    int    GetProgress() const { return mProgress; }
    int    GetEpisode()  const { return mEpisode;  }
    int    GetEpTicks()  const { return mEpTicks;  }
    float  GetEpReward() const { return mEpReward; }
    double GetEpsilon()  const { return mQLearner.GetEpsilon(); }

    const char* GetFSMStateName() const;
    TimeOfDay   GetTimeOfDay() const { return TickToTimeOfDay(mEpTicks); }

    float GetUEnergy()   const { return float(mEnergy)   / StudentThreshold::MAX_ENERGY; }
    float GetUHunger()   const { return 1.f - float(mHunger)   / StudentThreshold::MAX_HUNGER; }
    float GetUStress()   const { return 1.f - float(mStress)   / StudentThreshold::MAX_STRESS; }
    float GetUMoney()    const { return float(mMoney)    / StudentThreshold::MAX_MONEY; }
    float GetUProgress() const { return float(mProgress) / StudentThreshold::MAX_PROGRESS; }
    float GetCompositeUtility() const;

    void AddEnergy  (int v);
    void AddHunger  (int v);
    void AddStress  (int v);
    void AddMoney   (int v);
    void AddProgress(int v);

    QLearner&     GetQLearner()   { return mQLearner; }
    StudentQState BuildQState() const;
    void          AddReward(float r) { mEpReward += r; }

    void AddLog(const std::string& msg);
    const std::deque<std::string>& GetLog() const { return mLog; }

    bool IsWalking() const { return mWalking; }
    AnimSpriteComponent* GetSprite()const {return asc;}

    EpisodeOutcome GetLastOutcome() const { return mLastOutcome; }
    float          GetLastReward()  const { return mLastReward;  }
    int            GetLastEpisode() const { return mLastEpisode; }

    float          GetFlashAlpha()   const { return mFlashTimer / FLASH_DURATION; }
    EpisodeOutcome GetFlashOutcome() const { return mFlashOutcome; }
    bool           IsFlashing()      const { return mFlashTimer > 0.f; }

    bool IsBetweenEpisodes() const { return mPauseTimer > 0.f; }
       // Resets STATS only — does NOT touch the Q-table or epsilon
   void StartFreshEpisode();

private:
    void BuildStates();
    void CheckEpisodeEnd();
    void EndEpisode(EpisodeOutcome outcome);
    void ResetStats();
    void UpdateWalking(float dt);

    StudentFSMState* mCurrentState = nullptr;
    std::unordered_map<std::string, StudentFSMState*> mStates;
    std::string mPendingState;

    bool                 mWalking = false;
    std::vector<Vector2> mWaypoints;
    int                  mCurrentWP = 0;
    float                mWalkSpeed = 200.f;
    static constexpr float ARRIVE_RADIUS = 8.0f;

    PathFinder mPathFinder;

    // ── Single sprite component — animated, replaces old plain SpriteComponent
    AnimSpriteComponent* asc = nullptr;

    QLearner mQLearner;

    int mEnergy   = StudentThreshold::MAX_ENERGY;
    int mHunger   = 0;
    int mStress   = 0;
    int mMoney    = 3;
    int mProgress = 0;

    int   mEpisode  = 0;
    float mEpReward = 0.f;
    int   mEpTicks  = 0;

    EpisodeOutcome mLastOutcome = EpisodeOutcome::None;
    float          mLastReward  = 0.f;
    int            mLastEpisode = -1;

    float          mFlashTimer   = 0.f;
    EpisodeOutcome mFlashOutcome = EpisodeOutcome::None;
    static constexpr float FLASH_DURATION = 1.0f;

    float mPauseTimer = 0.f;
    static constexpr float PAUSE_DURATION = 1.5f;

    Vector2 mUniversityPos { 704.f,   560.f };
    Vector2 mLibraryPos    { 464.f,   176.f };
    Vector2 mHomePos       {-880.f,   208.f };
    Vector2 mShopPos       {-352.f,  -128.f };
    Vector2 mParkPos       {-864.f,  -160.f };

    std::deque<std::string> mLog;
    static constexpr int MAX_LOG = 60;

          // Park roaming (slow circular walk while at park)
      float   mRoamTimer       = 0.f;
      float   mRoamAngle       = 0.f;
      bool    mRoamingInPark   = false;
      Vector2 mRoamCenter      {0.f, 0.f};
      static constexpr float ROAM_RADIUS   = 64.f;   // circle radius (pixels)
      static constexpr float ROAM_SPEED    = 0.4f;   // radians per second
      static constexpr float ROAM_REPLAN   = 4.5f;   // seconds between waypoint picks
};

#endif
