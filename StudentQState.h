#pragma once
#ifndef STUDENTQSTATE_H
#define STUDENTQSTATE_H

#include <string>

//  StudentQState  —> the discrete state fed into the Q-table
//
//  Six dimensions: 5 stats + time of day
//  State space: 7 × 6 × 7 × 6 × 6 × 3 = 27,216 unique states

//Time of day phases
enum class TimeOfDay
{
    Morning   = 0,   // first third of episode
    Afternoon = 1,   // middle third
    Evening   = 2    // final third
};

inline const char* TimeOfDayName(TimeOfDay t)
{
    switch (t)
    {
        case TimeOfDay::Morning:   return "Morning";
        case TimeOfDay::Afternoon: return "Afternoon";
        case TimeOfDay::Evening:   return "Evening";
    }
    return "Morning";
}

//Discrete state
struct StudentQState
{
    int energy;    // 0–6
    int hunger;    // 0–5
    int stress;    // 0–6
    int money;     // 0–5
    int progress;  // 0–5
    int timeOfDay; // 0–2

    int ToIndex() const
    {
        return energy
             + 7         * hunger
             + 7*6       * stress
             + 7*6*7     * money
             + 7*6*7*6   * progress
             + 7*6*7*6*6 * timeOfDay;
    }

    static constexpr int STATE_COUNT = 7 * 6 * 7 * 6 * 6 * 3; // 27216
};

//Actions
enum StudentAction
{
    ACT_ATTEND = 0,  // AtUniversity
    ACT_STUDY  = 1,  // AtLibrary
    ACT_REST   = 2,  //AtHome
    ACT_EAT    = 3,  // AtShop
    ACT_RELAX  = 4,  // AtPark
    ACT_COUNT  = 5
};

inline const char* ActionToStateName(int action)
{
    switch (action)
    {
        case ACT_ATTEND: return "AtUniversity";
        case ACT_STUDY:  return "AtLibrary";
        case ACT_REST:   return "AtHome";
        case ACT_EAT:    return "AtShop";
        case ACT_RELAX:  return "AtPark";
        default:         return "AtHome";
    }
}

//Stat thresholds and episode timing
namespace StudentThreshold
{
    constexpr int MAX_ENERGY   = 6;
    constexpr int MAX_HUNGER   = 5;
    constexpr int MAX_STRESS   = 6;
    constexpr int MAX_MONEY    = 5;
    constexpr int MAX_PROGRESS = 5;

    constexpr int REST_ENERGY    = 2;
    constexpr int EAT_HUNGER     = 4;
    constexpr int RELAX_STRESS   = 5;
    constexpr int STUDY_STRESS   = 5;
    constexpr int LECTURE_ENERGY = 2;

    constexpr int MAX_EP_TICKS  = 5000;
    constexpr int MORNING_END   = 1667;
    constexpr int AFTERNOON_END = 3333;
}

// Convert episode tick to time of day
inline TimeOfDay TickToTimeOfDay(int tick)
{
    if (tick < StudentThreshold::MORNING_END)   return TimeOfDay::Morning;
    if (tick < StudentThreshold::AFTERNOON_END) return TimeOfDay::Afternoon;
    return TimeOfDay::Evening;
}

#endif
