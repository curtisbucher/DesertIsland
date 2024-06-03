#include "noise.h"

// myhash function to generate random numbers
float myhash(float n) {
  return glm::fract(sin(n) * 753.5453123);
}

float snoise(glm::vec3 x){
	glm::vec3 p = floor(x);
	glm::vec3 f = fract(x);
	// f = f * f * (3.0 - 2.0 * f);
	f = f * f * (glm::vec3(3.0) - 2.0f * f);

	float n = p.x + p.y * 157.0 + 113.0 * p.z;
	return mix(mix(mix(myhash(n + 0.0), myhash(n + 1.0), f.x),
		mix(myhash(n + 157.0), myhash(n + 158.0), f.x), f.y),
		mix(mix(myhash(n + 113.0), myhash(n + 114.0), f.x),
			mix(myhash(n + 270.0), myhash(n + 271.0), f.x), f.y), f.z);
}

// Changing octaves, frequency and presistance results in a total different landscape.
float noise(glm::vec3 position, int octaves, float frequency, float persistence) {
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
float get_height(glm::vec3 pos){
//   float height = noise(pos.xzy,
	float height = noise(vec3(pos),
    PARAM_HEIGHT_OCTAVES,
    PARAM_HEIGHT_FREQUENCY,
    PARAM_HEIGHT_PERSISTENCE);
//   float baseheight = noise(pos.xzy,
	float baseheight = noise(vec3(pos),
    PARAM_BASE_HEIGHT_OCTAVES,
    PARAM_BASE_HEIGHT_FREQUENCY,
    PARAM_BASE_HEIGHT_PERSISTENCE);
  baseheight = pow(baseheight, PARAM_BASE_HEIGHT_POW)*3;

  height *= PARAM_HEIGHT_SCALE;
  height -= PARAM_HEIGHT_SCALE/2;
  height = baseheight*height;
  return height;
}