#version 430

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
    LightParams  light;
};

in vec4 vWorldPos;
in vec4 vNormal;
in vec2 vUv;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
