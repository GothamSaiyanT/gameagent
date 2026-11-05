#ifndef CIRCLECOMPONENT_H_INCLUDED
#define CIRCLECOMPONENT_H_INCLUDED

#include "Component.h"
#include "Math.h"
class CircleComponent: public Component
{
public:
    CircleComponent(class Actor* owner);
    void SetRadius(float radius){mRadius = radius;}
    float GetRadius()const;
    const Vector2& GetCenter()const;
private:
    float mRadius;

};
    bool Intersect(const CircleComponent& a,const CircleComponent& b);

#endif // CIRCLECOMPONENT_H_INCLUDED
