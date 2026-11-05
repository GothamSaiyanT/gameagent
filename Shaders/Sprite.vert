#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

uniform mat4 uViewProj;
uniform mat4 uWorldTransform;

uniform vec2 uUVOffset;
uniform vec2 uUVScale;
uniform bool uUseUVTransform;

out vec2 fragTexCoord;

void main()
{
    vec4 pos = vec4(inPosition, 1.0);
    gl_Position = pos * uWorldTransform * uViewProj;

    if (uUseUVTransform)
        fragTexCoord = inTexCoord * uUVScale + uUVOffset;
    else
        fragTexCoord = inTexCoord; // Default full texture
}
