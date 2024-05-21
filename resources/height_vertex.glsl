#version  330 core
layout(location = 0) in vec4 vertPos;
// layout(location = 1) in vec3 vertNor;
layout(location = 1) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

uniform vec3 camoff;

out vec2 vTexCoord;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;



// Default values
#define PARAM_HEIGHT_OCTAVES (8)
#define PARAM_HEIGHT_FREQUENCY (0.02)
#define PARAM_HEIGHT_PERSISTENCE (0.5)
#define PARAM_BASE_HEIGHT_OCTAVES (8)
#define PARAM_BASE_HEIGHT_FREQUENCY (0.02)
#define PARAM_BASE_HEIGHT_PERSISTENCE (0.5)

#define PARAM_BASE_HEIGHT_POW (5)

#define PARAM_HEIGHT_SCALE (60)
#define PARAM_HEIGHT_TRANS (-10)

// Hash function to generate random numbers
float hash(float n) {
  return fract(sin(n) * 753.5453123);
}

// Simplex noise function
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

// Function to get the height of the terrain
float get_height(vec3 pos){
  float height = noise(pos.xzy,
    PARAM_HEIGHT_OCTAVES,
    PARAM_HEIGHT_FREQUENCY,
    PARAM_HEIGHT_PERSISTENCE);
  float baseheight = noise(pos.xzy,
    PARAM_BASE_HEIGHT_OCTAVES,
    PARAM_BASE_HEIGHT_FREQUENCY,
    PARAM_BASE_HEIGHT_PERSISTENCE);
  baseheight = pow(baseheight, PARAM_BASE_HEIGHT_POW)*3;
  height = baseheight*height;
  height *= PARAM_HEIGHT_SCALE - (0.5 * PARAM_HEIGHT_SCALE) + PARAM_HEIGHT_TRANS;
  return height;

}

void main() {
  // calculate position
  vec4 vp = vec4(vertPos.xyz - camoff, 1);

  /* get other 3 vertices */
  vec3 v1 = vp.xyz;
  vec3 v2 = vp.xyz + vec3(1.0, 0.0, 0.0);
  vec3 v3 = vp.xyz + vec3(0.0, 0.0, -1.0);

  /* calculate normal */
  vec3 pos1 = vec3(v1.x, get_height(v1.xzy), v1.z);
  vec3 pos2 = vec3(v2.x, get_height(v2.xzy), v2.z);
  vec3 pos3 = vec3(v3.x, get_height(v3.xzy), v3.z);

  vec3 normal = normalize(cross(pos2 - pos1, pos3 - pos1));
  fragNor = (M * vec4(normal, 0.0)).xyz;
  lightDir = lightPos - (M*vp).xyz;

  // texture position
  vec4 tpos = vp;
	tpos =  M * tpos;
	tpos.y += get_height(vp.xzy);

  // pass position to fragment shader
  EPos = tpos.xyz;
	gl_Position = P * V * tpos;
	vTexCoord = vertTex;
}