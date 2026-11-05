#ifndef BOXCOLLISIONCOMPONENT_H_INCLUDED
#define BOXCOLLISIONCOMPONENT_H_INCLUDED

#include "Component.h"
#include "math.h"
class BoxCollisionComponent : public Component
{
public:
    BoxCollisionComponent(Actor* owner);

    Vector2 GetMin() const;
    Vector2 GetMax() const;
    float GetWidth() const { return mWidth; }
float GetHeight() const { return mHeight; }

    void SetSize(float w, float h);

private:
    float mWidth;
    float mHeight;
};


#endif // BOXCOLLISIONCOMPONENT_H_INCLUDED
