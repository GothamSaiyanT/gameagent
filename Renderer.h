#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include "Math.h"
class Renderer
{
    public:
        Renderer(class Game * game);
        ~Renderer();

        bool Initialize(float ScreenWidth,float ScreenHeight);
        void Shutdown();
        void UnloadData();

        void Draw();

        void AddSprite(class SpriteComponent* sprite);
        void RemoveSprite(class SpriteComponent* sprite);

        class Texture* GetTexture(const std::string& fileName);

        void SetViewMatrix(const Matrix4& view) {mView = view;}
        float GetZoom() const { return mZoom; }
        void SetCameraPos(const Vector2& pos) { mCameraPos = pos; }
Vector2 GetCameraPos() const { return mCameraPos; }
void  SetZoom(float z)  { mZoom = z; }
void  UpdateProjection();    // call this whenever zoom changes
        float GetScreenWidth()const {return mScreenWidth;}
        float GetScreenHeight()const {return mScreenHeight;}
            // Needed by Game::GenerateOutput() for HUD swap + re-bind
    SDL_Window*   GetWindow()  const { return mWindow;  }
    SDL_GLContext GetContext() const { return mContext;  }

    private:
    void CreateSpriteVerts();
    bool LoadShaders();

    //Map of Textures Loaded
    std::unordered_map<std::string,class Texture* >mTextures;
    //all the sprite componets drawn
    std::vector<class SpriteComponent*> mSprites;

    //Game
    class Game* mGame;

    //sprite shader
    class Shader* mSpriteShader;

    //sprite vertex array
    class VertexArray* mSpriteVerts;

    //view/projection for shaders
    Matrix4 mView;
    Matrix4 mProjection;

    //width/height of screen
    float mScreenWidth;
    float mScreenHeight;

   Vector2 mCameraPos = Vector2(0.0f, 0.0f);

    float mZoom = 1.5f; //0.5f = 2x the map  and 1.0 normal

    //window
    SDL_Window * mWindow;

    //OpenGL context
    SDL_GLContext mContext;
};


#endif // RENDERER_H_INCLUDED
