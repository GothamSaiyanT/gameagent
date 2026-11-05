#include <exception>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include "Game.h"


int main( int argc, char * argv[] )
{   Game game;
    int success = game.Initialize();
    if(success){
        game.RunLoop();
    }
    game.ShutDown();

    return 0;
}
