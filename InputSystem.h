#ifndef INPUTSYSTEM_H_INCLUDED
#define INPUTSYSTEM_H_INCLUDED

#include "SDL2/SDL.h"
enum ButtonState
{
    ENone,
    EPressed,
    EReleased,
    EHeld
};



class KeyboardState
{
    public:
        friend class InputSystem;

        //get just boolean true/false value of key
        bool GetKeyValue(SDL_Scancode keyCode) const;

        //Get a state based on current and previous framw
        ButtonState GetKeyState(SDL_Scancode keyCode)const;

    private:
        //current state
        const Uint8* mCurrentState;
        //state previous frame
        Uint8 mPrevState[SDL_NUM_SCANCODES];

};

//wrapper that contains current state of input
struct InputState
{
    KeyboardState Keyboard;
};

class InputSystem
{
    public:
        bool Initialize();
        void ShutDown();

        //called right before SDL_PollEvents loop
        void PrepareForUpdate();
        //called right after SDL_pollevents loop
        void Update();

        //called to process an sdl event in input system
        void ProcessEvent(union SDL_Event& event);

        const InputState& GetState()const {return mState;}

    private:
        InputState mState;


};



#endif // INPUTSYSTEM_H_INCLUDED
