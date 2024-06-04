#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D Indexed_FRM;
uniform vec3 ColorPalette[256];

void main()
{
    vec4 index = texture(Indexed_FRM, vec2(TexCoord.x, TexCoord.y));

    vec3 texel = ColorPalette[int(index.r*255)];

    //alpha channel background
    if (index.r == 0) {
        float xTile = mod(TexCoord.x, 0.1);
        float yTile = mod(TexCoord.y, 0.1);
        if ((xTile < 0.05 && yTile < 0.05) || (xTile >= 0.05 && yTile >= 0.05))
        {
            FragColor = vec4(0.5,0.5,0.5,1);
        }
        else {
            FragColor = vec4(0.2,0.2,0.2,1);
        }
    }
    else {
    //forground
        FragColor = vec4(texel, 1.0f);
    }
}