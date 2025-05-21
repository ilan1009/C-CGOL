#version 330 core
layout(location = 0) in ivec2 aPos;

uniform mat4 uProjection;
uniform float uCellSize;

void main() {
    vec2 position = vec2(aPos) * uCellSize;
    gl_Position = uProjection * vec4(position, 0.0, 1.0);
    gl_PointSize = uCellSize;
}