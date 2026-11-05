#include "InputSystem.h"
#include <SDL2/SDL.h>
#include <cstring>

    //get just boolean true/false value of key
bool KeyboardState::GetKeyValue(SDL_Scancode keyCode) const
{
    return mCurrentState[keyCode] == 1;
}
//Get a state based on current and previous framw
ButtonState KeyboardState::GetKeyState(SDL_Scancode keyCode)const
{
    if(mPrevState[keyCode] == 0)
    {
        if(mCurrentState[keyCode] == 0)
        {
            return ENone;
        }else
        {
            return EPressed;
        }
    }
    else//prev state must be 1
    {
        if(mCurrentState[keyCode] == 0)
        {
            return EReleased;
        }else
        {
            return EHeld;
        }
    }
}
bool InputSystem::Initialize()
{
    //keyboard
    //assign current state pointer
    mState.Keyboard.mCurrentState = SDL_GetKeyboardState(NULL);
    //clear previuos state memory
    memset(mState.Keyboard.mPrevState,0,
           SDL_NUM_SCANCODES);

           return true;
}
void InputSystem::ShutDown()
{

}

//called right before SDL_PollEvents loop
void InputSystem::PrepareForUpdate()
{
    //copy current state to previous
    //keyboard
    memcpy(mState.Keyboard.mPrevState,
           mState.Keyboard.mCurrentState,
           SDL_NUM_SCANCODES);

}
//called right after SDL_pollevents loop
void InputSystem::Update()
{

}

//called to process an sdl event in input system
void InputSystem::ProcessEvent(union SDL_Event& event)
{

}
