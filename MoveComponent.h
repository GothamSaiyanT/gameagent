#ifndef MOVECOMPONENT_H_INCLUDED
#define MOVECOMPONENT_H_INCLUDED

#include "Component.h"
#include "Renderer.h"
enum class CharacterState
{
    Idle,
    Walking,
    Jumping
};
class MoveComponent:public Component
{
public:
    MoveComponent(class Actor* owner,int updateOrder = 10);

    void Update(float deltaTime)override;

    float GeTAngularSpeed()const{return mAngularSpeed;}
    float GetForwardSpeed()const {return mForwardSpeed;}
    void SetAngularSpeed(float angSpeed){mAngularSpeed = angSpeed;}
    void SetForwardSpeed(float FSpeed){mForwardSpeed = FSpeed;}

    //CharacterState GetState()const {return mState;}
    CharacterState GetState() const
    {
            if (!mOnGround)
        return CharacterState::Jumping;
        if (!Math::NearZero(mForwardSpeed))
            return CharacterState::Walking;
        return CharacterState::Idle;
    }

    void Jump(float velocity); // call to jump

private:
    //controls rotation(radians/second)
    float mAngularSpeed;
    //controls forward speed
    float mForwardSpeed;

    float mVerticalSpeed = 0.0f; // for jumping
    bool mOnGround = true;       // is actor on the ground

    CharacterState mState = CharacterState::Idle; // current state

     const float Gravity = 980.0f; // pixels/sec^2


};


#endif // MOVECOMPONENT_H_INCLUDED
