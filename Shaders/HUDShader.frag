#version 330
in vec2 fragTexCoord;

uniform sampler2D uTexture;
uniform vec4      uTint;       // RGBA tint multiplier
uniform bool      uUseTexture; // false = solid colour rectangle

out vec4 outColor;

void main()
{
    if (uUseTexture)
        outColor = texture(uTexture, fragTexCoord) * uTint;
    else
        outColor = uTint;
}
