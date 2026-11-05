#ifndef VERTEXARRAY_H_INCLUDED
#define VERTEXARRAY_H_INCLUDED

class VertexArray
{
    public:
        VertexArray(const float* verts,unsigned int numVerts
                    ,const unsigned int* indices,unsigned int numIndices);
        ~VertexArray();

        //activate this vertex array(so we can draw it)
        void SetActive();

        unsigned int GetNumIndices()const {return mNumIndices;}
        unsigned int GetNumVerts()const {return mNumVerts;}

    private:
        //how many vertices in the vertex buffer?
        unsigned int mNumVerts;
        //how many indices in the index buffer
        unsigned int mNumIndices;
        //openGl id of vertex buffer
        unsigned int mVertexBuffer;
        //openGl id of  index buffer
        unsigned int mIndexBuffer;
        //openGl id of vertex array object
        unsigned int mVertexArray;



};

#endif // VERTEXARRAY_H_INCLUDED
