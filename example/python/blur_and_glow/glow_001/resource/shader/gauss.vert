#version 400

layout (location = 0) in vec2 inPos;

out vec2 vertPos;

void main()
{
    vertPos.xy  = inPos.xy;
    gl_Position = vec4( inPos, 0.0, 1.0 );
}