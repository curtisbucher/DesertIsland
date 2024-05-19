#pragma  once

#ifndef PROCTERRAIN_H
#define PROCTERRAIN_H

#include "Texture.h"
#include "Program.h"
#include "GLSL.h"
#include <vector>
#include <iostream>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ProcTerrain {
    public:
        ProcTerrain();
        virtual ~ProcTerrain();
        void init();
        void draw(std::shared_ptr<Program> curS, std::shared_ptr<Texture> texture0);
    private:
        GLuint GrndBuffObj;
        GLuint GrndNorBuffObj;
        GLuint GrndTexBuffObj;
        GLuint GIndxBuffObj;
        GLuint GroundVertexArrayID;
        int g_GiboLen;
};

void SetModel(glm::vec3 trans, float rotY, float rotX, float sc, std::shared_ptr<Program> curS);

#endif