#include "HUDManager.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

HUDManager::HUDManager() {}
HUDManager::~HUDManager() {}

//
//  Build screen-space orthographic projection.
//  Top-left of screen = (0,0); bottom-right = (screenW, screenH).
//  Stored as 16-float column-major (GL convention).
//
//  Standard ortho matrix (with rows: x→clip_x, y→clip_y):
//      x_clip = (2/W)*x_pixel - 1
//      y_clip = 1 - (2/H)*y_pixel        ← flip Y so 0 is at top
//
static void BuildOrthoMatrix(float* out, int screenW, int screenH)
{
    float W = static_cast<float>(screenW);
    float H = static_cast<float>(screenH);

    // Column-major:
    // [ 2/W   0    0  -1 ]
    // [  0  -2/H   0   1 ]
    // [  0   0    1   0  ]
    // [  0   0    0   1  ]
    out[0] =  2.f/W;  out[4] =  0.f;     out[8]  = 0.f;  out[12] = -1.f;
    out[1] =  0.f;    out[5] = -2.f/H;   out[9]  = 0.f;  out[13] =  1.f;
    out[2] =  0.f;    out[6] =  0.f;     out[10] = 1.f;  out[14] =  0.f;
    out[3] =  0.f;    out[7] =  0.f;     out[11] = 0.f;  out[15] =  1.f;
}

static float gOrtho[16];   // populated in Initialize

//
bool HUDManager::Initialize(SDL_Window* window, int screenW, int screenH)
{
    mWindow  = window;
    mScreenW = screenW;
    mScreenH = screenH;

    if (TTF_Init() != 0)
    {
        SDL_Log("HUDManager: TTF_Init failed: %s", TTF_GetError());
        return false;
    }

    if (!LoadShaders())
    {
        SDL_Log("HUDManager: failed to load shaders");
        TTF_Quit();
        return false;
    }

    CreateQuad();
    CreateWhiteTexture();

    mFont    = LoadFont(16);
    mFontSm  = LoadFont(13);
    mFontBig = LoadFont(40);
    if (!mFont) SDL_Log("HUDManager: no system font found.");

    BuildOrthoMatrix(gOrtho, screenW, screenH);

    SDL_Log("HUDManager: initialised (%dx%d)", screenW, screenH);
    mReady = true;
    return true;
}

void HUDManager::Shutdown()
{
    for (auto& kv : mTextCache)
        if (kv.second.texID) glDeleteTextures(1, &kv.second.texID);
    mTextCache.clear();

    if (mWhiteTex) glDeleteTextures(1, &mWhiteTex);

    if (mFontBig) { TTF_CloseFont(mFontBig); mFontBig = nullptr; }
    if (mFont)    { TTF_CloseFont(mFont);    mFont    = nullptr; }
    if (mFontSm)  { TTF_CloseFont(mFontSm);  mFontSm  = nullptr; }

    delete mQuad; mQuad = nullptr;
    if (mShader) { mShader->Unload(); delete mShader; mShader = nullptr; }

    TTF_Quit();
    mReady = false;
}

//
bool HUDManager::LoadShaders()
{
    mShader = new Shader();
    if (!mShader->Load("Shaders/HUDShader.vert", "Shaders/HUDShader.frag"))
        return false;
    return true;
}

void HUDManager::CreateQuad()
{
    float vertices[] = {
        // Position           UV
        // inPos.y = +0.5 → bottom of screen rectangle → sample bottom of texture (V=1)
        // inPos.y = -0.5 → top of screen rectangle    → sample top of texture (V=0)
        -0.5f,  0.5f, 0.f,  0.f, 1.f,   // left + bottom of rect, sample left+bottom of tex
         0.5f,  0.5f, 0.f,  1.f, 1.f,   // right + bottom of rect, sample right+bottom
         0.5f, -0.5f, 0.f,  1.f, 0.f,   // right + top of rect, sample right+top
        -0.5f, -0.5f, 0.f,  0.f, 0.f    // left + top of rect, sample left+top
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    mQuad = new VertexArray(vertices, 4, indices, 6);
}

void HUDManager::CreateWhiteTexture()
{
    unsigned char pixel[4] = { 255, 255, 255, 255 };
    glGenTextures(1, &mWhiteTex);
    glBindTexture(GL_TEXTURE_2D, mWhiteTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

TTF_Font* HUDManager::LoadFont(int size)
{
    const char* paths[] = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
        "C:/Windows/Fonts/arial.ttf",
        nullptr
    };
    for (int i = 0; paths[i]; ++i)
    {
        TTF_Font* f = TTF_OpenFont(paths[i], size);
        if (f) return f;
    }
    return nullptr;
}

//
//  Raw GL helpers — bypassing Shader class for uniforms it doesn't expose
//
static GLint GetShaderProgramID()
{
    GLint id = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);
    return id;
}

void HUDManager::SetTintUniform(const HUDColor& col)
{
    GLint loc = glGetUniformLocation(GetShaderProgramID(), "uTint");
    if (loc != -1) glUniform4f(loc, col.r, col.g, col.b, col.a);
}

void HUDManager::SetOrthoUniform()
{
    GLint loc = glGetUniformLocation(GetShaderProgramID(), "uOrtho");
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, gOrtho);
}


