#ifndef KEYBOARDSTATE_H_INCLUDED
#define KEYBOARDSTATE_H_INCLUDED

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

#endif // KEYBOARDSTATE_H_INCLUDED
