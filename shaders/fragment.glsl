#version 430

struct GpuCameraParams  {
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

in vec4 vWorldPos;
in vec4 vNormal;
in vec2 vUv;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(gpuSceneParams.backgroundColor.rgb, 1.0);
}
