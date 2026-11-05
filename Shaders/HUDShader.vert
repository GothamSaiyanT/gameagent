#version 330
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

uniform mat4 uOrtho;          // screen-space projection
uniform vec2 uPosition;       // top-left pixel position
uniform vec2 uSize;           // width, height in pixels

out vec2 fragTexCoord;

void main()
{
    // Quad is -0.5..0.5 — scale to size then translate to position
    // Then add 0.5*size so uPosition refers to TOP-LEFT
    vec2 worldPos = vec2(
        inPosition.x * uSize.x + uPosition.x + uSize.x * 0.5,
        inPosition.y * uSize.y + uPosition.y + uSize.y * 0.5
    );

    gl_Position = uOrtho * vec4(worldPos, 0.0, 1.0);
    fragTexCoord = inTexCoord;
}
