#include "QLearner.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

QLearner::QLearner()
{
    // Optimistic initialisation — small positive Q-values encourage exploration
    for (int s = 0; s < StudentQState::STATE_COUNT; ++s)
        for (int a = 0; a < ACT_COUNT; ++a)
            mQTable[s][a] = 0.1f;
}


int QLearner::GetAction(int stateIdx)
{
    double r = static_cast<double>(rand()) / RAND_MAX;
    int action = (r < mEpsilon)
        ? (rand() % ACT_COUNT)   // explore
        : ArgMax(stateIdx);      // exploit

    // Decay epsilon over time — encourages more exploitation as we learn
    mEpsilon = std::max(EPSILON_MIN, mEpsilon * EPSILON_DECAY);
    return action;
}

//
//  Bellman update:
//  Q(s,a) ← Q(s,a) + α [r + γ · max(Q(s',a')) − Q(s,a)]
//
void QLearner::Update(int stateIdx, int action,
                      float reward, int nextStateIdx)
{
    float oldQ     = mQTable[stateIdx][action];
    float bestNext = MaxQ(nextStateIdx);
    mQTable[stateIdx][action] =
        oldQ + mAlpha * (reward + mGamma * bestNext - oldQ);
}

int QLearner::ArgMax(int stateIdx) const
{
    int   best  = 0;
    float bestV = mQTable[stateIdx][0];
    for (int a = 1; a < ACT_COUNT; ++a)
        if (mQTable[stateIdx][a] > bestV)
        {
            bestV = mQTable[stateIdx][a];
            best  = a;
        }
    return best;
}

float QLearner::MaxQ(int stateIdx) const
{
    float best = mQTable[stateIdx][0];
    for (int a = 1; a < ACT_COUNT; ++a)
        if (mQTable[stateIdx][a] > best)
            best = mQTable[stateIdx][a];
    return best;
}


//  SAVE  —  CSV format
//  First line: header
//  Second line: # epsilon: 0.342
//  Then one row per state: stateIdx, q[0], q[1], q[2], q[3], q[4]

void QLearner::SaveCSV(const std::string& filename) const
{
    std::ofstream f(filename);
    if (!f.is_open())
    {
        SDL_Log("QLearner: failed to open %s for writing", filename.c_str());
        return;
    }

    f << "# epsilon: " << mEpsilon << "\n";
    f << "state,attend,study,rest,eat,relax\n";

    for (int s = 0; s < StudentQState::STATE_COUNT; ++s)
    {
        f << s;
        for (int a = 0; a < ACT_COUNT; ++a)
            f << "," << mQTable[s][a];
        f << "\n";
    }
}

//
//  LOAD  —  reads the Q-table back so learning continues across launches
//
bool QLearner::LoadCSV(const std::string& filename)
{
    std::ifstream f(filename);
    if (!f.is_open())
    {
        SDL_Log("QLearner: no saved Q-table found (%s) — starting fresh",
                filename.c_str());
        return false;
    }

    std::string line;
    int loaded = 0;

    while (std::getline(f, line))
    {
        if (line.empty()) continue;

        // Header line: "# epsilon: 0.342"
        if (line[0] == '#')
        {
            auto pos = line.find("epsilon:");
            if (pos != std::string::npos)
            {
                std::string val = line.substr(pos + 8);
                try { mEpsilon = std::stod(val); } catch (...) {}
            }
            continue;
        }

        // Skip column header
        if (line.find("state,") == 0) continue;

        // Parse row: stateIdx, q0, q1, q2, q3, q4
        std::stringstream ss(line);
        std::string token;
        std::vector<float> values;

        while (std::getline(ss, token, ','))
        {
            try { values.push_back(std::stof(token)); }
            catch (...) { values.push_back(0.f); }
        }

        if (values.size() < 1 + ACT_COUNT) continue;

        int s = static_cast<int>(values[0]);
        if (s < 0 || s >= StudentQState::STATE_COUNT) continue;

        for (int a = 0; a < ACT_COUNT; ++a)
            mQTable[s][a] = values[1 + a];

        loaded++;
    }

    SDL_Log("QLearner: loaded %d states from %s (epsilon = %.4f)",
            loaded, filename.c_str(), mEpsilon);
    return loaded > 0;
}
