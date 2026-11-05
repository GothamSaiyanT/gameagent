#ifndef SPRITECOMPONENT_H_INCLUDED
#define SPRITECOMPONENT_H_INCLUDED
#pragma once
#include "Component.h"
#include "SDL2/SDL.h"
#include "Shader.h"
#include "Texture.h"

class SpriteComponent:public Component
{
public:
    //(lower draw order corresponds with further back)
    SpriteComponent(class Actor* owner,int drawOrder = 100);
    ~SpriteComponent();
    virtual void Draw(Shader* shader);
    virtual void SetTexture(class Texture* texture);

    int GetDrawOrder()const{return mDrawOrder;}
    int GetTexHeight()const{return mTexHeight;}
    int GetTexWidth()const{return mTexWidth;}

    void SetFlipX(bool flip) { mFlipX = flip; }
    bool GetFlipX() const { return mFlipX; }
    bool IsVisible() const{return  mVisible;}
    void SetVisible(bool Visible){ mVisible =Visible;}

protected:
    //texture to draw
    class Texture* mTexture;
    //draw order used for painter's algorithm
    int mDrawOrder;
    //width/height of texture
    int mTexWidth;
    int mTexHeight;
    bool mVisible = true;
         bool mFlipX = false; // default: not flipped

};

#endif // SPRITECOMPONENT_H_INCLUDED
