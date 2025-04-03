#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D Indexed_FRM;
uniform sampler2D Indexed_PAL;
uniform sampler2D Indexed_MSK;
uniform uint ColorPaletteINT[256];

void main()
{
    vec4 texel;
    vec4 index1 = texture(Indexed_FRM, vec2(TexCoord.x, TexCoord.y));
    vec4 index2 = texture(Indexed_PAL, vec2(TexCoord.x, TexCoord.y));
    vec4 index3 = texture(Indexed_MSK, vec2(TexCoord.x, TexCoord.y));

    if (index2.r == 0) {
        // texel = vec4(ColorPaletteINT[int(index1.r*255)]);//, 1.0f);
        uint idx = uint(index1.r*255);
        uint color = ColorPaletteINT[idx];
        texel = vec4(
            float(int(color >>  0) & 0xFF) / 255.0,    // r
            float(int(color >>  8) & 0xFF) / 255.0,    // g
            float(int(color >> 16) & 0xFF) / 255.0,    // b
            1.0
        );
    }
    else {
        // texel = vec4(ColorPaletteINT[int(index2.r*255)]);//, 1.0f);
        uint idx = uint(index2.r*255);
        uint color = ColorPaletteINT[idx];
        texel = vec4(
            float(int(color >>  0) & 0xFF) / 255.0,    // r
            float(int(color >>  8) & 0xFF) / 255.0,    // g
            float(int(color >> 16) & 0xFF) / 255.0,    // b
            1.0
        );
    }

    if (index3.r > 0) {
        texel = mix(texel, vec4(1,1,1,1), 0.5);
    }

    //alpha channel background
    if ((index1.r + index2.r + index3.r) == 0) {
        float xTile = mod(TexCoord.x, 0.1);
        float yTile = mod(TexCoord.y, 0.1);
        if ((xTile < 0.05 && yTile < 0.05) || (xTile >= 0.05 && yTile >= 0.05))
        {
            FragColor = vec4(0.5,0.5,0.5,1.0);
        }
        else {
            FragColor = vec4(0.2,0.2,0.2,1.0);
        }
    }
    else {
    //forground
        FragColor = texel;
    }
}