#include "ProcTerrain.h"


ProcTerrain::ProcTerrain() :
    GrndBuffObj(0),
    GrndNorBuffObj(0),
    GrndTexBuffObj(0),
    GIndxBuffObj(0),
    GroundVertexArrayID(0),
    g_GiboLen(0)
{
}

ProcTerrain::~ProcTerrain()
{
}

#define MESHSIZE 100
void ProcTerrain::init()
{
    // the dimensions of the grid (in points)
    int points_w = 20;
    int points_h = 20;

    int num_points = points_w * points_h;
    int num_tris = (points_w - 1) * (points_h - 1) * 2;

    GLfloat tex_zoom = 20;
    float g_groundSize = 20;
    float g_groundY = -0.25;

    // TODO: programmatically fill in positions, and idxs.
    float* GrndPos = (float*) malloc(sizeof(float) * num_points * 3);
    float* GrndNorm = (float*) malloc(sizeof(float) * num_points * 3);
    static GLfloat* GrndTex = (float*) malloc(sizeof(float) * num_points * 2);

    glm::vec3 vertices[MESHSIZE * MESHSIZE * 4];
    for(int x=0;x<MESHSIZE;x++)
        for (int z = 0; z < MESHSIZE; z++)
            {
            vertices[x * 4 + z*MESHSIZE * 4 + 0] = glm::vec3(0.0, 0.0, 0.0) + glm::vec3(x, 0, z);
            vertices[x * 4 + z*MESHSIZE * 4 + 1] = glm::vec3(1, 0.0, 0.0) + glm::vec3(x, 0, z);
            vertices[x * 4 + z*MESHSIZE * 4 + 2] = glm::vec3(1, 0.0, 1) + glm::vec3(x, 0, z);
            vertices[x * 4 + z*MESHSIZE * 4 + 3] = glm::vec3(0.0, 0.0, 1) + glm::vec3(x, 0, z);
            }

    //tex coords
    float t = 1. / 1;
    glm::vec2 tex[MESHSIZE * MESHSIZE * 4];
    for (int x = 0; x<MESHSIZE; x++)
        for (int y = 0; y < MESHSIZE; y++)
        {
            tex[x * 4 + y*MESHSIZE * 4 + 0] = glm::vec2(0.0, 0.0)+ glm::vec2(x, y)*t;
            tex[x * 4 + y*MESHSIZE * 4 + 1] = glm::vec2(t, 0.0)+ glm::vec2(x, y)*t;
            tex[x * 4 + y*MESHSIZE * 4 + 2] = glm::vec2(t, t)+ glm::vec2(x, y)*t;
            tex[x * 4 + y*MESHSIZE * 4 + 3] = glm::vec2(0.0, t)+ glm::vec2(x, y)*t;
        }

    for(int i = 0; i < points_w; i++) {
        for(int j = 0; j < points_h; j++) {
            // // normal
            GrndNorm[i * points_h + j] = 0;
            GrndNorm[i * points_h + j + 1] = 1;
            GrndNorm[i * points_h + j + 2] = 0;
        }
    }

    // elements
    GLushort elements[MESHSIZE * MESHSIZE * 6];
    int ind = 0;
    for (int i = 0; i<MESHSIZE * MESHSIZE * 6; i+=6, ind+=4){
        elements[i + 0] = ind + 0;
        elements[i + 1] = ind + 1;
        elements[i + 2] = ind + 2;
        elements[i + 3] = ind + 0;
        elements[i + 4] = ind + 2;
        elements[i + 5] = ind + 3;
    }

    //generate the ground VAO
    glGenVertexArrays(1, &GroundVertexArrayID);
    glBindVertexArray(GroundVertexArrayID);

    g_GiboLen = MESHSIZE * MESHSIZE * 6;//6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * MESHSIZE * MESHSIZE * 4, vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &GrndNorBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * MESHSIZE * MESHSIZE * 4, tex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*MESHSIZE * MESHSIZE * 6, elements, GL_STATIC_DRAW);
}

// randomely generate terrain
void ProcTerrain::draw(std::shared_ptr<Program> curS, std::shared_ptr<Texture> texture0, glm::vec3 camera_pos){
    curS->bind();
    glBindVertexArray(GroundVertexArrayID);
    texture0->bind(curS->getUniform("Texture0"));

    // center the ground plane
    SetModel(glm::vec3(-MESHSIZE/2, 0, -MESHSIZE/2), 0, 0, 1, curS);

    // pass camera position to shader
    glm::vec3 offset = camera_pos;
    offset.y = 0;
    offset.x = (int)offset.x;
    offset.z = (int)offset.z;
    // for calculating vertices, decimal
    glUniform3fv(curS->getUniform("camoff"), 1, &offset[0]);
    // for calculating color, float
    glUniform3fv(curS->getUniform("campos"), 1, &camera_pos[0]);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // draw!
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    curS->unbind();
}

/* helper function to set model trasnforms */
void SetModel(glm::vec3 trans, float rotY, float rotX, float sc, std::shared_ptr<Program> curS) {
    glm::mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
    glm::mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, glm::vec3(1, 0, 0));
    glm::mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, glm::vec3(0, 1, 0));
    glm::mat4 ScaleS = glm::scale(glm::mat4(1.0f), glm::vec3(sc));
    glm::mat4 ctm = Trans*RotX*RotY*ScaleS;
    glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
}

