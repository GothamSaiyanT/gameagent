#include "Actor.h"
#include "Game.h"
#include "Component.h"
#include <algorithm>

//construtor
Actor::Actor( Game* game)
:mState(EActive)
,mPosition(Vector2::Zero)
,mScale(1.0f)
,mRotation(0.0f)
,mGame(game)
,mRecomputeWorldTransform(true)
{
    mGame->AddActor(this);
}
Actor::~Actor(){
    mGame->RemoveActor(this);
    //need to delete the componets
    //since ~componet calls remove compone we need a different loop
    while(!mComponents.empty()){
        delete mComponents.back();
    }
}

//update function called from game(no override
void Actor::Update(float deltaTime){
    if(mState == EActive){
    ComputeWorldTransform();
    UpdateComponets(deltaTime);
    UpdateActor(deltaTime);

    ComputeWorldTransform();
    }

}
//update the componets attacted to actor no overriden
void Actor::UpdateComponets(float deltaTime){
    for(auto comp:mComponents){
        comp->Update(deltaTime);
    }
}
//any actor - specific update code(overridable)
void Actor::UpdateActor(float deltaTime){
}
void Actor::AddComponent(class Component* component){
    //find the insertion point in sorted vector
    //the first element with the order higher than me
    int myOrder = component->GetUpdateOrder();
    auto iter = mComponents.begin();
    for(;iter != mComponents.end();++iter){
        if(myOrder < (*iter)->GetUpdateOrder()){
            break;
        }
    }
    //insert elements before the position of iterator
    mComponents.insert(iter,component);
}
void Actor::RemoveComponent(class Component* component){
auto iter = std::find(mComponents.begin(),mComponents.end(),component);
if(iter != mComponents.end()){
    mComponents.erase(iter);
}
}
void Actor::ProcessInput(const struct InputState& state)
{
    if(mState== EActive){
        for(auto comp:mComponents){
            comp->ProcessInput(state);
        }
        ActorInput(state);
    }
}
void Actor::ActorInput(const struct InputState& state){
     }
void Actor::ComputeWorldTransform(){
    if(mRecomputeWorldTransform)
    {
        mRecomputeWorldTransform = false;

        //scale,then rotate ,the  translate
        mWorldTransform = Matrix4::CreateScale(mScale);
        mWorldTransform *= Matrix4::CreateRotationZ(mRotation);
        mWorldTransform *= Matrix4::CreateTranslation(Vector3(mPosition.x,mPosition.y,0.0f));

        for(auto comp:mComponents){
            comp->OnUpdateWorldTransform();
        }
    }
}
