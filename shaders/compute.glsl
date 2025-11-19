#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct Vertex 
{
    vec4 pos;
    vec4 normal;
    vec2 uv;
};

struct Material
{
    vec4 albedo;
    float roughness;
    float metallic;
    float emissionStrength;
    vec4 emissionColor;
};

struct GpuCameraParams{
	mat4 viewProj;
	vec4 origin;
	vec4 lowerLeft;
	vec4 horizontal;
	vec4 vertical;
}; 

struct GpuLightParams{
	vec4 position;
	vec4 color;
};

struct GpuSceneParams
{
    GpuCameraParams camera;
    GpuLightParams light;
};

layout(binding = 0, rgba32f) uniform image2D outputImage;

// Mesh data SSBO
layout(std430, binding = 0) buffer VertexBuffer 
{
    Vertex vertices[];
};

layout(std430, binding = 1) buffer IndexBuffer 
{
    uint indices[];
};

layout(std430, binding = 2) buffer MaterialsBuffer 
{
    Material materials[];
};
// Camera data UBO
layout(std140, binding = 0) uniform SceneParams
{
    GpuSceneParams gpuSceneParams;
};

void main() 
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);

    if (pixel.x < 0 || pixel.y < 0 || pixel.x >= size.x || pixel.y >= size.y)
    {
        return;
    }

    vec2 uv = (vec2(pixel) + 0.5) / vec2(size);
    vec3 col = vec3(uv.x, uv.y, 0.5);

    imageStore(outputImage, pixel, vec4(col, 1.0));
}