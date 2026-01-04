#version 430

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aColor;

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
    vec4 intensity; //only .x used and its between [0,1]
};

layout(std140, binding = 0) uniform SceneBlock 
{
    CameraParams camera;
    LightParams light;
};

out vec4 vColor;
out vec4 vWorldPos;

void main() 
{
    vWorldPos = aPos;
    vColor = aColor;
	
    gl_Position = camera.viewProj * aPos;
}