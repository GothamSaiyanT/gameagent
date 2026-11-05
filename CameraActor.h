#ifndef CAMERAACTOR_H_INCLUDED
#define CAMERAACTOR_H_INCLUDED

#pragma once
#include "Component.h"
#include "Math.h"

class CameraActor : public Component
{
public:
    CameraActor(class Actor* owner, int updateOrder = 10); // Higher priority than default

    void Update(float deltaTime) override;
    void OnUpdateWorldTransform() override;

    // Camera properties
    void SetViewSize(float width, float height);
    void SetZoom(float zoom);
    float GetZoom() const { return mZoom; }

    // Get the view matrix for rendering
    const Matrix4& GetViewMatrix() const { return mViewMatrix; }
    const Matrix4& GetProjectionMatrix() const { return mProjectionMatrix; }

private:
    // Camera configuration
    float mViewWidth;
    float mViewHeight;
    float mZoom; // 1.0 = normal, >1.0 = zoom in, <1.0 = zoom out

    // Matrices for rendering
    Matrix4 mViewMatrix;
    Matrix4 mProjectionMatrix;

    // Recalculate matrices when needed
    void CalculateViewMatrix();
    void CalculateProjectionMatrix();
};

#endif // CAMERAACTOR_H_INCLUDED
