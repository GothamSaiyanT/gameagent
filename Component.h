#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include <cstdint>
#include "InputSystem.h"
class Component{
public:

    //constructor
    //the lower the update order ,the earlier the componet updates

    Component(class Actor* owner,int updateOrder = 100);
    //destructor
    virtual ~Component();

    //movemnt
    virtual void ProcessInput(const struct InputState& state){}

    //update this component by delta
    virtual void Update(float deltaTime);
    //called when world transform changes
    virtual void OnUpdateWorldTransform(){}
    int GetUpdateOrder()const {return mUpdateOrder;}

protected:
    class Actor* mOwner;
    //update order of component
    //allows certain components to update before or after other components
    int mUpdateOrder;
};


#endif // COMPONENT_H_INCLUDED
