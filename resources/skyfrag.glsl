#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;

// ratio between day and night 0-1
uniform float day_night_ratio;
uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
	vec4 tcol = texture(tex, vertex_tex);
	vec4 tcol2 = texture(tex2, vertex_tex);

	// interpolate between day and night
	color = tcol * day_night_ratio + tcol2 * (1 - day_night_ratio);
}
