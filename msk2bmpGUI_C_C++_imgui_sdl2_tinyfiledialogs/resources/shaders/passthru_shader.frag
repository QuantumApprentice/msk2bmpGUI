#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D inTexture;

void main()
{
    vec4 index = texture(inTexture, vec2(TexCoord.x, TexCoord.y));
    vec4 temp_alpha;

    float xTile = mod(TexCoord.x, 0.1);
    float yTile = mod(TexCoord.y, 0.1);
    if ((xTile < 0.05 && yTile < 0.05) || (xTile >= 0.05 && yTile >= 0.05))
    {
        temp_alpha = vec4(0.5,0.5,0.5,1);
    }
    else {
        temp_alpha = vec4(0.2,0.2,0.2,1);
    }
    FragColor = mix(index, temp_alpha, 1-index.a);
}
