#include "SpriteComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Renderer.h"
SpriteComponent::SpriteComponent( Actor* owner,int drawOrder )
:mDrawOrder(drawOrder)
,Component(owner)
,mTexture(nullptr)
,mTexWidth(0)
,mTexHeight(0)

{
    mOwner->GetGame()->GetRenderer()->AddSprite(this);
}
SpriteComponent::~SpriteComponent(){
    mOwner->GetGame()->GetRenderer()->RemoveSprite(this);
}
void SpriteComponent::SetTexture(Texture* texture){
    mTexture = texture;
    //get width/height of texture
    mTexWidth = texture->GetWidth();
    mTexHeight = texture->GetHeight();

}
void SpriteComponent::Draw(Shader* shader){
    if(!mVisible) return;
    if(!mTexture) return;
    float scaleX = static_cast<float>(mTexWidth);
    float scaleY = static_cast<float>(mTexHeight);

    if (mFlipX)
        scaleX = -scaleX; // flip horizontally

        Matrix4 scaleMat = Matrix4::CreateScale(scaleX,scaleY,1.0f);

        Matrix4 world = scaleMat * mOwner->GetWorldTransform();

        //since all sprites use the same shader/vertices
        //the game sets them active first before any sprite draws
        //set the world transform
        shader->SetMatrixUniform("uWorldTransform",world);
        shader->SetIntUniform("uUseUVTransform", 0);

        //set the current texture
        mTexture->SetActive();
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,nullptr);



    }

