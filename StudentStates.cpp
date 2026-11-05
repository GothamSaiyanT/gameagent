#include "StudentStates.h"
#include "StudentActor.h"
#include "QLearner.h"
#include "StudentQState.h"
#include <algorithm>
#include <string>

using namespace StudentThreshold;

//  Q-Learning helpers — used by every state's Execute()
// Ask Q-Learner where it wants to go next based on current state
static const char* QPreferredState(StudentActor* agent)
{
    StudentQState qs = agent->BuildQState();
    int action = agent->GetQLearner().GetAction(qs.ToIndex());
    return ActionToStateName(action);
}

// Apply Bellman update — call after stat changes so reward reflects what happened
static void QUpdate(StudentActor* agent,
                    const StudentQState& prevQS,
                    int action, float reward)
{
    StudentQState nextQS = agent->BuildQState();
    agent->GetQLearner().Update(
    prevQS.ToIndex(), action, reward, nextQS.ToIndex());
    agent->AddReward(reward);
}

// Time-of-day reward bonus :teaches realistic daily routine
static float TimeBonus(StudentActor* agent, int action)
{
    TimeOfDay tod = agent->GetTimeOfDay();
    switch (action)
    {
        case ACT_ATTEND:
            if (tod == TimeOfDay::Morning)   return  1.5f;
            if (tod == TimeOfDay::Afternoon) return  0.5f;
            return -0.5f;
        case ACT_STUDY:
            if (tod == TimeOfDay::Morning ||
                tod == TimeOfDay::Afternoon) return 1.0f;
            return 0.0f;
        case ACT_REST:
            if (tod == TimeOfDay::Evening)   return 1.0f;
            return 0.0f;
        case ACT_RELAX:
            if (tod == TimeOfDay::Afternoon) return 0.5f;
            return 0.0f;
    }
    return 0.0f;
}

//  StateAtHome  —  Resting
void StateAtHome::Enter(StudentActor* agent)
{
    mTickTimer = 0.f;
    agent->AddLog(std::string("Student: [") +
        TimeOfDayName(agent->GetTimeOfDay()) + "] Home -> resting.");
}

void StateAtHome::Execute(StudentActor* agent, float dt)
{
    mTickTimer += dt;
    if (mTickTimer < TICK_INTERVAL) return;
    mTickTimer = 0.f;

    StudentQState prevQS = agent->BuildQState();

    // Apply rest effects
    agent->AddEnergy( 2);
    agent->AddMoney(1);
    //agent->AddStress(-1);
    agent->AddHunger(1);

if (agent->GetHunger() >= MAX_HUNGER)
{
    agent->AddLog("Student: Starved at home!");

    QUpdate(agent, prevQS, ACT_REST, -50.0f);

    agent->TransitionTo("AtHome");
    return;
}

    // Reward
    float reward = 0.f;
    if (prevQS.energy <= REST_ENERGY) reward += 4.0f;   // good — was tired
    else                              reward -= 2.0f;   // wasted time
    // NEW: hunger penalties
    if (agent->GetHunger() >= EAT_HUNGER)
        reward -= 4.0f;

    if (agent->GetHunger() >= MAX_HUNGER)
        reward -= 10.0f;

    reward += TimeBonus(agent, ACT_REST);

    QUpdate(agent, prevQS, ACT_REST, reward);


    // Decide whether to leave
    const char* preferred = QPreferredState(agent);
    bool fullyRested = (agent->GetEnergy() >= MAX_ENERGY
                     && agent->GetStress() <= 1);

    if (fullyRested || std::string(preferred) != GetName())
        agent->TransitionTo(preferred);
}

void StateAtHome::Exit(StudentActor* agent)
{
    agent->AddLog("Student: Leaving home.");
}


//  StateAtUniversity  —  Attending Lectures
void StateAtUniversity::Enter(StudentActor* agent)
{
    mTickTimer = 0.f;
    agent->AddLog(std::string("Student: [") +
        TimeOfDayName(agent->GetTimeOfDay()) + "] At uni — attending lecture.");
}

