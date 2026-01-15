#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

#define EPSILON 5e-3
#define FLOOR_Y -20.0
#define floorPoint vec3(0.0, FLOOR_Y, 0.0)
#define floorNormal vec3(0.0, 1.0, 0.0)
#define TILE_SIZE 6.0
#define MAX_LIGHTS 64

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

    ivec4 lightMeta;

    GpuLightParams lights[MAX_LIGHTS];
	vec4 isPreview; // Only .x is used 1=true 0=false 
    vec4 backgroundColor;
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

// Reimplementation of the algorithm of M�ller and Trumbore
// M�ller, T., & Trumbore, B. (1997). Fast, minimum storage ray-triangle intersection. Journal of Graphics Tools, 2(1), 21-28.
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

    // Iterate over all triangles
    for (uint i = 0u; i < triangles.length(); ++i)
    {
        if (i == ignoreTri) continue; 

        Triangle tri = triangles[i];
        vec3 v0 = tri.v1.xyz;
        vec3 v1 = tri.v2.xyz;
        vec3 v2 = tri.v3.xyz;

        float tHitShadow;
        vec2 dummy;

        if (intersectTriangle(hitPos, shadowDir, v0, v1, v2, tHitShadow, dummy))
        {
            if (tHitShadow < maxDist - 1e-4)
            {
                return true;// In shadow
            }
        }
    }
    return false; // Not in shadow
}

bool intersectPlane(vec3 orig, vec3 dir, vec3 planePoint, vec3 planeNormal, out float tHit)
{
    float denom = dot(planeNormal, dir);

    if (abs(denom) < EPSILON)
        return false; // Ray parallel to plane

    tHit = dot(planePoint - orig, planeNormal) / denom;
    return tHit > EPSILON;
}


