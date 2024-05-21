#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

uniform vec3 camoff;

out vec2 vTexCoord;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;



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
  float height = noise(pos.xzy, 11, 0.5, 0.5);
  float baseheight = noise(pos.xzy, 4, 0.4, 0.3);
  baseheight = pow(baseheight, 5)*3;
  height = baseheight*height;
  return height;
  height*=60;
}

void main() {
  vec4 vPosition;

  /* First model transforms */
  // gl_Position = P * V * M * vec4(vertPos.x, get_height(vertPos.xzy), vertPos.z, 1.0);

  vec4 vp = vec4(vertPos.xyz - camoff, 1);

  /* get other 3 vertices */
  vec3 v1 = vp.xyz;
  vec3 v2 = vp.xyz + vec3(1.0, 0.0, 0.0);
  vec3 v3 = vp.xyz + vec3(0.0, 0.0, -1.0);

  vec3 pos1 = vec3(v1.x, get_height(v1.xzy), v1.z);
  vec3 pos2 = vec3(v2.x, get_height(v2.xzy), v2.z);
  vec3 pos3 = vec3(v3.x, get_height(v3.xzy), v3.z);
  // calculate normal
  vec3 normal = normalize(cross(pos2 - pos1, pos3 - pos1));
  fragNor = (M * vec4(normal, 0.0)).xyz;
  lightDir = lightPos - (M*vp).xyz;

  /* pass through the texture coordinates to be interpolated */
  // vTexCoord = vertTex;
  // EPos = (M*vertPos).xyz;

  // texture position
  vec4 tpos = vertPos;
	tpos.z -=camoff.z;
	tpos.x -=camoff.x;

	tpos =  M * tpos;

	tpos.y += get_height(vp.xzy);
	EPos = tpos.xyz;

	gl_Position = P * V * tpos;
	vTexCoord = vertTex;
}