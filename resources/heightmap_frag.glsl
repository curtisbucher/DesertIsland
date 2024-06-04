#version 330 core

in vec2 vTexCoord;
out vec4 color;

uniform vec3 camoff;


/* --- FOR HEIGHT CALCULATIONS, IF CHANGE HERE, CHANGE IN simplex.c --- */
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

  height *= PARAM_HEIGHT_SCALE;
  height -= PARAM_HEIGHT_SCALE/2;
  height = baseheight*height;
  return height;

}


void main() {
	// calculate position
	vec4 vp = vec4(vTexCoord.x, vTexCoord.y, 0, 1) * 100;

	// combine all components
	float height = (get_height(vp.xzy));
	color = vec4(vec3(height), 1.0);
}