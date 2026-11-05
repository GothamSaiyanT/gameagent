 #include "CircleComponent.h"
 #include "Actor.h"

CircleComponent::CircleComponent(class Actor* owner)
:Component(owner)
,mRadius(0.0f)
{
}
float CircleComponent::GetRadius()const{
    return mOwner->GetScale() * mRadius;
}

const Vector2& CircleComponent::GetCenter()const{
    return mOwner->GetPosition();
}
bool Intersect(const CircleComponent& a,const CircleComponent& b){
    //calculate the distance squared
    Vector2 diff = a.GetCenter() - b.GetCenter();
    float distSq = diff.LengthSq();

    //calculate the sum
    float raddisq = a.GetRadius() + b.GetRadius();
    return distSq <= raddisq;
}
