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

	// get texture colors
	vec4 texColor0 = texture(Texture0, texcoords);
	vec4 texColor1 = texture(Texture1, texcoords);
	vec4 texColor2 = texture(Texture2, texcoords);
	vec4 texColor;

	// if height is above tex 1 line
	// if(EPos.y + biome_modifier < TEX1_MIN_HEIGHT){
	// 	texColor = texColor0;
	// }
	// else if(EPos.y + biome_modifier < TEX2_MIN_HEIGHT){
	// 	texColor = texColor1;
	// }
	// else {
	// 	texColor = texColor2;
	// }
	// normalize the texture based on height
	float height = EPos.y + biome_modifier;
	float texHeight = 0;
	if(height < TEX1_MIN_HEIGHT / 2){
		texColor = texColor0;
	}
	if(height < TEX1_MIN_HEIGHT){
		texHeight = (height) / (TEX1_MIN_HEIGHT);
		texColor = mix(texColor0, texColor1, texHeight);
	}
	else if(height < TEX2_MIN_HEIGHT){
		texHeight = (height - TEX1_MIN_HEIGHT) / (TEX2_MIN_HEIGHT - TEX1_MIN_HEIGHT);
		texColor = mix(texColor1, texColor2, texHeight);
	}
	else {
		texColor = texColor2;
	}


	// calculate lighting
	vec3 normal = normalize(fragNor);
	if (flip == 1) {
		normal = normal * -1.0f;
	}
	vec3 light = normalize(lightDir);
	float dC = clamp(dot(normal, light), 0, 1);
	vec3 halfV = normalize(-1*EPos) + normalize(light);
	float sC = pow(max(dot(normalize(halfV), normal), 0), shineIntensity);

	// calculate light components
	vec3 ambientLight = lightColor * ambientIntensity;
	vec3 diffuseLight = lightColor * dC;
	vec3 specularLight = lightColor * sC;

	// combine all components
	color = vec4(ambientLight + diffuseLight + specularLight, 1.0) * texColor;

	// fade out
	float len = length(EPos.xz+campos.xz);
	len-=41;
	len/=8.;
	len=clamp(len,0,1);
	color.a=1-len;
}