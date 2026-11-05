#ifndef BGSPRITECOMPONENT_H_INCLUDED
#define BGSPRITECOMPONENT_H_INCLUDED

#pragma once
#include "SpriteComponent.h"
#include <vector>
#include "Math.h"
#include "Texture.h"

class BGSpriteComponent:public SpriteComponent
{
public:
    //set draw order to default to lower (bACKGROUND)
    BGSpriteComponent(class Actor* owner,int drawOrder = 10);
    //update/draw overriden from parent
    void Update(float deltaTime) override;
    void Draw(Shader* Shader) override;

    //set the textures used for background
    void SetBGtextures(const std::vector<class Texture*>& textures,const std::vector<float>& parallaxSpeeds);
    //get /set screen size and scroll speed
    void SetScreenSize(const Vector2& Size){ mScreenSize = Size;}
    void setScrollSpeed(float speed){ mScrollSpeed = speed;}
    float GetScrollSpeed()const{return mScrollSpeed;}


private:
    //struct to encapsulate each BG image and its offset
    struct BGTexture
    {
        class Texture* mTexture;
        Vector2 mOffset;
        float mParallexSpeed;
    };
    std::vector<BGTexture> mBGTexture;
    Vector2 mScreenSize;
    float mScrollSpeed;



};;


#endif // BGSPRITECOMPONENT_H_INCLUDED
