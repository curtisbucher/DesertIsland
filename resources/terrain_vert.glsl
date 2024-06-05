#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

// the generated heightmap
uniform sampler2D heightmap;

uniform vec3 lightPos;
uniform vec3 camoff;

out vec2 vTexCoord;
out vec3 fragNor;
out vec3 lightDir;
// the output vertex position
out vec3 EPos;
// a modifier for altitude to make biomes occur at varying levels
out float biome_modifier;

void main() {
    // calculate position
    vec4 vp = vec4(vertPos.xyz - camoff, 1);

    /* get other 3 vertices */
    vec3 v1 = vp.xyz;
    vec3 v2 = vp.xyz + vec3(1.0, 0.0, 0.0);
    vec3 v3 = vp.xyz + vec3(0.0, 0.0, -1.0);

    /* calculate normal */
    vec3 pos1 = vec3(v1.x, texture(heightmap, v1.xz).r, v1.z);
    vec3 pos2 = vec3(v2.x,texture(heightmap, v2.xz).r, v2.z);
    vec3 pos3 = vec3(v3.x, texture(heightmap, v3.xz).r, v3.z);

    vec3 normal = normalize(cross(pos2 - pos1, pos3 - pos1));
    fragNor = (M * vec4(normal, 0.0)).xyz;
    lightDir = lightPos - (M*vp).xyz;

    // texture position
    vec4 tpos = vp;
    tpos =  M * tpos;
    float height = texture(heightmap, tpos.xz / 100.0).r * 10;
    tpos.y += height;

    // pass position to fragment shader
    EPos = tpos.xyz;
    gl_Position = P * V * tpos;
    vTexCoord = vertTex;

    // pass biome modifier
    biome_modifier = 0;//2 * (0.5 - noise(-v1.xzy, 8, 0.2, 0.1));
}