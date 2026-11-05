#pragma once
#ifndef STUDENTFSMSTATE_H
#define STUDENTFSMSTATE_H


//  StudentFSMState  —  abstract base class (GoF State Pattern)
//
//  Each concrete state (AtHome, AtUniversity, etc.) implements:
//    Enter()    — called once when entering this state
//    Execute()  — called every tick while in this state
//    Exit()     — called once when leaving this state
//
//  States call agent->TransitionTo("StateName") directly to change states.

class StudentActor;  // forward declaration

class StudentFSMState
{
public:
    explicit StudentFSMState(const char* name) : mName(name) {}
    virtual ~StudentFSMState() = default;

    virtual void Enter  (StudentActor* agent) = 0;
    virtual void Execute(StudentActor* agent, float dt) = 0;
    virtual void Exit   (StudentActor* agent) = 0;

    const char* GetName() const { return mName; }

private:
    const char* mName;
};

#endif
