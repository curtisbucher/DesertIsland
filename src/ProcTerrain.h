#pragma  once

#ifndef PROCTERRAIN_H
#define PROCTERRAIN_H

#include "Texture.h"
#include "Program.h"
#include "GLSL.h"
#include <vector>
#include <iostream>
#include "Texture.h"

#define DEFAULT_TEX_ZOOM (1)
#define DEFAULT_MESH_SIZE (100)

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

class ProcTerrain {
    public:
        ProcTerrain();
        virtual ~ProcTerrain();
        void init(const shared_ptr<Program> shader, const char* texture_filename);
        void draw(glm::vec3 camera_pos);
        void drawPlane();
    private:
        // buffer objects for communicating with the GPU
        GLuint GrndBuffObj;
        GLuint GrndNorBuffObj;
        GLuint GrndTexBuffObj;
        GLuint GIndxBuffObj;
        GLuint GroundVertexArrayID;
        int g_GiboLen;
        // texture
        int tex_zoom;
        int mesh_size;
        // shader
        std::shared_ptr<Program> shader;
        std::shared_ptr<Texture> texture;
};

void SetModel(glm::vec3 trans, float rotY, float rotX, float sc, std::shared_ptr<Program> curS);

#endif