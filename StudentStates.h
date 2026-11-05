#pragma once
#ifndef STUDENTSTATES_H
#define STUDENTSTATES_H

#include "StudentFSMState.h"

//  Five FSM states, one per location.
//  Each one applies stat changes per tick and asks Q-Learning whether to leave.

// AtHome — restores energy, reduces stress
class StateAtHome : public StudentFSMState
{
public:
    StateAtHome() : StudentFSMState("AtHome") {}
    void Enter  (StudentActor* agent) override;
    void Execute(StudentActor* agent, float dt) override;
    void Exit   (StudentActor* agent) override;
private:
    float mTickTimer = 0.f;
    static constexpr float TICK_INTERVAL = 1.5f;
};

// AtUniversity — attends lectures, gains progress at high cost
class StateAtUniversity : public StudentFSMState
{
public:
    StateAtUniversity() : StudentFSMState("AtUniversity") {}
    void Enter  (StudentActor* agent) override;
    void Execute(StudentActor* agent, float dt) override;
    void Exit   (StudentActor* agent) override;
private:
    float mTickTimer = 0.f;
    static constexpr float TICK_INTERVAL = 2.0f;
};

// AtLibrary — studies, gains progress at lower cost
class StateAtLibrary : public StudentFSMState
{
public:
    StateAtLibrary() : StudentFSMState("AtLibrary") {}
    void Enter  (StudentActor* agent) override;
    void Execute(StudentActor* agent, float dt) override;
    void Exit   (StudentActor* agent) override;
private:
    float mTickTimer = 0.f;
    static constexpr float TICK_INTERVAL = 2.5f;
};

// AtShop — eats once, then leaves
class StateAtShop : public StudentFSMState
{
public:
    StateAtShop() : StudentFSMState("AtShop") {}
    void Enter  (StudentActor* agent) override;
    void Execute(StudentActor* agent, float dt) override;
    void Exit   (StudentActor* agent) override;
private:
    bool mEaten = false;
};

// AtPark — relaxes, clears stress
class StateAtPark : public StudentFSMState
{
public:
    StateAtPark() : StudentFSMState("AtPark") {}
    void Enter  (StudentActor* agent) override;
    void Execute(StudentActor* agent, float dt) override;
    void Exit   (StudentActor* agent) override;
private:
    float mTickTimer = 0.f;
    static constexpr float TICK_INTERVAL = 1.5f;
};

#endif
