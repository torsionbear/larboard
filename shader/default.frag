#version 430 core

uniform sampler2D texture0;

in vec2 fTexCoord;
layout (location = 0)  out vec4 fColor;

void main()
{
    fColor = texture(texture0, fTexCoord);
    //fColor = vec4(c.x/255, c.y/255, c.z/255, c.w/255);
}