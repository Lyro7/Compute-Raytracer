#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

#define EPSILON 5e-3

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
	vec4 isPreview; // only .x is used 1=true 0=false 
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
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

    vec3 p = cross(e1, e2);
    float det = -dot(dir, p);

    if (det < EPSILON)
        return false;

    float invDet = 1.0 / det;
    vec3 ao = orig - v0;
    vec3 dao = cross(ao, dir);

    float t = dot(ao, p) * invDet;
    float u = dot(e2, dao) * invDet;
    float v = -dot(e1, dao) * invDet;
    float w = 1.0 - u - v;

    if (t < 0 || u < 0.0 || v < 0.0 || w < 0.0)
    {
        return false;
    }

    hit = vec2(u, v);
    tHit = t;
    return true;
}

bool isInShadow(vec3 hitPos, vec3 lightPos, uint ignoreTri)
{
    vec3 shadowDir = normalize(lightPos - hitPos);
    float maxDist = length(lightPos - hitPos);

    // Durch alle Dreiecke iterieren
    for (uint i = 0u; i < triangles.length(); ++i)
    {
        if (i == ignoreTri) continue; 

        Triangle tri = triangles[i];
        vec3 v0 = tri.v1.xyz;
        vec3 v1 = tri.v2.xyz;
        vec3 v2 = tri.v3.xyz;

        float tHitShadow;
        vec2 dummy;

        // Verschiebe den Ursprung ein wenig, um Selbstüberschneidungen zu vermeiden
        if (intersectTriangle(hitPos, shadowDir, v0, v1, v2, tHitShadow, dummy))
        {
            if (tHitShadow < maxDist - 1e-4)
            {
                return true; // im Schatten
            }
        }
    }
    return false; // nicht im Schatten
}

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

    vec3 skyColor = vec3(0.429, 0.708, 0.822);
    vec3 color = skyColor;

    if (isHit)
    {
        Triangle tri = triangles[hitTriIndex];
        Material mat = tri.material;

        if(gpuSceneParams.isPreview.x == 1)
        {
            vec3 diffuse = mat.diffuseColor.rgb;
            color = (diffuse + gpuSceneParams.light.color.xyz * 0.2) * (gpuSceneParams.light.intensity.x / 250.0);
        }
        else
        {
            vec3 hitPos   = origin + dir * distanceToClosestIntersection;
            vec3 lightPos = gpuSceneParams.light.position.xyz;
            vec3 toLight  = normalize(lightPos - hitPos);

            float distanceToLight = length(lightPos - hitPos);
            float attenuation = 1.0 / (1.0 + 0.02 * distanceToLight + 0.01 * distanceToLight * distanceToLight);

            float u = closestHitPoint.x;
            float v = closestHitPoint.y;
            float w = 1.0 - u - v;

            vec3 v0 = tri.v1.xyz;
            vec3 v1 = tri.v2.xyz;
            vec3 v2 = tri.v3.xyz;

            vec3 N = normalize(w * tri.NA.xyz + u * tri.NB.xyz + v * tri.NC.xyz);

        

            vec3 diffuse = mat.diffuseColor.rgb;

            // scales with light intensitiy
            vec3 ambientLighting = diffuse * vec3(0.3,0.3,0.4) * (gpuSceneParams.light.intensity.x /100.0);
            color = ambientLighting;
        
            if (!isInShadow(hitPos, lightPos, hitTriIndex))
            { 
                //Lambert -> Helligkeit abhängig vom Einfallswinkel
                float NdotL = max(dot(N, toLight), 0.0);
                color += diffuse * gpuSceneParams.light.color.xyz * NdotL * attenuation * gpuSceneParams.light.intensity.x;
            }
        
            //imageStore(outputImage, pixel, vec4(N * 0.5 + 0.5, 1.0));
            //return;
           }
    }
      
    imageStore(outputImage, pixel, vec4(color, 1.0));
}
