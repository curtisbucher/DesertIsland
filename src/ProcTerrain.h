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
        void init(const shared_ptr<Program> shader, const vector<std::string>texture_filenames);
        void draw(glm::vec3 camera_pos);
        void drawPlane(const shared_ptr<Program> shader, const shared_ptr<Texture> texture, glm::vec3 camera_pos);
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
        std::vector<std::shared_ptr<Texture>> textures;
};

void SetModel(glm::vec3 trans, float rotY, float rotX, float sc, std::shared_ptr<Program> curS);

#endif