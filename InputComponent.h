#ifndef INPUTCOMPONENT_H_INCLUDED
#define INPUTCOMPONENT_H_INCLUDED

#include "MoveComponent.h"
#include "Actor.h"


class InputComponent:public MoveComponent
{
public:
    InputComponent(class Actor* owner);
    void ProcessInput(const struct InputState& state) override;

    //getters and setters
    float GetMaximumForwardSpeed()const{return mMaximumForwardSpeed;}
    float GetMaximumAngularSpeed()const{return mMaximumAngularSpeed;}
    int GetForwardKey()const{return mForwardkey;}
    int GetBackKey()const{return mBackKey;}
    int GetClockwiseKey()const{return mClockWiseKey;}
    int GetCounterClockwiseKey()const{return mCounterClockwiseKey;}


    CharacterState GetState() const;
    void SetMaximuForwardSpeed(float speed){mMaximumForwardSpeed = speed;}
    void SetMaximuAngularSpeed(float speed){mMaximumAngularSpeed = speed;}
    void SetForwardkey(int key){mForwardkey = key;}
    void SetBackkey(int key){mBackKey = key;}
    void SetClockWiseKey(int key){mClockWiseKey = key;}
    void SetJumpKey(int key) { mJumpKey = key; }
    void SetCounterClockWiseKey(int key){mCounterClockwiseKey = key;}

private:
    //the maximum forward/angular speed
    float mMaximumForwardSpeed;
    float mMaximumAngularSpeed;

    //keys for forward/back movemnt
    int mForwardkey;
    int mBackKey;
    int mJumpKey; // new jump key

    //key for angular movement
    int mClockWiseKey;
    int mCounterClockwiseKey;
};

#endif // INPUTCOMPONENT_H_INCLUDED
