#include "Shader.h"
#include <SDL2/SDL.h>
#include <fstream>
#include <sstream>

Shader::Shader():
    mShaderProgram(0)
    ,mVertexShader(0)
    ,mFragShader(0)
{
}
Shader::~Shader(){
}
//load the vertex/fragment shader with the given names
bool Shader::Load(const std::string& vertName,
      const std::string& fragName){

          //compile vertex and fragment shaders
          if(!CompileShader(vertName,GL_VERTEX_SHADER,mVertexShader)||
             !CompileShader(fragName,GL_FRAGMENT_SHADER,mFragShader))
          {
              return false;
          }

          //create a shader program that links together the vertex /frag shaders
          mShaderProgram = glCreateProgram();
          glAttachShader(mShaderProgram,mVertexShader);
          glAttachShader(mShaderProgram,mFragShader);
          glLinkProgram(mShaderProgram);

          //verify that the program linked successfully
          if(!isValidProgram()){
            return false;
          }
          return true;
}
//set this as the active shader program
void Shader::SetActive(){
    glUseProgram(mShaderProgram);
}
    //tries to compile the specified file
bool Shader::CompileShader(const std::string& fileName,
               GLenum shaderType,GLuint& outShader)
{
    //open file
    std::ifstream shaderFile(fileName);
    if(shaderFile.is_open()){
        //read all the text buffers into ss then to string
        std::stringstream sstream;
        sstream<< shaderFile.rdbuf();
        std::string contents = sstream.str();
        const char* contentsChar = contents.c_str();

        //create a shader of the specified type
        outShader = glCreateShader(shaderType);
        //Set the source characters and try to compile
        glShaderSource(outShader,1,&(contentsChar),nullptr);
        glCompileShader(outShader);
        if (!isCompiled(outShader))
        {
        SDL_Log("Failed to compile shader %s", fileName.c_str());
        return false;
        }
    }
    else{
        SDL_Log("Shader file not found: %s", fileName.c_str());
        return false;
    }
    return true;

}
//test whether shader compiled successfully
bool Shader::isCompiled(GLuint shader){
    GLint status;
    //Query the compile status
    glGetShaderiv(shader,GL_COMPILE_STATUS,&status);
    if(status != GL_TRUE){

        char buffer[512];
        memset(buffer,0,512);
        glGetShaderInfoLog(shader,511,nullptr,buffer);
        SDL_Log("GLSL Compile Failed:\n%s",buffer);
        return false;
    }
    return true;
}
//Teste whether vertex/fragment programs link
bool Shader::isValidProgram(){
    	GLint status;
	// Query the link status
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
		SDL_Log("GLSL Link Status:\n%s", buffer);
		return false;
	}

	return true;
}
void Shader::Unload(){
    //delete shaders
    glDeleteProgram(mShaderProgram);
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragShader);
}
void Shader::SetMatrixUniform(const char* name,const Matrix4& matrix){
    //find the uniform by this name
    GLuint loc = glGetUniformLocation(mShaderProgram,name);

    //send the matrix daat tto the uniform
    glUniformMatrix4fv(loc,1,GL_TRUE,matrix.GetAsFloatPtr());
}
void Shader::SetVector2Uniform(const char* name, const Vector2& value)
{
    GLint loc = glGetUniformLocation(mShaderProgram,name);
    if (loc != -1)
    {
        glUniform2f(loc, value.x, value.y);
    }
}
void Shader::SetIntUniform(const char* name, int value)
{
    GLint location = glGetUniformLocation(mShaderProgram,name);
        if (location != -1)
    {
        glUniform1i(location, value);
    }
}


