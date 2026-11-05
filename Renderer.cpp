#include "Renderer.h"
#include "Texture.h"
#include <algorithm>
#include "Shader.h"
#include "VertexArray.h"
#include "SpriteComponent.h"
#include <GL/glew.h>

Renderer::Renderer( Game * game)
:mGame(game)
,mSpriteShader(nullptr)
{

}
Renderer::~Renderer()
{

}

bool Renderer::Initialize(float ScreenWidth,float ScreenHeight)
{
    mScreenWidth = ScreenWidth;
    mScreenHeight = ScreenHeight;
    //use the core openGL PROFILE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    //specify version 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
    //request a color buffer with 8-bits per RGBA channel
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
    //enable double bufferring
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    //force opengl to use hardware acceleration(GPU)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,1);

    mWindow = SDL_CreateWindow(
    "Game",
    100,
    100,
    static_cast<int>(mScreenWidth),
    static_cast<int>(mScreenHeight),
    SDL_WINDOW_OPENGL
    );
    if(!mWindow){
        SDL_Log("unable to create the window: %s",SDL_GetError());
        return false;
    }
    mContext = SDL_GL_CreateContext(mWindow);
        if(!mContext){
        SDL_Log("unable to create the GLcontext: %s",SDL_GetError());
        return false;
    }
    //initialize GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }
        //on some platform ,Glew will emit a benign error code , so clear it
    glGetError();
    	// Make sure we can create/compile shaders
	if (!LoadShaders())
	{
		SDL_Log("Failed to load shaders.");
		return false;
	}

    //create quad for drawing sprites
     CreateSpriteVerts();
     return true;
}
void Renderer::Shutdown()
{
    delete mSpriteVerts;
    mSpriteShader->Unload();
    delete mSpriteShader;
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);

}
void Renderer::UnloadData()
{
    	// Destroy textures
	for (auto i : mTextures)
	{
		i.second->Unload();
		delete i.second;
	}
	mTextures.clear();
}

void Renderer::Draw()
{


    //set tghe clear color to gray
    glClearColor(0.86f,0.86f,0.86f,1.0f);
    //clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable alpha blending on the color buffer
    glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //todo: draw scene
    mSpriteShader->SetActive();
    mSpriteVerts->SetActive();

    mSpriteShader->SetMatrixUniform("uViewProj", mView);

    for(auto sprite:mSprites){
        sprite->Draw(mSpriteShader);
    }
    //swap the buffers ,which also displays the scene
    //SDL_GL_SwapWindow(mWindow);

}

void Renderer::AddSprite( SpriteComponent* sprite)
{
         //find the insertion point in the soted vector
     //(the first element with a higher draw order than me)
     int myDraworder = sprite->GetDrawOrder();
     auto iter = mSprites.begin();
     for(;iter!= mSprites.end();++iter){
        if(myDraworder < (*iter)->GetDrawOrder()){
            break;
        }
     }
     //inserts element before postion of iterator
     mSprites.insert(iter,sprite);
}
void Renderer::RemoveSprite( SpriteComponent* sprite)
{
        	// (We can't swap because it ruins ordering)
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	mSprites.erase(iter);
}
 Texture* Renderer::GetTexture(const std::string& fileName)
{
    Texture* tex = nullptr;
//is the texture in map already?
auto iter = mTextures.find(fileName);
if(iter != mTextures.end()){
    tex = iter->second;
}else{
    tex = new Texture();
    if(tex->Load(fileName))
    {
        mTextures.emplace(fileName,tex);
    }else
    {
        delete tex;
        tex = nullptr;
    }

}
return tex;
};
void Renderer::CreateSpriteVerts()
{
        float vertices [] = {
        -0.5f,0.5f,0.f,0.f,0.f,//top left
        0.5f,0.5f,0.f,1.f,0.f,//top right
         0.5f, -0.5f, 0.f, 1.f, 1.f, // bottom right
		-0.5f, -0.5f, 0.f, 0.f, 1.f  // bottom left

    };
    unsigned int  indices[] = {
    0,1,2,
    2,3,0
    };
    mSpriteVerts = new VertexArray(vertices,4,indices,6);
}
    bool Renderer::LoadShaders()
    {
       mSpriteShader = new Shader();
    if(!mSpriteShader->Load("Shaders/Sprite.vert","Shaders/Sprite.frag"))
    {
        SDL_Log("Failed to Load shader",SDL_GetError());
        return false;
    }
    mSpriteShader->SetActive();

    // AFTER:
UpdateProjection();   // just call the helper so there's one source of truth
    return true;
    }
void Renderer::UpdateProjection()
{
        mSpriteShader->SetActive();

    float w = mScreenWidth  / mZoom;
    float h = mScreenHeight / mZoom;

    // Build view+proj in one matrix, baking camera offset directly into it
    // This matches how CreateSimpleViewProj works internally but with camera offset
    Matrix4 proj = Matrix4::CreateSimpleViewProj(w, h);

    // Scale the camera offset to NDC space (same scale CreateSimpleViewProj uses)
    Matrix4 cam = Matrix4::CreateTranslation(
        Vector3(
            -mCameraPos.x * (2.0f / w),
             -mCameraPos.y * (2.0f / h),
             0.0f
        )
    );

    mView = proj * cam;
    mSpriteShader->SetMatrixUniform("uViewProj", mView);

}
