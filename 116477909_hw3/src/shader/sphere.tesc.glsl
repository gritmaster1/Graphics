#version 410

layout (vertices = 1) out;


uniform float tessLevelOuter;

in vec3 tessPosition[];
out vec3 tessControlPosition[];

void main()
{
    gl_TessLevelOuter[0] = tessLevelOuter;
    gl_TessLevelOuter[1] = tessLevelOuter;
    gl_TessLevelOuter[2] = tessLevelOuter;
    gl_TessLevelOuter[3] = tessLevelOuter;

    gl_TessLevelInner[0] = tessLevelOuter;
    gl_TessLevelInner[1] = tessLevelOuter;


     tessControlPosition[gl_InvocationID] = tessPosition[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}



