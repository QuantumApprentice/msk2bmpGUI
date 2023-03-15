#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D Indexed_FRM;
uniform sampler2D Indexed_PAL;
uniform vec3 ColorPalette[256];

void main()
{
    vec3 texel;
    vec4 index1 = texture(Indexed_FRM, vec2(TexCoord.x, TexCoord.y));
    vec4 index2 = texture(Indexed_PAL, vec2(TexCoord.x, TexCoord.y));
    if (index2.r == 0) {
        texel = ColorPalette[int(index1.r*255)];
    }
    else {
        texel = ColorPalette[int(index2.r*255)];
    }
    //texel = ColorPalette[int(index1.r*255)];

    //alpha channel background
    if (index1.r == 0) {
        float xTile = mod(TexCoord.x, 0.1);
        float yTile = mod(TexCoord.y, 0.1);
        if ((xTile < 0.05 && yTile < 0.05) || (xTile >= 0.05 && yTile >= 0.05))
        {
            FragColor = vec4(1,1,1,1);
        }
        else {
            FragColor = vec4(0.8,0.8,0.8,1);
        }
    }
    else {
    //forground
        FragColor = vec4(texel, 1.0f);
        //FragColor = vec4(TexCoord.x, TexCoord.y, 1.0, 1.0f);
    }
}