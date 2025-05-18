#version 330 core
layout(location = 0) in vec2 aQuadPos;
layout(location = 1) in ivec2 aGridCoord;

uniform mat4 uProjection;
uniform float uCellSize;

void main() {
    vec2 worldPos = vec2(aGridCoord) * uCellSize + aQuadPos * uCellSize;
    gl_Position = uProjection * vec4(worldPos, 0.0, 1.0);
}