#version 330 core

uniform sampler1D Palette;              // A palette of 256 colors
uniform sampler2D IndexedColorTexture;  // A texture using indexed color
in vec2 TexCoord0;                      // UVs
out vec4 FragColor;

void main()
{
    // Pick up a color index
    vec4 index = texture2D(IndexedColorTexture, TexCoord0);

    // Retrieve the actual color from the palette
    vec4 texel = texture(Palette, index.x);

    FragColor = texel;   //Output the color
}