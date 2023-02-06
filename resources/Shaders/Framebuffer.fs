#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
 
uniform sampler2D screenTexture;
 
uniform float offsetNumerator = 300;  
float offset = 1.0 / offsetNumerator;  
void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // верхний-левый
        vec2( 0.0f,    offset), // верхний-центральный
        vec2( offset,  offset), // верхний-правый
        vec2(-offset,  0.0f),   // центральный-левый
        vec2( 0.0f,    0.0f),   // центральный-центральный
        vec2( offset,  0.0f),   // центральный-правый
        vec2(-offset, -offset), // нижний-левый
        vec2( 0.0f,   -offset), // нижний-центральный
        vec2( offset, -offset)  // нижний-правый    
    );
 
    float kernel[9] = float[](
        1,  1, 1,
        1,  1, 1,
        1,  1, 1
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.xy + offsets[i]));
    }
    vec3 col = vec3(0.0);
    
        col += sampleTex[0]*0.10205837202;
        col += sampleTex[1]*0.31946576033846985;
        col += sampleTex[2]*0.10205837202;
        col += sampleTex[3]*0.31946576033846985;
        col += sampleTex[4]*0.3610684793230603;
        col += sampleTex[5]*0.31946576033846985;
        col += sampleTex[6]*0.10205837202;
        col += sampleTex[7]*0.31946576033846985;
        col += sampleTex[8]*0.10205837202;
        col /= 2.047;
        FragColor = vec4(col , 1.0);
        
    
    
    
} 

