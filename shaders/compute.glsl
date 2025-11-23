#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct Vertex 
{
    vec4 pos;
    vec4 normal;
    vec2 uv;
    vec2 padding;
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

layout(std430, binding = 2) buffer MaterialBuffer 
{
    Material materials[];
};

layout(std430, binding = 3) buffer MaterialIndexBuffer 
{
    uint materialIndices[];
};

// Camera data UBO
layout(std140, binding = 0) uniform SceneParams
{
    GpuSceneParams gpuSceneParams;
};

//Reimplementation of the algorithm of Möller and Trumbore
//Möller, T., & Trumbore, B. (1997). Fast, minimum storage ray-triangle intersection. Journal of Graphics Tools, 2(1), 21-28.
bool intersectTriangle(vec3 orig, vec3 dir, vec3 v0, vec3 v1, vec3 v2, out float tHit, out vec2 hit)
{
    const float EPSILON = 1e-3;

    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

    vec3 p = cross(dir, e2);
    float det = dot(e1, p);

    if (abs(det) < EPSILON)
        return false;

    float invDet = 1.0 / det;
    vec3 t = orig - v0;

    float u = dot(t, p) * invDet;
    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(t, e1);
    float v = dot(dir, q) * invDet;
    if (v < 0.0 || u + v > 1.0)
        return false;

    float tTemp = dot(e2, q) * invDet;
    if (tTemp < EPSILON) {
        return false;
    }

    hit = vec2(u, v);
    tHit = tTemp;
    return true;
}

void main() 
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outputImage);

    if (pixel.x < 0 || pixel.y < 0 || pixel.x >= size.x || pixel.y >= size.y)
    {
        return;
    }

    // Normalized uv in [0,1]
    vec2 uv = (vec2(pixel) + 0.5) / vec2(size);

    vec3 origin = gpuSceneParams.camera.origin.xyz;
    vec3 lowerLeft = gpuSceneParams.camera.lowerLeft.xyz;
    vec3 horizontal = gpuSceneParams.camera.horizontal.xyz;
    vec3 vertical = gpuSceneParams.camera.vertical.xyz;

    vec3 target = lowerLeft + uv.x * horizontal + uv.y * vertical;
    vec3 dir = normalize(target - origin);

    bool isHit = false;

    // Corresponds to "t" from the term: intersection = origin + t * direction
    // Big initial value to guarantee that the first intersection is closer than the inital value
    float distanceToClosestIntersection = 1e30;

    uint indexCount = indices.length();

    // Iterate over triangles
    for (uint i = 0; i < indexCount; i += 3)
    {
        uint i0 = indices[i + 0];
        uint i1 = indices[i + 1];
        uint i2 = indices[i + 2];

        //Corners of the triangle
        vec3 v0 = vertices[i0].pos.xyz;
        vec3 v1 = vertices[i1].pos.xyz;
        vec3 v2 = vertices[i2].pos.xyz;

        //Saves the distance "t" (intersection = origin + t * direction) to the intersection calculated by intersectTriangle()
        float distanceToIntersection;

        //UV coordinates inside the triangle of the hitpoint 
        vec2 hitPoint;

        if (intersectTriangle(origin, dir, v0, v1, v2, distanceToIntersection, hitPoint))
        {
            if (distanceToIntersection < distanceToClosestIntersection) 
            {
                distanceToClosestIntersection = distanceToIntersection;
                isHit = true;
            }
        }
    }

    if (isHit)
        imageStore(outputImage, pixel, vec4(1.0, 1.0, 1.0, 1.0));
    else
        imageStore(outputImage, pixel, vec4(0.0, 0.0, 0.0, 1.0));
}