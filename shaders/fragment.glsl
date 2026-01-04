#version 430

in vec4 vColor;        // interpolierte Vertex-Farbe
in vec4 vWorldPos;     // für späteres Licht, optional

struct CameraParams {
    mat4 viewProj;
    vec4 origin;
    vec4 lowerLeft;
    vec4 horizontal;
    vec4 vertical;
};

struct LightParams {
    vec4 intensity; //only .x used and its between [0,1]
    vec4 color;
};

layout(std140, binding = 0) uniform SceneBlock
{
    CameraParams camera;
    LightParams light;
};

out vec4 FragColor;

void main()
{
    // Intensität des Lichts (0..1)
    float intensity = light.intensity.x;

    // Lichtfarbe
    vec3 lightCol = light.color.rgb;

    // Kombination Vertex-Farbe + Licht
    vec3 finalColor = vColor.rgb * lightCol * intensity;

    FragColor = vec4(finalColor, 1.0);
}
