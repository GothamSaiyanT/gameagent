#include "InputComponent.h"
#include "InputSystem.h"

    InputComponent::InputComponent(class Actor* owner)
    :MoveComponent(owner)
    ,mForwardkey(0)
    ,mBackKey(0)
    ,mClockWiseKey(0)
    ,mCounterClockwiseKey(0)
    {
    }
   void InputComponent::ProcessInput(const struct InputState& state){

        //calculate forward speed for movement
        float forwardSpeed = 0.0f;
        if(state.Keyboard.GetKeyValue(SDL_Scancode(mForwardkey))){
            forwardSpeed += mMaximumForwardSpeed;
        }
        if(state.Keyboard.GetKeyValue(SDL_Scancode(mBackKey))){
            forwardSpeed -= mMaximumForwardSpeed;
        }
        SetForwardSpeed(forwardSpeed);

        //calculate angular speed for movecomponent
            float angularSpeed = 0.0f;
        if(state.Keyboard.GetKeyValue(SDL_Scancode(mClockWiseKey))){
            angularSpeed += mMaximumAngularSpeed;
        }
        if(state.Keyboard.GetKeyValue(SDL_Scancode(mCounterClockwiseKey))){
            angularSpeed -= mMaximumAngularSpeed;
        }
        SetAngularSpeed(angularSpeed);

            // Jump
    if (state.Keyboard.GetKeyValue(SDL_Scancode(mJumpKey)))
    {
        Jump(400.0f); // upward velocity
    }
   }
   CharacterState InputComponent::GetState() const
{
    // Use MoveComponent's mForwardSpeed
    return GetForwardSpeed() != 0.0f ? CharacterState::Walking : CharacterState::Idle;
}

