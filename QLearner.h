#pragma once
#ifndef QLEARNER_H
#define QLEARNER_H

#include "StudentQState.h"
#include <string>

//
//  QLearner  —  Student Agent's Q-Learning brain
//
//  Q-table: 27,216 states × 5 actions
//  - Epsilon-greedy action selection with decay
//  - Bellman update after every action
//  - Persists Q-table to disk so learning continues between game launches
//
class QLearner
{
public:
    QLearner();

    // Pick best action for a state (epsilon-greedy)
    int  GetAction(int stateIdx);

    // Bellman update after reward + next state
    void Update(int stateIdx, int action, float reward, int nextStateIdx);

    // Persistence
    // Save Q-table + epsilon + episode counter
    void SaveCSV(const std::string& filename) const;

    // Load Q-table from disk. Returns false if file doesn't exist (first run)
    bool LoadCSV(const std::string& filename);

    //  Inspection
    double GetEpsilon()    const { return mEpsilon; }
    int    GetStateCount() const { return StudentQState::STATE_COUNT; }
    float  GetQValue(int s, int a) const { return mQTable[s][a]; }

    // Allow Game/StudentActor to set epsilon manually after load
    void SetEpsilon(double e) { mEpsilon = e; }

private:
    float  mQTable[StudentQState::STATE_COUNT][ACT_COUNT];

    double mEpsilon = 1.0;
    float  mAlpha   = 0.25f;
    float  mGamma   = 0.9f;

    static constexpr double EPSILON_MIN   = 0.05;
    static constexpr double EPSILON_DECAY = 0.9995;

    int   ArgMax(int stateIdx) const;
    float MaxQ  (int stateIdx) const;
};

#endif