void StateAtUniversity::Execute(StudentActor* agent, float dt)
{
    mTickTimer += dt;
    if (mTickTimer < TICK_INTERVAL) return;
    mTickTimer = 0.f;

    StudentQState prevQS = agent->BuildQState();

    // Safety: collapse check
    if (agent->GetEnergy() <= 0)
    {
        QUpdate(agent, prevQS, ACT_ATTEND, -5.0f);
        agent->AddLog("Student: Collapsed at uni!");
        agent->TransitionTo("AtHome");
        return;
    }

    // Apply lecture effects
    agent->AddProgress( 1);
    agent->AddEnergy  (-2);
    agent->AddStress  ( 1);
    agent->AddHunger  ( 1);

    // Starvation failure
if (agent->GetHunger() >= MAX_HUNGER)
{
    agent->AddLog("Student: Starved at university!");

    QUpdate(agent, prevQS, ACT_ATTEND, -50.0f);

    agent->TransitionTo("AtHome"); // or reset episode
    return;
}

    // Reward
    float reward = 3.0f;
    // Progressive exhaustion penalty
reward -= (MAX_ENERGY - agent->GetEnergy()) * 0.5f;
// Progressive stress penalty
reward -= agent->GetStress() * 0.7f;
    if (agent->GetStress() >= MAX_STRESS) reward -= 3.0f;
    if (agent->GetEnergy() <= 0)          reward -= 4.0f;
    // NEW: hunger penalties
if (agent->GetHunger() >= EAT_HUNGER)
    reward -= 4.0f;

if (agent->GetHunger() >= MAX_HUNGER)
    reward -= 10.0f;
    reward += TimeBonus(agent, ACT_ATTEND);

    QUpdate(agent, prevQS, ACT_ATTEND, reward);

    // Hard safety overrides
    if (agent->GetEnergy() <= LECTURE_ENERGY)
    {
        agent->AddLog("Student: Too tired for lectures.");
        agent->TransitionTo("AtHome");
        return;
    }
    if (agent->GetStress() >= MAX_STRESS)
    {
        agent->AddLog("Student: Stressed out — to the park.");
        agent->TransitionTo("AtPark");
        return;
    }
    // Q-Learning preferred destination
    const char* preferred = QPreferredState(agent);
    if (std::string(preferred) != GetName())
        agent->TransitionTo(preferred);
}

void StateAtUniversity::Exit(StudentActor* agent)
{
    agent->AddLog("Student: Leaving university.");
}


//  StateAtLibrary  —  Studying
void StateAtLibrary::Enter(StudentActor* agent)
{
    mTickTimer = 0.f;
    agent->AddLog(std::string("Student: [") +
        TimeOfDayName(agent->GetTimeOfDay()) + "] Library — studying.");
}

void StateAtLibrary::Execute(StudentActor* agent, float dt)
{
    mTickTimer += dt;
    if (mTickTimer < TICK_INTERVAL) return;
    mTickTimer = 0.f;

    StudentQState prevQS = agent->BuildQState();

    if (agent->GetEnergy() <= 0)
    {
        QUpdate(agent, prevQS, ACT_STUDY, -5.0f);
        agent->AddLog("Student: Fell asleep at library!");
        agent->TransitionTo("AtHome");
        return;
    }

    agent->AddProgress( 1);
    agent->AddEnergy  (-1);
    agent->AddStress  ( 1);
    agent->AddHunger  ( 1);

    // Starvation failure
if (agent->GetHunger() >= MAX_HUNGER)
{
    agent->AddLog("Student: Starved while studying!");

    QUpdate(agent, prevQS, ACT_STUDY, -50.0f);

    agent->TransitionTo("AtHome");
    return;
}

    float reward = 2.0f;
    // Progressive exhaustion penalty
reward -= (MAX_ENERGY - agent->GetEnergy()) * 0.5f;
// Progressive stress penalty
reward -= agent->GetStress() * 0.7f;
    if (agent->GetStress() >= STUDY_STRESS) reward -= 2.0f;
    if (agent->GetEnergy() <= 0)            reward -= 4.0f;
    // NEW: hunger penalties
if (agent->GetHunger() >= EAT_HUNGER)
    reward -= 4.0f;

if (agent->GetHunger() >= MAX_HUNGER)
    reward -= 10.0f;
    reward += TimeBonus(agent, ACT_STUDY);

    QUpdate(agent, prevQS, ACT_STUDY, reward);

    if (agent->GetEnergy() <= 1)
    {
        agent->AddLog("Student: Can't focus — going home.");
        agent->TransitionTo("AtHome");
        return;
    }
    if (agent->GetStress() >= STUDY_STRESS)
    {
        agent->AddLog("Student: Need a break — to the park.");
        agent->TransitionTo("AtPark");
        return;
    }


    const char* preferred = QPreferredState(agent);
    if (std::string(preferred) != GetName())
        agent->TransitionTo(preferred);
}

