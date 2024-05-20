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

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {

  vec4 vPosition;

  /* First model transforms */
  gl_Position = P * V * M * vec4(vertPos.x, rand(vertPos.xz), vertPos.z, 1.0);

  fragNor = (M * vec4(vertNor, 0.0)).xyz;
  lightDir = lightPos - (M*vertPos).xyz;

  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
  EPos = (M*vertPos).xyz;
}