vec3 traceColor(vec3 ro, vec3 rd)
{
    // find closest hit
    bool isHit = false;
    float tMin = 1e30;
    uint hitTriIndex = 0u;
    vec2 closestHitPoint = vec2(0.0);

    for (uint i = 0u; i < triangles.length(); ++i)
    {
        Triangle tri = triangles[i];
        float tHit;
        vec2 hitUV;

        if (intersectTriangle(ro, rd, tri.v1.xyz, tri.v2.xyz, tri.v3.xyz, tHit, hitUV))
        {
            if (tHit < tMin)
            {
                tMin = tHit;
                isHit = true;
                hitTriIndex = i;
                closestHitPoint = hitUV;
            }
        }
    }

    bool hitFloor = false;
    float floorTHit = 1e30;
    if (intersectPlane(ro, rd, floorPoint, floorNormal, floorTHit))
    {
        if (floorTHit < tMin)
        {
            hitFloor = true;
            tMin = floorTHit;
            isHit = true;
        }
    }

    if (!isHit) return gpuSceneParams.backgroundColor.rgb;

    vec3 hitPos = ro + rd * tMin;
    int lightCount = clamp(gpuSceneParams.lightMeta.x, 0, MAX_LIGHTS);

    // floor hit
    if (hitFloor)
    {
        int cx = int(floor(hitPos.x / TILE_SIZE));
        int cz = int(floor(hitPos.z / TILE_SIZE));
        int checker = (cx + cz) & 1;

        vec3 floorDiffuse = (checker == 0) ? vec3(0.85) : vec3(0.15);
        vec3 colorOut = floorDiffuse * 0.2;

        for (int li = 0; li < lightCount; ++li)
        {
            vec3 lightPos = gpuSceneParams.lights[li].position.xyz;
            vec3 lightCol = gpuSceneParams.lights[li].color.xyz;
            float intensity = gpuSceneParams.lights[li].intensity.x;

            vec3 toLight = normalize(lightPos - hitPos);

            float dist = length(lightPos - hitPos);
            float attenuation = 1.0 / (1.0 + 0.02 * dist + 0.01 * dist * dist);

            float NdotL = max(dot(floorNormal, toLight), 0.0);

            // This helper is used only during raytrace reflections,
            // so always do the shadow test (nice result)
            if (!isInShadow(hitPos + floorNormal * 1e-3, lightPos, uint(-1)))
                colorOut += floorDiffuse * lightCol * NdotL * attenuation * intensity;
        }

        return colorOut;
    }

    //triangle hit
    Triangle tri = triangles[hitTriIndex];
    Material mat = tri.material;

    float u = closestHitPoint.x;
    float v = closestHitPoint.y;
    float w = 1.0 - u - v;

    vec3 N = normalize(w * tri.NA.xyz + u * tri.NB.xyz + v * tri.NC.xyz);

    vec3 diffuse = mat.diffuseColor.rgb;
    vec3 colorOut = diffuse * vec3(0.3, 0.3, 0.4);

    for (int li = 0; li < lightCount; ++li)
    {
        vec3 lightPos = gpuSceneParams.lights[li].position.xyz;
        vec3 lightCol = gpuSceneParams.lights[li].color.xyz;
        float intensity = gpuSceneParams.lights[li].intensity.x;

        vec3 toLight = normalize(lightPos - hitPos);

        float dist = length(lightPos - hitPos);
        float attenuation = 1.0 / (1.0 + 0.02 * dist + 0.01 * dist * dist);

        if (!isInShadow(hitPos + N * 1e-3, lightPos, hitTriIndex))
        {
            float NdotL = max(dot(N, toLight), 0.0);
            colorOut += diffuse * lightCol * NdotL * attenuation * intensity;
        }
    }

    return colorOut;
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

    // Iterate over all triangles
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

    bool hitFloor = false;
    float floorTHit = 1e30;

    if (intersectPlane(origin, dir, floorPoint, floorNormal, floorTHit))
    {
        if (floorTHit < distanceToClosestIntersection)
        {
            hitFloor = true;
            distanceToClosestIntersection = floorTHit;
        }
    }

    vec3 color = gpuSceneParams.backgroundColor.rgb;
    int lightCount = gpuSceneParams.lightMeta.x;
    lightCount = clamp(lightCount, 0, MAX_LIGHTS);

    if (hitFloor)
    {
        vec3 hitPos   = origin + dir * distanceToClosestIntersection;

        int cx = int(floor(hitPos.x / TILE_SIZE));
        int cz = int(floor(hitPos.z / TILE_SIZE));
        int checker = (cx + cz) & 1;

        vec3 floorDiffuse = (checker == 0)
            ? vec3(0.85)
            : vec3(0.15);

        vec3 colorOut = floorDiffuse * 0.2;

        for (int li = 0; li < lightCount; ++li) 
        {
            vec3 lightPos = gpuSceneParams.lights[li].position.xyz;
            vec3 lightCol = gpuSceneParams.lights[li].color.xyz;
            float intensity = gpuSceneParams.lights[li].intensity.x;

            vec3 toLight = normalize(lightPos - hitPos);

            float dist = length(lightPos - hitPos);
            float attenuation = 1.0 / (1.0 + 0.02 * dist + 0.01 * dist * dist);

            float NdotL = max(dot(floorNormal, toLight), 0.0);

            if(gpuSceneParams.isPreview.x == 1)
            {
                colorOut += floorDiffuse
                        * lightCol
                        * NdotL
                        * attenuation
                        * intensity;
            }
            else
            {
                //Shadow-test
                if (!isInShadow(hitPos + floorNormal * 1e-3, lightPos, uint(-1)))
                {
                    colorOut += floorDiffuse
                        * lightCol
                        * NdotL
                        * attenuation
                        * intensity;
                }
            }
       }
       if (gpuSceneParams.isPreview.x == 0.0)
    {
        float floorRefl = 0.45; 

        vec3 R = normalize(reflect(dir, floorNormal));
        vec3 ro2 = hitPos + floorNormal * 1e-3;

        vec3 reflCol = traceColor(ro2, R);

        float cosTheta = clamp(dot(-dir, floorNormal), 0.0, 1.0);
        float F = floorRefl + (1.0 - floorRefl) * pow(1.0 - cosTheta, 5.0);

        colorOut = mix(colorOut, reflCol, F);
    }
       color = colorOut;
    }
    else if (isHit)
    {
        Triangle tri = triangles[hitTriIndex];
        Material mat = tri.material;

        if(gpuSceneParams.isPreview.x == 1)
        {
            vec3 diffuse = mat.diffuseColor.rgb;
            vec3 c = diffuse;

            for (int li = 0; li < lightCount; ++li) 
            {
                vec3 lightCol = gpuSceneParams.lights[li].color.xyz;
                float intensity = gpuSceneParams.lights[li].intensity.x;
                c += lightCol * 0.2 * (intensity / 250.0);
            }
            color = c;
        }
        else
        {
            vec3 hitPos = origin + dir * distanceToClosestIntersection;

            float u = closestHitPoint.x;
            float v = closestHitPoint.y;
            float w = 1.0 - u - v;

            vec3 v0 = tri.v1.xyz;
            vec3 v1 = tri.v2.xyz;
            vec3 v2 = tri.v3.xyz;

            vec3 N = normalize(w * tri.NA.xyz + u * tri.NB.xyz + v * tri.NC.xyz);

            vec3 diffuse = mat.diffuseColor.rgb;

            vec3 ambientLighting = diffuse * vec3(0.3,0.3,0.4);
            color = ambientLighting;
            
            for (int li = 0; li < lightCount; ++li) 
            {
                vec3 lightPos = gpuSceneParams.lights[li].position.xyz;
                vec3 lightCol = gpuSceneParams.lights[li].color.xyz;
                float intensity = gpuSceneParams.lights[li].intensity.x;

                vec3 toLight = normalize(lightPos - hitPos);

                float d = length(lightPos - hitPos);
                float attenuation = 1.0 / (1.0 + 0.02 * d + 0.01 * d * d);

                if (!isInShadow(hitPos + N * 1e-3, lightPos, hitTriIndex))
                {
                    float NdotL = max(dot(N, toLight), 0.0);
                    color += diffuse * lightCol * NdotL * attenuation * intensity;
                }
            }
           }
    }
      
    imageStore(outputImage, pixel, vec4(color, 1.0));
}
