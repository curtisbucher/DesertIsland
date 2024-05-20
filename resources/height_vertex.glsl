#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

out vec2 vTexCoord;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;

float hash(float n) {
  return fract(sin(n) * 753.5453123);
}

float snoise(vec3 x){
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0 - 2.0 * f);

	float n = p.x + p.y * 157.0 + 113.0 * p.z;
	return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
		mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
		mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
			mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}

// Changing octaves, frequency and presistance results in a total different landscape.
float noise(vec3 position, int octaves, float frequency, float persistence) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += snoise(position * frequency) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
		}
	return total / maxAmplitude;
	}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
  float height = noise(vertPos.xzy, 11, 0.5, 0.5);
  float baseheight = noise(vertPos.xzy, 4, 0.4, 0.3);
  baseheight = pow(baseheight, 5)*3;
  height = baseheight*height;
  // height*=60;

  vec4 vPosition;

  /* First model transforms */
  gl_Position = P * V * M * vec4(vertPos.x, height, vertPos.z, 1.0);

  fragNor = (M * vec4(vertNor, 0.0)).xyz;
  lightDir = lightPos - (M*vertPos).xyz;

  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
  EPos = (M*vertPos).xyz;
}