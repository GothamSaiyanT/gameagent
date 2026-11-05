#include "VertexArray.h"
#include "GL/glew.h"

VertexArray::VertexArray(const float* verts,unsigned int numVerts
        ,const unsigned int* indices,unsigned int numIndices)
{
    //create the vertex array object and store its ID in the mVertexArray member variable
    glGenVertexArrays(1,&mVertexArray);
    glBindVertexArray(mVertexArray);

    //create the vertex buffer
    glGenBuffers(1,&mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,mVertexBuffer);

    //now copy the verts data passed into the vertex array construtor into this vertex buffer
    glBufferData(GL_ARRAY_BUFFER,//the active buffer type to write to
                  numVerts * 5 * sizeof(float),//number of bytes to copy
                  verts,//source to copy from(pointer)
                  GL_STATIC_DRAW//how will we use this data
                  );

    glGenBuffers(1,&mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mIndexBuffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,//index buffer
                  numIndices * sizeof(unsigned int),//size od data
                  indices,GL_STATIC_DRAW

                  );
                  glEnableVertexAttribArray(0);
                    glVertexAttribPointer(
                    0, // Attribute index (0 for first one)
                    3, // Number of components (3 in this case)
                    GL_FLOAT, // Type of the components
                    GL_FALSE, // (Only used for integral types)
                    sizeof(float) * 5, // Stride (usually size of each vertex)
                    0 // Offset from start of vertex to this attribute
                    );
    glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5,
		reinterpret_cast<void*>(sizeof(float) * 3));


}
VertexArray::~VertexArray(){
    glDeleteBuffers(1,&mVertexBuffer);
    glDeleteBuffers(1,&mIndexBuffer);
    glDeleteBuffers(1,&mVertexArray);

}
 void VertexArray::SetActive(){
     glBindVertexArray(mVertexArray);
 }
