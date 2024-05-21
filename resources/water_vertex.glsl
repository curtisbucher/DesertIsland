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

void main() {
  // calculate position
  vec4 vp = vec4(vertPos.xyz - camoff, 1);

  fragNor = vec3(0, -1, 0);
  lightDir = lightPos - (M*vp).xyz;

  // texture position
  vec4 tpos = vp;
	tpos =  M * tpos;

  // pass position to fragment shader
  EPos = tpos.xyz;
	gl_Position = P * V * tpos;
	vTexCoord = vertTex;
}