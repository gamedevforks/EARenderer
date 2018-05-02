#version 400 core

// Attributes

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iLightmapCoords;

// Output

out InterfaceBlock {
    vec2 lightmapCoords;
} vs_out;

// Functions

void main() {
    vs_out.lightmapCoords = iLightmapCoords;
    gl_Position = vec4(iPosition, 1.0);
}