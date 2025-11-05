#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform image2D outputImage;

struct Vertex 
{
	vec4 pos;
	vec4 normal;
	vec2 uv;
};

// Mesh data
layout(std430, binding = 1) buffer VertexBuffer 
{
	Vertex vertices[];
};

layout(std430, binding = 2) buffer IndexBuffer 
{
	uint indices[];
};

// Camera data...

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