#version 330 core
     layout (location = 0) in vec3 aPos;
     layout (location = 1) in vec2 aTexCoord;

     uniform float new_zoom;
     uniform vec2 bottom_left_pos;

     out vec2 TexCoord;

     void main()
     {
        float scale = 1.0/new_zoom;

        vec2  scaled_TexCoord   = aTexCoord * scale;

        TexCoord = scaled_TexCoord + bottom_left_pos;
        gl_Position = vec4(aPos.x, aPos.y, 0, 1);
     }