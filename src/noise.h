#ifndef NOISE_H
#define NOISE_H

#include "GLSL.h"
#include <glad/glad.h>
#include <glm/glm.hpp> // Include the necessary header file for glm::vec3
#include <glm/gtc/matrix_transform.hpp>


using namespace std;
using namespace glm;


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

float myhash(float n);
float snoise(glm::vec3 x);
// Changing octaves, frequency and presistance results in a total different landscape.
float noise(glm::vec3 position, int octaves, float frequency, float persistence);
// Function to get the height of the terrain
float get_height(glm::vec3 pos);

#endif