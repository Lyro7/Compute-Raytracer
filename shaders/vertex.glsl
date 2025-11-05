#version 430

// Fulscreen triangle
const vec2 verts[3] = vec2[3](
	vec2(-1.0, -1.0),
	vec2( 3.0, -1.0),
	vec2(-1.0,  3.0)
);

out vec2 coords;

void main() 
{
	vec2 pos = verts[gl_VertexID];
	gl_Position = vec4(pos, 0.0, 1.0);
	coords = 0.5 * pos + 0.5;
}