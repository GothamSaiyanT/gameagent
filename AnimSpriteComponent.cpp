#include "AnimSpriteComponent.h"
#include "Math.h"
#include "Texture.h"
#include "MoveComponent.h"

AnimSpriteComponent::AnimSpriteComponent( Actor* owner,int draOrder )
:SpriteComponent(owner,draOrder)
,mCurrframe(0.0f)
,mAnimFPS(24.0f)
,mCurrentAnim("")
{
}
//update animation every frame(overriden from component)
void AnimSpriteComponent::Update(float deltaTime){
    SpriteComponent::Update(deltaTime);
        if(!mCurrentAnim.empty() && mAnimation.count(mCurrentAnim) > 0){
                auto& animData = mAnimation[mCurrentAnim];
                const auto& frame = animData.textures;
        if(frame.size()>0){
            mCurrframe += mAnimFPS * deltaTime;
                // Handle looping/non-looping animations
        if (mCurrframe >= frame.size()) {
            if (animData.looping) {
                mCurrframe -= frame.size();
            } else {
                mCurrframe = static_cast<float>(frame.size() - 1); // Stop at last frame
            }
        }
        SetTexture(frame[static_cast<int>(mCurrframe)]);

        }
    }


}
   void AnimSpriteComponent::SetAnimTextures(const std::string& animeName,const std::vector<Texture*>& textures,bool looping){

        mAnimation[animeName] = {textures,looping};
        if(mCurrentAnim.empty()){
            SetAnimation(animeName);
        }
       }
void AnimSpriteComponent::SetAnimation(const std::string& Name){

    if(mCurrentAnim == Name) return; // Don't reset frame every time
    if(mAnimation.count(Name) == 0) return;

    mCurrentAnim = Name;
    mCurrframe = 0.0f;
    if(!mAnimation[mCurrentAnim].textures.empty()) {
        SetTexture(mAnimation[mCurrentAnim].textures[0]);
    }
}
void AnimSpriteComponent::UpdateAnimation(CharacterState state)
{
    switch(state)
    {
    case CharacterState::Idle:
        SetAnimation("Idle");
        break;
    case CharacterState::Walking:
        SetAnimation("Walk");
        break;
    case CharacterState::Jumping:
        SetAnimation("Jump");
        break;
    }
}
