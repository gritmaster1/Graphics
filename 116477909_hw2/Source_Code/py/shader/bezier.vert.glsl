#version 330 core
layout (location = 0) in vec2 aPos;

void main() {
    gl_Position = vec4(aPos.x / 500.0 - 1.0, aPos.y / 500.0 - 1.0, 0.0, 1.0);
}
