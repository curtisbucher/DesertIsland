#version 330 core

#define TEX1_MIN_HEIGHT (2)
#define TEX2_MIN_HEIGHT (7)

// texture image
uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform sampler2D Texture2;

// color and normal of the fragment
uniform int flip;
uniform vec3 lightColor;
uniform float ambientIntensity;
uniform float shineIntensity;

in vec2 vTexCoord;
out vec4 color;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
in vec3 EPos;
in float biome_modifier;

//camera offset
uniform vec3 camoff;
uniform vec3 campos;

uniform int tex_zoom;
uniform int mesh_size;

void main() {

	vec2 texcoords=vTexCoord;
	float t=1. / tex_zoom;
	texcoords -= vec2(camoff.x,camoff.z)*t;

	// normalize the texture based on height
	float height = EPos.y + biome_modifier;

	// combine all components
	color = vec4(vec3(0), 1.0);
}