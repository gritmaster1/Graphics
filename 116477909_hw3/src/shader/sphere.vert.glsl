#version 410 core

layout (location = 0) in vec3 position;
out vec3 tessPosition;

void main()
{
    tessPosition = position;
    gl_Position = vec4(position, 1.0f);
}




