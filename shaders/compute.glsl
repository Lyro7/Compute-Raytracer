#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform image2D outputImage;

struct Vertex {
	vec3 pos;
	vec3 normal;
	vec2 uv;
};

// Mesh data
layout(std430, binding = 1) buffer VertexBuffer {
	Vertex vertices[];
};

layout(std430, binding = 2) buffer IndexBuffer {
	uint indices[];
};

// Camera data...

void main() {
	// Do work
}