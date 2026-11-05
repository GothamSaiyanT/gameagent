#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#pragma once
#include <GL/glew.h>
#include <string>
#include "Math.h"
class Shader
{
    public:
        Shader();
        ~Shader();
        //load the vertex/fragment shader with the given names
        bool Load(const std::string& vertName,
                  const std::string& fragName);
        //set this as the active shader program
        void SetActive();

        //sets the matrix transform
        void SetMatrixUniform(const char* name,const Matrix4& matrix);
         void Unload();
         void SetVector2Uniform(const char* name, const Vector2& value);
         void SetIntUniform(const char* name, int value);



    private:
        //tries to compile the specified file
        bool CompileShader(const std::string& fileName,
                           GLenum shaderType,GLuint& outShader);
        //test whether shader compiled successfully
        bool isCompiled(GLuint shader);
        //Teste whether vertex/fragment programs link
        bool isValidProgram();

        //store the shader object IDs
        GLuint mVertexShader;
        GLuint mFragShader;
        GLuint mShaderProgram;
};

#endif // SHADER_H_INCLUDED
