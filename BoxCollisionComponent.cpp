#include "BoxCollisionComponent.h"
#include "Actor.h"

BoxCollisionComponent::BoxCollisionComponent(Actor* owner):Component(owner)
,mHeight(32.0f)
,mWidth(32.0f)
{

}
void BoxCollisionComponent::SetSize(float w, float h)
{
    mWidth = w;
    mHeight = h;
}

Vector2 BoxCollisionComponent::GetMin() const
{
    return mOwner->GetPosition() - Vector2(mWidth / 2, mHeight / 2);
}

Vector2 BoxCollisionComponent::GetMax() const
{
    return mOwner->GetPosition() + Vector2(mWidth / 2, mHeight / 2);
}