HUDManager::CachedText HUDManager::GetOrCreateText(
    const std::string& text, const HUDColor& col, TTF_Font* font)
{
    if (!font || text.empty()) return {};

    std::ostringstream keyss;
    keyss << TTF_FontHeight(font) << "_"
          << int(col.r*255) << "_" << int(col.g*255) << "_" << int(col.b*255)
          << "_" << text;
    std::string key = keyss.str();

    auto it = mTextCache.find(key);
    if (it != mTextCache.end()) return it->second;

    SDL_Color sdlCol = {
        static_cast<Uint8>(col.r * 255),
        static_cast<Uint8>(col.g * 255),
        static_cast<Uint8>(col.b * 255),
        255
    };
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), sdlCol);
    if (!surf) return {};

    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(surf);
    if (!rgba) return {};

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba->w, rgba->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    CachedText ct{ texID, rgba->w, rgba->h };
    SDL_FreeSurface(rgba);

    // Cache size limit
    if (mTextCache.size() > 500)
    {
        auto it2 = mTextCache.begin();
        for (int i = 0; i < 250 && it2 != mTextCache.end(); ++i)
        {
            if (it2->second.texID) glDeleteTextures(1, &it2->second.texID);
            it2 = mTextCache.erase(it2);
        }
    }

    mTextCache[key] = ct;
    return ct;
}

