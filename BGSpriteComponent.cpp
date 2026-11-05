#include "BGSpriteComponent.h"
#include "Actor.h"
#include "Texture.h"

BGSpriteComponent::BGSpriteComponent(class Actor* owner,int drawOrder)
:SpriteComponent(owner,drawOrder)
,mScrollSpeed(0.0f)
{
}
//update/draw overriden from parent
void BGSpriteComponent::Update(float deltaTime){
    SpriteComponent::Update(deltaTime);
    for(auto& bg:mBGTexture)
    {
        //update the x offset
        bg.mOffset.x += bg.mParallexSpeed*mScrollSpeed * deltaTime;
        //if thi is completelt off the screen,reset offset to
        //the right of the last bg tecture
        if(bg.mOffset.x < -mScreenSize.x)
        {
            bg.mOffset.x = (mBGTexture.size() - 1) * mScreenSize.x - 1;

        }
    }
}
void BGSpriteComponent::Draw(Shader* shader){
    for(auto& bg : mBGTexture){
        float width;
        float height;

        width = static_cast<int>(mScreenSize.x);
        height = static_cast<int>(mScreenSize.y);

            Matrix4 scaleMat = Matrix4::CreateScale(width,height,1.0f);
            Matrix4 transmat = Matrix4::CreateTranslation(Vector3( bg.mOffset.x, bg.mOffset.y,0.0f));

        Matrix4 world = scaleMat *transmat* mOwner->GetWorldTransform();
        shader->SetMatrixUniform("uWorldTransform",world);
        bg.mTexture->SetActive();
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,nullptr);
    }
}
void BGSpriteComponent::SetBGtextures(const std::vector<Texture*>& textures,const std::vector<float>& parallaxSpeeds){
    int count = 0;
        for(size_t i=0;i < textures.size();i++){
            BGTexture temp;
            temp.mTexture = textures[i];
            temp.mOffset.x = count * mScreenSize.x;
            temp.mOffset.y = 0;
            temp.mParallexSpeed = (i < parallaxSpeeds.size()) ? parallaxSpeeds[i] : 1.0f;

            mBGTexture.emplace_back(temp);
            count++;

                // Duplicate first texture at the end for smooth looping
    if (!mBGTexture.empty()) {
        BGTexture temp = mBGTexture.front();
        temp.mOffset.x = mBGTexture.size() * mScreenSize.x;
        mBGTexture.push_back(temp);
    }

        }
    }

