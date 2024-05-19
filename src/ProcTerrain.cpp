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

void ProcTerrain::init()
{
    GLfloat tex_zoom = 20;
    float g_groundSize = 20;
    float g_groundY = -0.25;

    // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
        -g_groundSize, g_groundY, -g_groundSize,
        -g_groundSize, g_groundY,  g_groundSize,
        g_groundSize, g_groundY,  g_groundSize,
        g_groundSize, g_groundY, -g_groundSize
    };

    float GrndNorm[] = {
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0
    };

    static GLfloat GrndTex[] = {
        0, 0, // back
        0, tex_zoom,
        tex_zoom, tex_zoom,
        tex_zoom, 0 };

    unsigned short idx[] = {0, 1, 2, 0, 2, 3};

    //generate the ground VAO
    glGenVertexArrays(1, &GroundVertexArrayID);
    glBindVertexArray(GroundVertexArrayID);

    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndNorBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}

// randomely generate terrain
void ProcTerrain::draw(std::shared_ptr<Program> curS, std::shared_ptr<Texture> texture0){
    curS->bind();
    glBindVertexArray(GroundVertexArrayID);
    texture0->bind(curS->getUniform("Texture0"));

    //draw the ground plane
    SetModel(glm::vec3(0, -1, 0), 0, 0, 1, curS);

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

