#version 330 core
uniform sampler2D Texture0;

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

void main() {
	// get texture color
	vec4 texColor0 = texture(Texture0, vTexCoord);

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
	color = vec4(ambientLight + diffuseLight + specularLight, 1.0) * texColor0;
}