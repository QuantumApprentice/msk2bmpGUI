#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D Palette;
uniform sampler2D IndexedColorTexture;

uniform float alpha;
uniform float beta;
uniform float time;
float   last_time;
float   y = 0;

void main()
{
    vec4 index = texture(IndexedColorTexture, vec2(TexCoord.x, 1-TexCoord.y));

    if(index.r >= 229.0/255.0 && index.r < 233.0/255.0)             //slime colors
    {
        float color_count  = 4.0;
        float change_time_ms = 0.200;
        float cycle_length = (change_time_ms/1.0) * color_count;

        y = (color_count/6.0) * mod(time, cycle_length)/ cycle_length;
    }
    if(index.r >= 233.0/255.0 && index.r < 238.0/255.0)             //monitor colors
    {
        float color_count  = 5.0;
        float change_time_ms = 0.100;
        float cycle_length = (change_time_ms/1.0) * color_count;

        y = (color_count/6.0) * mod(time, cycle_length)/ cycle_length;
    }
    if(index.r >= 238.0/255.0 && index.r < 243.0/255.0)             //slow fire colors
    {
        float color_count  = 5.0;
        float change_time_ms = 0.200;
        float cycle_length = (change_time_ms/1.0) * color_count;

        y = (color_count/6.0) * mod(time, cycle_length)/ cycle_length;
    }
    if(index.r >= 243.0/255.0 && index.r < 248.0/255.0)             //fast fire colors
    {
        float color_count  = 5.0;
        float change_time_ms = 0.142;
        float cycle_length = (change_time_ms/1.0) * color_count;

        y = (color_count/6.0) * mod(time, cycle_length)/ cycle_length;
    }
    if(index.r >= 248.0/255.0 && index.r < 254.0/255.0)             //beach colors
    {
        float color_count  = 6.0;
        float change_time_ms = 0.200;
        float cycle_length = (change_time_ms/1.0) * color_count;

        y = (color_count/6.0) * mod(time, cycle_length)/ cycle_length;
    }
    if(index.r >= 254.0/255.0 && index.r < 255.0/255.0)             //blinking red colors
    {
        float color_count  = 6.0;
        float change_time_ms = 0.033;
        float cycle_length = 4.2;    //(change_time_ms/1.0) * color_count;

        y = (color_count/6.0) * mod(time, cycle_length)/ cycle_length;
    }

    //vec4 texel = texture(Palette, vec2(index.r, mod(time, 1.0)));
    vec4 texel = texture(Palette, vec2(index.r, y));

//alpha channel background
    if(index.r == 0){
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
        FragColor = texel;
    }
}