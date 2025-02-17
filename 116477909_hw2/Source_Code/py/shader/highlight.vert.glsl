#version 330 core
layout (location = 0) in vec2 aPos;

uniform float windowWidth;
uniform float windowHeight;

void main() {
    vec2 pos = aPos / vec2(windowWidth, windowHeight) * 2.0 - 1.0;
    gl_Position = vec4(pos * vec2(1.0, -1.0), 0.0, 1.0);
}
