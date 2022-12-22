#version 330 core

uniform sampler2D state;
uniform vec2 scale;
in vec2 TexCoord;

void main()
{
if (TexCoord.x < 0 || TexCoord.x > 1 || TexCoord.y < 0 || TexCoord.y > 1)
    {
        float xTile = mod(TexCoord.x*scale.x, 20);
        float yTile = mod(TexCoord.y*scale.y, 20);
        if ((xTile < 10 && yTile < 10) || (xTile >= 10 && yTile >= 10))
        {
            gl_FragColor = vec4(0.3,
                                0.3,
                                0.3,
                                1.0);
        }
        else {
            gl_FragColor = vec4(0.2,
                                0.2,
                                0.2,
                                1.0);
        }
    }
else {
        float xTile = mod(TexCoord.x*scale.x, 20);
        float yTile = mod(TexCoord.y*scale.y, 20);
        if ((xTile < 10 && yTile < 10) || (xTile >= 10 && yTile >= 10))
        {
            gl_FragColor = vec4(0.3,
                                0.3,
                                0.3,
                                1.0);
        }
        else {
            gl_FragColor = vec4(0.2,
                                0.2,
                                0.2,
                                1.0);
        }
        gl_FragColor = texture2D(state, TexCoord);
    }
}