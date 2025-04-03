#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D Indexed_FRM;
uniform uint ColorPaletteINT[256];

void main()
{
    vec4 index = texture(Indexed_FRM, vec2(TexCoord.x, TexCoord.y));
    // vec4 texel = vec4(index);

    uint idx = uint(index.r*255.0);
    uint color = ColorPaletteINT[idx];
    vec4 texel = vec4(
        float((color >>  0) & 0xFFu) / 255.0,    // r
        float((color >>  8) & 0xFFu) / 255.0,    // g
        float((color >> 16) & 0xFFu) / 255.0,    // b
        1.0
    );


    //alpha channel background
    if (index.r == 0) {
        float xTile = mod(TexCoord.x, 0.1);
        float yTile = mod(TexCoord.y, 0.1);
        if ((xTile < 0.05 && yTile < 0.05) || (xTile >= 0.05 && yTile >= 0.05))
        {
            FragColor = vec4(0.5,0.5,0.5,1.0);
        }
        else {
            FragColor = vec4(0.2,0.2,0.2,1.0);
        }
    } else {
        //forground
        // FragColor = vec4(texel, 1.0);
        FragColor = texel;
    }
}