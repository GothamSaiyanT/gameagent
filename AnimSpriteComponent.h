#ifndef ANIMSPRITECOMPONENT_H_INCLUDED
#define ANIMSPRITECOMPONENT_H_INCLUDED

#pragma once
#include "SpriteComponent.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "InputComponent.h"
struct AnimationData {
    std::vector<Texture*> textures;
    bool looping;
};

class AnimSpriteComponent: public SpriteComponent
{
public:
    AnimSpriteComponent(class Actor* owner,int draOrder = 100);
    //update animation every frame(overriden from component)
    void Update(float deltaTime) override;
    //set the tectures used for animation
    void SetAnimTextures(const std::string& animeName, const std::vector<Texture*>& textures,bool looping);
    //set/get the animation FPS
    float GetAnimeFPS()const{return mAnimFPS;}
    void SetAnimFPS(float fps){mAnimFPS = fps;}
    void SetAnimation(const std::string& Name);


    void UpdateAnimation(CharacterState state);


private:
    //all textures in animation
    //std::vector<SDL_Texture*> mAnimTextures;
    std::unordered_map<std::string,AnimationData> mAnimation;
    //current frame displayed
    float mCurrframe;
    //animation frame rate
    float mAnimFPS;
    std::string mCurrentAnim;
    // Define struct for animation data

        bool mAnimFinished = false; // NEW: track if current animation ended
};


#endif // ANIMSPRITECOMPONENT_H_INCLUDED
