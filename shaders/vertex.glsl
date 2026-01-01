#version 430

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec2 aUv;

struct GpuCameraParams{
    mat4 viewProj;
    vec4 origin;
    vec4 lowerLeft;
    vec4 horizontal;
    vec4 vertical;
};

struct GpuLightParams  {
    vec4 position;
    vec4 color;
    float intensity;
    vec3 _pad;
};

struct GpuSceneParams {
    GpuCameraParams camera;
    GpuLightParams  light;
    vec4 backgroundColor;
};

layout(std140, binding = 0) uniform SceneParams 
{
       GpuSceneParams gpuSceneParams;
};

out vec4 vWorldPos;
out vec4 vNormal;
out vec2 vUv;

void main() 
{
    vWorldPos = aPos;
	vNormal   = aNormal;
    vUv       = aUv;
    gl_Position = gpuSceneParams.camera.viewProj * aPos;
}