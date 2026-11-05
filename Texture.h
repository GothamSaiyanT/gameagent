#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <string>

class Texture{
public:
    Texture();
    ~Texture();

    bool Load(const std::string& fileName);
    void Unload();
    void SetActive();

    int GetWidth()const{return mWidth;}
    int GetHeight()const{return mHeight;}


private:
    //openGL ID of this texture
    unsigned int mTextureID;

    //width/height
    int mWidth;
    int mHeight;

};

#endif // TEXTURE_H_INCLUDED
