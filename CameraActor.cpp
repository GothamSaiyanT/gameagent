#include "CameraActor.h"
#include "Actor.h"

CameraActor::CameraActor(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mViewWidth(1024.0f)
    , mViewHeight(768.0f)
    , mZoom(1.0f)
{
    CalculateProjectionMatrix();
    CalculateViewMatrix();
}

void CameraActor::Update(float deltaTime)
{
    // Always make sure view matrix is up to date
    CalculateViewMatrix();
}

void CameraActor::OnUpdateWorldTransform()
{
    // Recalculate view matrix when owner's transform changes
    CalculateViewMatrix();
}

void CameraActor::SetViewSize(float width, float height)
{
    mViewWidth = width;
    mViewHeight = height;
    CalculateProjectionMatrix();
}

void CameraActor::SetZoom(float zoom)
{
    mZoom = zoom;
    CalculateProjectionMatrix();
}

void CameraActor::CalculateViewMatrix()
{
    // Get owner's position and orientation
    Vector2 position = mOwner->GetPosition();
    float rotation = mOwner->GetRotation();

    // Create view matrix (inverse of camera's transform)
    // For 2D, we can simplify this compared to 3D
    Matrix4 translation = Matrix4::CreateTranslation(
        Vector3(-position.x, -position.y, 0.0f));
    Matrix4 rotationMat = Matrix4::CreateRotationZ(-rotation);

    mViewMatrix = rotationMat * translation;
}

void CameraActor::CalculateProjectionMatrix()
{
    // Simple orthographic projection for 2D
    float halfWidth = (mViewWidth / 2.0f) / mZoom;
    float halfHeight = (mViewHeight / 2.0f) / mZoom;

    mProjectionMatrix = Matrix4::CreateOrtho(
        -halfWidth, halfWidth,
        -halfHeight, halfHeight/*,
        0.0f, 100.0f*/); // Near and far planes (not as important in 2D)
}
