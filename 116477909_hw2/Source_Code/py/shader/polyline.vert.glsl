#version 330 core
layout (location = 0) in vec2 aPos;

uniform float windowWidth;
uniform float windowHeight;

void main() {
    float xNorm = (aPos.x / windowWidth) * 2.0 - 1.0;
    float yNorm = (aPos.y / windowHeight) * 2.0 - 1.0;
    gl_Position = vec4(xNorm, yNorm, 0.0, 1.0);
}
