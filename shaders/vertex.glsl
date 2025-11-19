#version 430

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec2 aUv;

struct CameraParams {
    mat4 viewProj;
    vec4 origin;
    vec4 lowerLeft;
    vec4 horizontal;
    vec4 vertical;
};

struct LightParams {
    vec4 position;
    vec4 color;
};

layout(std140, binding = 0) uniform SceneBlock 
{
    CameraParams camera;
    LightParams light;
};

out vec4 vWorldPos;
out vec4 vNormal;
out vec2 vUv;

void main() 
{
    vWorldPos = aPos;
	vNormal   = aNormal;
    vUv       = aUv;
    gl_Position = camera.viewProj * aPos;
}