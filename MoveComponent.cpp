#include "MoveComponent.h"
#include "Math.h"
#include "Component.h"
#include "Actor.h"

MoveComponent::MoveComponent(Actor *owner,int UpdateOrder )
:Component(owner,UpdateOrder)
,mForwardSpeed(0.0f)
,mAngularSpeed(0.0f)
{

}
void MoveComponent::Update(float deltaTime){
    if(!Math::NearZero(mAngularSpeed)){

        float rot = mOwner->GetRotation();
        rot += mAngularSpeed * deltaTime;
        mOwner->SetRotation(rot);

    }
    if(!Math::NearZero(mForwardSpeed)){
        Vector2 pos = mOwner->GetPosition();
        pos += mOwner->GetForward()* mForwardSpeed * deltaTime;
        mOwner->SetPosition(pos);

    }
        if (!mOnGround || !Math::NearZero(mVerticalSpeed))
    {
        mVerticalSpeed -= Gravity * deltaTime; // gravity
        Vector2 pos = mOwner->GetPosition();
        pos.y += mVerticalSpeed * deltaTime;

        if (pos.y <= 0.0f) // ground collision
        {
            pos.y = 0.0f;
            mVerticalSpeed = 0.0f;
            mOnGround = true;
        }
        else
        {
            mState = CharacterState::Jumping;
            mOnGround = false;
        }

        mOwner->SetPosition(pos);
    }
}
void MoveComponent::Jump(float velocity)
{
    if (mOnGround)
    {
        mVerticalSpeed = velocity;
        mOnGround = false;
        mState = CharacterState::Jumping;
    }
}