void StateAtLibrary::Exit(StudentActor* agent)
{
    agent->AddLog("Student: Leaving library.");
}


//  StateAtShop  —  Eating
void StateAtShop::Enter(StudentActor* agent)
{
    mEaten = false;
    agent->AddLog(std::string("Student: [") +
        TimeOfDayName(agent->GetTimeOfDay()) + "] At shop — buying food.");
}

void StateAtShop::Execute(StudentActor* agent, float dt)
{
    if (mEaten) return;
    mEaten = true;

    StudentQState prevQS = agent->BuildQState();
    float reward = 0.f;

    if (agent->GetMoney() <= 0)
    {
        // Broke — Q-Learning learns not to come here
        reward -= 4.0f;
        QUpdate(agent, prevQS, ACT_EAT, reward);
        agent->AddLog("Student: No money!");
        const char* preferred = QPreferredState(agent);
        agent->TransitionTo(preferred);
        return;
    }

    // Eat
    agent->AddHunger(-3);   // clear hunger
    agent->AddMoney (-3);
    agent->AddEnergy(1);

reward = 0.0f;

// Reward eating based on how hungry the student was
reward += prevQS.hunger * 4.0f;

// Extra reward if critically hungry
if (prevQS.hunger >= MAX_HUNGER - 1)
{
    reward += 10.0f;
}

// Punish unnecessary eating
if (prevQS.hunger < EAT_HUNGER)
{
    reward -= 5.0f;
}

reward += TimeBonus(agent, ACT_EAT);


    QUpdate(agent, prevQS, ACT_EAT, reward);
    agent->AddLog(std::string("Student: Ate. Money: ")
                  + std::to_string(agent->GetMoney()));

    const char* preferred = QPreferredState(agent);
    agent->TransitionTo(preferred);
}

void StateAtShop::Exit(StudentActor* agent)
{
    (void)agent;
}

//  StateAtPark  —  Relaxing

void StateAtPark::Enter(StudentActor* agent)
{
    mTickTimer = 0.f;
    agent->AddLog(std::string("Student: [") +
        TimeOfDayName(agent->GetTimeOfDay()) + "] Park — relaxing.");
}

void StateAtPark::Execute(StudentActor* agent, float dt)
{
    mTickTimer += dt;
    if (mTickTimer < TICK_INTERVAL) return;
    mTickTimer = 0.f;

    StudentQState prevQS = agent->BuildQState();

    agent->AddStress(-2);
    agent->AddEnergy( 1);
    agent->AddHunger( 1);

    // Starvation failure
if (agent->GetHunger() >= MAX_HUNGER)
{
    agent->AddLog("Student: Starved in the park!");

    QUpdate(agent, prevQS, ACT_RELAX, -50.0f);

    agent->TransitionTo("AtHome");
    return;
}

    float reward = 2.5f * std::min(2, prevQS.stress);
    if (prevQS.stress < RELAX_STRESS) reward -= 1.0f;
    reward += TimeBonus(agent, ACT_RELAX);

    QUpdate(agent, prevQS, ACT_RELAX, reward);


    // Force leave once stress is fully cleared
    if (agent->GetStress() <= 0)
    {
        agent->AddLog("Student: Refreshed.");
        const char* preferred = QPreferredState(agent);
        agent->TransitionTo(preferred);
        return;
    }

    const char* preferred = QPreferredState(agent);
    if (std::string(preferred) != GetName())
        agent->TransitionTo(preferred);
}

void StateAtPark::Exit(StudentActor* agent)
{
    agent->AddLog("Student: Leaving park.");
}
