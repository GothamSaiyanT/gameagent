#pragma once
#ifndef HUDMANAGER_H
#define HUDMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include "Shader.h"
#include "VertexArray.h"
#include "Math.h"
#include <string>
#include <deque>
#include <unordered_map>

//
//  HUDData  —  filled by Game::GenerateOutput each frame
//
struct HUDData
{
    int    episode    = 0;
    int    totalTicks = 0;
    int    epTicks    = 0;
    double epsilon    = 1.0;
    float  reward     = 0.0f;
    float  utility    = 0.0f;
    bool   paused     = false;

    std::string location  = "AtHome";
    std::string timeOfDay = "Morning";

    int energy   = 6; int maxEnergy   = 6;
    int hunger   = 0; int maxHunger   = 5;
    int stress   = 0; int maxStress   = 6;
    int money    = 3; int maxMoney    = 5;
    int progress = 0; int maxProgress = 5;

    int         lastEpisode = -1;
    float       lastReward  = 0.f;
    std::string lastOutcome = "None";

    float       flashAlpha   = 0.f;
    std::string flashOutcome = "None";

    bool        betweenEpisodes = false;

    std::deque<std::string> log;
};

//
//  Simple RGBA colour (HUD-internal, no Math.h dependency)
//
struct HUDColor
{
    float r, g, b, a;
    HUDColor() : r(1), g(1), b(1), a(1) {}
    HUDColor(float R, float G, float B, float A = 1.f)
        : r(R), g(G), b(B), a(A) {}
};

//
//  HUDManager  —  OpenGL-based HUD overlay.
//
//  Renders through the same GL context as the world, so it survives buffer
//  swaps and never conflicts with SDL_Renderer.
//
class HUDManager
{
public:
    HUDManager();
    ~HUDManager();

    bool Initialize(SDL_Window* window, int screenW, int screenH);
    void Shutdown();
    void Draw(const HUDData& hud);

    void AddLog(const std::string& message);
    const std::deque<std::string>& GetLog() const { return mLog; }

    bool IsReady() const { return mReady; }

private:
    SDL_Window*  mWindow   = nullptr;
    Shader*      mShader   = nullptr;
    VertexArray* mQuad     = nullptr;
    GLuint       mWhiteTex = 0;

    int mScreenW = 0;
    int mScreenH = 0;
    bool mReady  = false;

    TTF_Font* mFontBig = nullptr;
    TTF_Font* mFont    = nullptr;
    TTF_Font* mFontSm  = nullptr;

    struct CachedText { GLuint texID = 0; int w = 0; int h = 0; };
    std::unordered_map<std::string, CachedText> mTextCache;

    std::deque<std::string> mLog;
    static const int MAX_LOG  = 60;
    static const int SHOW_LOG = 4;

    static const int BAR_H     = 200;
    static const int PAD       = 10;
    static const int ROW_H     = 20;
    static const int STATBAR_H = 14;
    static const int LABEL_W   = 75;

    static HUDColor Yellow()  { return {220/255.f, 180/255.f,  55/255.f, 1.f}; }
    static HUDColor Cyan()    { return { 65/255.f, 195/255.f, 210/255.f, 1.f}; }
    static HUDColor Red()     { return {215/255.f,  65/255.f,  55/255.f, 1.f}; }
    static HUDColor Green()   { return { 70/255.f, 195/255.f,  90/255.f, 1.f}; }
    static HUDColor White()   { return {235/255.f, 215/255.f, 150/255.f, 1.f}; }
    static HUDColor Dim()     { return {110/255.f, 100/255.f,  70/255.f, 1.f}; }
    static HUDColor Magenta() { return {185/255.f, 100/255.f, 180/255.f, 1.f}; }

    void DrawRect(float x, float y, float w, float h, const HUDColor& col);
    void DrawText(const std::string& txt, float x, float y,
                  const HUDColor& col, TTF_Font* font = nullptr);
    void DrawBar (float x, float y, float w, float h,
                  int value, int maxVal, const HUDColor& fill);

    int  StatRow(const std::string& label,
                 int value, int maxVal, const HUDColor& barCol,
                 int x, int y, int width);

    void DrawFlash(const HUDData& hud);
    void DrawBetweenEpisodesBanner(const HUDData& hud);
    void DrawPermanentEpisodeBanner(const HUDData& hud);

    CachedText GetOrCreateText(const std::string& text,
                                const HUDColor& col,
                                TTF_Font* font);

    // Helpers to set uniforms not provided by your Shader class — uses raw GL
    void SetTintUniform(const HUDColor& col);
    void SetOrthoUniform();

    TTF_Font* LoadFont(int size);
    void CreateWhiteTexture();
    bool LoadShaders();
    void CreateQuad();
};

#endif