//
//  Drawing primitives
//
void HUDManager::DrawRect(float x, float y, float w, float h, const HUDColor& col)
{
    glBindTexture(GL_TEXTURE_2D, mWhiteTex);
    mShader->SetVector2Uniform("uPosition", Vector2(x, y));
    mShader->SetVector2Uniform("uSize",     Vector2(w, h));
    SetTintUniform(col);
    mShader->SetIntUniform("uUseTexture", 1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void HUDManager::DrawText(const std::string& txt, float x, float y,
                          const HUDColor& col, TTF_Font* font)
{
    TTF_Font* f = font ? font : mFont;
    if (!f) return;

    CachedText ct = GetOrCreateText(txt, col, f);
    if (ct.texID == 0) return;

    glBindTexture(GL_TEXTURE_2D, ct.texID);
    mShader->SetVector2Uniform("uPosition", Vector2(x, y));
    mShader->SetVector2Uniform("uSize",
        Vector2(static_cast<float>(ct.w), static_cast<float>(ct.h)));
    SetTintUniform(HUDColor(1.f, 1.f, 1.f, 1.f));   // text is pre-coloured
    mShader->SetIntUniform("uUseTexture", 1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void HUDManager::DrawBar(float x, float y, float w, float h,
                         int value, int maxVal, const HUDColor& fill)
{
    // Track
    DrawRect(x, y, w, h, HUDColor(30/255.f, 26/255.f, 18/255.f, 0.85f));

    int filled = (maxVal > 0)
        ? std::min(static_cast<int>(w) - 2,
                   value * (static_cast<int>(w) - 2) / maxVal) : 0;
    if (filled > 0)
        DrawRect(x + 1, y + 1, static_cast<float>(filled), h - 2, fill);
}

int HUDManager::StatRow(const std::string& label,
                         int value, int maxVal, const HUDColor& barCol,
                         int x, int y, int width)
{
    TTF_Font* sf = mFontSm ? mFontSm : mFont;
    DrawText(label, static_cast<float>(x), static_cast<float>(y), White(), sf);

    int barX = x + LABEL_W;
    int barW = width - LABEL_W - 50;
    DrawBar(static_cast<float>(barX), static_cast<float>(y + 2),
            static_cast<float>(barW), static_cast<float>(STATBAR_H),
            value, maxVal, barCol);

    std::string valStr = std::to_string(value) + "/" + std::to_string(maxVal);
    DrawText(valStr, static_cast<float>(barX + barW + 6),
             static_cast<float>(y), Dim(), sf);

    return y + ROW_H;
}

//
//  Flash, banners
//
void HUDManager::DrawFlash(const HUDData& hud)
{
    if (hud.flashAlpha <= 0.01f) return;

    HUDColor c;
    if      (hud.flashOutcome == "WON")     c = Green();
    else if (hud.flashOutcome == "TIME UP") c = Yellow();
    else                                     c = Red();
    c.a = hud.flashAlpha * 0.7f;

    DrawRect(0, 0,
             static_cast<float>(mScreenW),
             static_cast<float>(mScreenH), c);
}

void HUDManager::DrawPermanentEpisodeBanner(const HUDData& hud)
{
    if (!mFont) return;

    const float BX = 10, BY = 10, BW = 380, BH = 70;

    DrawRect(BX, BY, BW, BH, HUDColor(15/255.f, 12/255.f, 8/255.f, 0.92f));

    HUDColor border = Yellow();
    DrawRect(BX, BY, BW, 2, border);
    DrawRect(BX, BY + BH - 2, BW, 2, border);
    DrawRect(BX, BY, 2, BH, border);
    DrawRect(BX + BW - 2, BY, 2, BH, border);

    {
        std::ostringstream ss;
        ss << "EPISODE " << hud.episode;
        DrawText(ss.str(), BX + 12, BY + 8, Yellow(), mFont);
    }

    TTF_Font* sf = mFontSm ? mFontSm : mFont;
    if (hud.lastEpisode < 0)
    {
        DrawText("Last: (first episode)", BX + 12, BY + 38, Dim(), sf);
    }
    else
    {
        std::ostringstream ss;
        ss << "Last: " << hud.lastOutcome
           << "  (R: " << std::fixed << std::setprecision(1)
           << hud.lastReward << ")";

        HUDColor col;
        if      (hud.lastOutcome == "WON")     col = Green();
        else if (hud.lastOutcome == "TIME UP") col = Yellow();
        else                                    col = Red();

        DrawText(ss.str(), BX + 12, BY + 38, col, sf);
    }
}

void HUDManager::DrawBetweenEpisodesBanner(const HUDData& hud)
{
    if (!hud.betweenEpisodes || !mFontBig) return;

    DrawRect(0, 0,
             static_cast<float>(mScreenW),
             static_cast<float>(mScreenH),
             HUDColor(0, 0, 0, 0.40f));

    std::ostringstream big;
    big << "NEW EPISODE " << hud.episode;

    int tw = 0, th = 0;
    TTF_SizeText(mFontBig, big.str().c_str(), &tw, &th);
    float bx = (mScreenW - tw) * 0.5f;
    float by = mScreenH / 3.0f;
    DrawText(big.str(), bx, by, Yellow(), mFontBig);

    std::ostringstream sub;
    sub << "Previous: " << hud.lastOutcome
        << "   Reward: " << std::fixed << std::setprecision(1) << hud.lastReward;
    int sw = 0, sh = 0;
    TTF_SizeText(mFont, sub.str().c_str(), &sw, &sh);
    DrawText(sub.str(), (mScreenW - sw) * 0.5f, by + th + 10, White(), mFont);
}

void HUDManager::AddLog(const std::string& message)
{
    mLog.push_back(message);
    if ((int)mLog.size() > MAX_LOG) mLog.pop_front();
}

//
//  Main Draw
//
void HUDManager::Draw(const HUDData& hud)
{
    if (!mReady || !mShader || !mQuad) return;

    // Save GL state
    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Activate HUD shader & quad
    mShader->SetActive();
    SetOrthoUniform();
    mQuad->SetActive();

    DrawFlash(hud);
    DrawPermanentEpisodeBanner(hud);

    TTF_Font* sf = mFontSm ? mFontSm : mFont;
    int barTop = mScreenH - BAR_H;

    DrawRect(0, static_cast<float>(barTop),
             static_cast<float>(mScreenW), static_cast<float>(BAR_H),
             HUDColor(10/255.f, 8/255.f, 5/255.f, 0.85f));
    DrawRect(0, static_cast<float>(barTop),
             static_cast<float>(mScreenW), 2.f, Yellow());

    int col1W = 280;
    int col2W = 360;
    int col3W = mScreenW - col1W - col2W - PAD * 4;

    int col1X = PAD;
    int col2X = col1X + col1W + PAD;
    int col3X = col2X + col2W + PAD;

    int y = barTop + 8;
    DrawText("STUDENT AGENT:Q-Learning + FSM",
             static_cast<float>(col1X), static_cast<float>(y), Yellow(), mFont);
    y += ROW_H;

    {
        std::ostringstream ss;
        ss << "Episode: " << hud.episode
           << "    Tick: " << hud.epTicks << "/5000";
        DrawText(ss.str(), static_cast<float>(col1X), static_cast<float>(y), Cyan(), sf);
        y += ROW_H;
    }
    {
        std::ostringstream ss;
        ss << "Time: " << hud.timeOfDay
           << "    Eps: " << std::fixed << std::setprecision(3) << hud.epsilon;
        DrawText(ss.str(), static_cast<float>(col1X), static_cast<float>(y), Cyan(), sf);
        y += ROW_H;
    }
    {
        std::ostringstream ss;
        ss << "Reward: " << std::fixed << std::setprecision(1) << hud.reward
           << "   Util: " << std::setprecision(2) << hud.utility;
        DrawText(ss.str(), static_cast<float>(col1X), static_cast<float>(y), White(), sf);
        y += ROW_H;
    }
    {
        DrawText("Location: ", static_cast<float>(col1X), static_cast<float>(y), Cyan(), sf);
        int pw = 0, ph = 0;
        if (sf) TTF_SizeText(sf, "Location: ", &pw, &ph);
        DrawText(hud.location, static_cast<float>(col1X + pw),
                 static_cast<float>(y), Green(), sf);
        y += ROW_H;
    }

    if (hud.lastEpisode >= 0)
    {
        std::ostringstream ss;
        ss << "Last Ep " << hud.lastEpisode << ": "
           << hud.lastOutcome
           << "  R:" << std::fixed << std::setprecision(1) << hud.lastReward;

        HUDColor col;
        if      (hud.lastOutcome == "WON")     col = Green();
        else if (hud.lastOutcome == "TIME UP") col = Yellow();
        else                                    col = Red();
        DrawText(ss.str(), static_cast<float>(col1X), static_cast<float>(y), col, sf);
    }

    // COL 2 — stat bars
    int sy = barTop + 8;
    int half = col2W / 2 - 4;

    HUDColor enCol = (hud.energy <= 1) ? Red() : Green();
    HUDColor huCol = (hud.hunger >= 4) ? Red() : Yellow();
    HUDColor stCol = (hud.stress >= 5) ? Red() : Magenta();
    HUDColor moCol = (hud.money  <= 0) ? Red() : Yellow();
    HUDColor prCol = Cyan();

    int ly = sy;
    ly = StatRow("Energy", hud.energy,   hud.maxEnergy,   enCol, col2X,        ly, half);
    ly = StatRow("Hunger", hud.hunger,   hud.maxHunger,   huCol, col2X,        ly, half);
    ly = StatRow("Stress", hud.stress,   hud.maxStress,   stCol, col2X,        ly, half);

    int ry = sy;
    ry = StatRow("Money",    hud.money,    hud.maxMoney,    moCol, col2X + half, ry, half);
    ry = StatRow("Progress", hud.progress, hud.maxProgress, prCol, col2X + half, ry, half);

    // COL 3 — log
    int ly2 = barTop + 8;
    DrawText("Dialogue Log", static_cast<float>(col3X), static_cast<float>(ly2), Yellow(), sf);
    ly2 += ROW_H - 2;

    int startIdx = std::max(0, (int)hud.log.size() - SHOW_LOG);
    int total    = (int)hud.log.size();

    for (int i = startIdx; i < total; ++i)
    {
        int age = total - 1 - i;
        HUDColor lineCol;
        if      (age == 0) lineCol = White();
        else if (age == 1) lineCol = HUDColor(200/255.f, 185/255.f, 130/255.f, 1.f);
        else               lineCol = Dim();

        std::string line = hud.log[i];
        size_t maxChars = static_cast<size_t>(col3W / 7);
        if (line.length() > maxChars)
            line = line.substr(0, maxChars - 3) + "...";

        DrawText(line, static_cast<float>(col3X), static_cast<float>(ly2),
                 lineCol, sf);
        ly2 += 18;
    }

    DrawBetweenEpisodesBanner(hud);

    // Restore GL state
    if (!blendWasEnabled) glDisable(GL_BLEND);
    if (depthWasEnabled)  glEnable(GL_DEPTH_TEST);
}
