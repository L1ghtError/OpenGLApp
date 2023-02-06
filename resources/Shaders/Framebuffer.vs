#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
 
out vec2 TexCoords;
 out vec2 position;
void main()
{
    position =  vec2(aPos.x, aPos.y);
    gl_Position =vec4(position,0.0,1.0); 
    TexCoords = aTexCoords;
}  