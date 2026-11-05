#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED

#pragma once
#include <Vector>
#include "Math.h"
#include <algorithm>
#include <SDL2/SDL.h>
#include <cstdint>
#include "InputComponent.h"
class Actor{
public:
    enum State{
        EActive,
        Epaused,
        EDead
    };

    //construtor(dependency injection)pattern where the const receives the pointer to a game class instead of singleton
    //to create another actor or access any other required game funtion
    Actor(class Game* game);
    virtual ~Actor();

    //update function called from game(no override
    void Update(float deltaTime);
    //update the componets attacted to actor no overriden
    void UpdateComponets(float deltaTime);
    //any actor - specific update code(overridable)
    virtual void UpdateActor(float deltaTime);

    //movement
    //ProcessInput called from game not overriden
     void ProcessInput(const struct InputState& state);

     //any actor specific input code(overridable)
     virtual void ActorInput(const struct InputState& state);

    //getters and helpers
    const Vector2&  GetPosition()const{return  mPosition;}
    void SetPosition(const Vector2& pos){mPosition = pos; mRecomputeWorldTransform = true;}
    float GetScale()const{return mScale;}
    void SetScale(float scale){mScale = scale; mRecomputeWorldTransform = true;}
    Vector2 GetForward()const{return Vector2(Math::Cos(mRotation),Math::Sin(mRotation));}
    float GetRotation()const{ return mRotation;}
    void SetRotation(float Rotation){mRotation = Rotation; mRecomputeWorldTransform=true;}
    State GetState()const{return mState;}
    void SetState(State state){mState = state;}
    void ComputeWorldTransform();
    const Matrix4& GetWorldTransform()const {return mWorldTransform;}

    class Game* GetGame(){ return mGame;};
    //add and remove
    void AddComponent(class Component* component);
    void RemoveComponent(class Component* component);

private:
    //state of the actor
    State mState;
    //transform
    float mforward;
    Matrix4 mWorldTransform;
    bool mRecomputeWorldTransform;
    Vector2 mPosition;//center position of actor
    float mScale;//uniform scale of actor 1.0 for 100%
    float mRotation;//rotation angle(radians)
    //componets held by this actor
    std::vector<class Component*> mComponents;
    class Game*  mGame;


};


#endif // ACTOR_H_INCLUDED
