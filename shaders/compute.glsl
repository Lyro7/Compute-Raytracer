#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct Material
{
    vec4 diffuseColor;	// rgb = diffuseColor.xyz, w unused
	vec4 specularColor; // rgb = specularColor.xyz, shininess(Ns) = w
	vec4 emission;      // emissionColor = emission.xyz, emissionStrength = w
};

struct Triangle {
    vec4 v1;
    vec4 v2;
    vec4 v3;
    vec4 NA;
    vec4 NB;
    vec4 NC;
    Material material;
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
    vec4 intensity;
};

struct GpuSceneParams
{
    GpuCameraParams camera;
    GpuLightParams light;
};

struct MeshInfo {
    int firstTriangleIndex;
    int numTriangles;
    vec2 padding;
};

layout(std430, binding = 0) buffer triangleBuffer 
{
    Triangle triangles[];
};

layout(std430, binding = 1) buffer meshInfoBuffer 
{
    MeshInfo meshInfos[];
};

layout(binding = 0, rgba32f) uniform image2D outputImage;

// Camera data UBO
layout(std140, binding = 0) uniform SceneParams
{
    GpuSceneParams gpuSceneParams;
};

//Reimplementation of the algorithm of M�ller and Trumbore
//M�ller, T., & Trumbore, B. (1997). Fast, minimum storage ray-triangle intersection. Journal of Graphics Tools, 2(1), 21-28.
bool intersectTriangle(vec3 orig, vec3 dir, vec3 v0, vec3 v1, vec3 v2, out float tHit, out vec2 hit)
{
    const float EPSILON = 1e-6;

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

/*
bool isInShadow(vec3 hitPos, vec3 lightPos, uint ignoreTri)
{
    vec3 shadowDir = normalize(lightPos - hitPos);
    float maxDist = length(lightPos - hitPos);

    uint indexCount = indices.length();

    for (uint i = 0; i < indexCount; i += 3)
    {
        if (i == ignoreTri) continue;
    
        uint i0 = indices[i + 0];
        uint i1 = indices[i + 1];
        uint i2 = indices[i + 2];

        vec3 v0 = vertices[i0].pos.xyz;
        vec3 v1 = vertices[i1].pos.xyz;
        vec3 v2 = vertices[i2].pos.xyz;

        float tHitShadow;
        vec2 dummy;

        //Send a ray from hitpoint to light source. Origin of the ray is moved a bit in the direction of the light source to prevent self-intersection.
        if (intersectTriangle(hitPos + shadowDir * 1e-6, shadowDir, v0, v1, v2, tHitShadow, dummy))
        {
            if (tHitShadow > 0.0 && tHitShadow < maxDist)
            {
                return true; //in shadow
            }
        }
    }
    return false; // not in shadow
}*/

void main() 
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = imageSize(outputImage);

    if (pixel.x < 0 || pixel.y < 0 || pixel.x >= size.x || pixel.y >= size.y)
        return;

    // Normalized uv in [0,1]
    vec2 uv = (vec2(pixel) + 0.5) / vec2(size);

    vec3 origin     = gpuSceneParams.camera.origin.xyz;
    vec3 lowerLeft  = gpuSceneParams.camera.lowerLeft.xyz;
    vec3 horizontal = gpuSceneParams.camera.horizontal.xyz;
    vec3 vertical   = gpuSceneParams.camera.vertical.xyz;

    vec3 target = lowerLeft + uv.x * horizontal + uv.y * vertical;
    vec3 dir    = normalize(target - origin);

    // Trace
    bool isHit = false;
    float distanceToClosestIntersection = 1e30;
    uint hitTriIndex = 0u;
    vec2 closestHitPoint = vec2(0.0);

    // Iterate over triangles
    for (uint i = 0u; i < triangles.length(); ++i)
    {
        Triangle tri = triangles[i];
        vec3 v0 = tri.v1.xyz;
        vec3 v1 = tri.v2.xyz;
        vec3 v2 = tri.v3.xyz;

        float tHit;
        vec2 hitUV;

        if (intersectTriangle(origin, dir, v0, v1, v2, tHit, hitUV))
        {
            if (tHit < distanceToClosestIntersection)
            {
                distanceToClosestIntersection = tHit;
                isHit = true;
                hitTriIndex = i;
                closestHitPoint = hitUV;
            }
        }
    }

    vec3 color = vec3(0.0);

    if (isHit)
    {
        Triangle tri = triangles[hitTriIndex];
        vec3 hitPos   = origin + dir * distanceToClosestIntersection;
        vec3 lightPos = gpuSceneParams.light.position.xyz;

        float distanceToLight = length(lightPos - hitPos);
        float attenuation = 1.0 / (1.0 + 0.02 * distanceToLight + 0.001 * distanceToLight * distanceToLight);
        vec3 radiance = gpuSceneParams.light.color.rgb * gpuSceneParams.light.intensity.x * attenuation;

        float u = closestHitPoint.x;
        float v = closestHitPoint.y;
        float w = 1.0 - u - v;

        vec3 N = normalize(w * tri.NA.xyz + u * tri.NB.xyz + v * tri.NC.xyz);
        if (dot(N, dir) > 0.0) N = -N;

        vec3 L = normalize(lightPos - hitPos);
        float NdotL = max(dot(N, L), 0.0);

        Material mat = tri.material;

        vec3 diffuse = mat.diffuseColor.rgb * radiance * NdotL;

        vec3 V = normalize(origin - hitPos);
        vec3 H = normalize(L + V);
        float specPow = mat.specularColor.w; // shininess
        float spec = pow(max(dot(N, H), 0.0), specPow);
        vec3 specular = mat.specularColor.rgb * radiance * spec;

        vec3 emission = mat.emission.xyz * mat.emission.w;

        color = diffuse + specular + emission;
    }

    imageStore(outputImage, pixel, vec4(color, 1.0));
}
