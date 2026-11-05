#include "Component.h"
#include "Actor.h"

Component::Component(class Actor* owner,int updateOrder )
:mOwner(owner),
mUpdateOrder(updateOrder)
{
    mOwner->AddComponent(this);
}
//destructor
Component::~Component(){
    mOwner->RemoveComponent(this);
}
//update this component by delta
void Component::Update(float deltaTime){

}
