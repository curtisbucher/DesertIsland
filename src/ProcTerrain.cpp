#include "ProcTerrain.h"


ProcTerrain::ProcTerrain() :
    GrndBuffObj(0),
    GrndNorBuffObj(0),
    GrndTexBuffObj(0),
    GIndxBuffObj(0),
    GroundVertexArrayID(0),
    g_GiboLen(0),

    tex_zoom(DEFAULT_TEX_ZOOM),
    mesh_size(DEFAULT_MESH_SIZE),

    shader(nullptr),
    texture(nullptr)
{
}


ProcTerrain::~ProcTerrain()
{
}

#define MESH_SIZE 100
void ProcTerrain::init(const shared_ptr<Program> shader, const char* texture_filename)
{
    // store the shader
    this->shader = shader;
    // load the texture
    this->texture = make_shared<Texture>();
    this->texture->setFilename(texture_filename);
    this->texture->init();
    this->texture->setUnit(0);
    this->texture->setWrapModes(GL_REPEAT, GL_REPEAT);

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

    glm::vec3 vertices[MESH_SIZE*MESH_SIZE * 4];
    for(int x=0;x<this->mesh_size;x++)
        for (int z = 0; z < this->mesh_size; z++)
            {
            vertices[x * 4 + z*this->mesh_size * 4 + 0] = glm::vec3(0.0, 0.0, 0.0) + glm::vec3(x, 0, z);
            vertices[x * 4 + z*this->mesh_size * 4 + 1] = glm::vec3(1, 0.0, 0.0) + glm::vec3(x, 0, z);
            vertices[x * 4 + z*this->mesh_size * 4 + 2] = glm::vec3(1, 0.0, 1) + glm::vec3(x, 0, z);
            vertices[x * 4 + z*this->mesh_size * 4 + 3] = glm::vec3(0.0, 0.0, 1) + glm::vec3(x, 0, z);
            }

    //tex coords
    float t = 1. / 1;
    glm::vec2 tex[MESH_SIZE * MESH_SIZE * 4];
    for (int x = 0; x<this->mesh_size; x++)
        for (int y = 0; y < this->mesh_size; y++)
        {
            tex[x * 4 + y*this->mesh_size * 4 + 0] = glm::vec2(0.0, 0.0)+ glm::vec2(x, y)*t;
            tex[x * 4 + y*this->mesh_size * 4 + 1] = glm::vec2(t, 0.0)+ glm::vec2(x, y)*t;
            tex[x * 4 + y*this->mesh_size * 4 + 2] = glm::vec2(t, t)+ glm::vec2(x, y)*t;
            tex[x * 4 + y*this->mesh_size * 4 + 3] = glm::vec2(0.0, t)+ glm::vec2(x, y)*t;
        }

    // elements
    GLushort elements[MESH_SIZE * MESH_SIZE * 6];
    int ind = 0;
    for (int i = 0; i<this->mesh_size * this->mesh_size * 6; i+=6, ind+=4){
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

    g_GiboLen = this->mesh_size * this->mesh_size * 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * this->mesh_size * this->mesh_size * 4, vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->mesh_size * this->mesh_size * 4, tex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*this->mesh_size * this->mesh_size * 6, elements, GL_STATIC_DRAW);
}

// randomely generate terrain
void ProcTerrain::draw(glm::vec3 camera_pos){
    this->shader->bind();
    glBindVertexArray(GroundVertexArrayID);
    this->texture->bind(this->shader->getUniform("Texture0"));

    // center the ground plane
    SetModel(glm::vec3(-this->mesh_size/2, 0, -this->mesh_size/2), 0, 0, 1, this->shader);

    // pass camera position to shader
    glm::vec3 offset = camera_pos;
    offset.y = 0;
    offset.x = (int)offset.x;
    offset.z = (int)offset.z;
    // for calculating vertices, decimal
    glUniform3fv(this->shader->getUniform("camoff"), 1, &offset[0]);
    // for calculating color, float
    glUniform3fv(this->shader->getUniform("campos"), 1, &camera_pos[0]);
    // pass mesh size to shader
    glUniform1i(this->shader->getUniform("mesh_size"), this->mesh_size);
    // pass tex zoom to shader
    glUniform1i(this->shader->getUniform("tex_zoom"), this->tex_zoom);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // draw!
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    this->shader->unbind();
}

// Draw a plane, for the waterline
void ProcTerrain::drawPlane(const shared_ptr<Program> shader, const shared_ptr<Texture> texture, glm::vec3 camera_pos) {
    // draw plane at y = 0, using same xy coords
    shader->bind();
    glBindVertexArray(GroundVertexArrayID);
    texture->bind(shader->getUniform("Texture0"));

    // center the ground plane
    SetModel(glm::vec3(-this->mesh_size/2, 0, -this->mesh_size/2), 0, 0, 1, shader);

    // pass camera position to shader
    glm::vec3 offset = camera_pos;
    offset.y = 0;
    offset.x = (int)offset.x;
    offset.z = (int)offset.z;
    // for calculating vertices, decimal
    glUniform3fv(shader->getUniform("camoff"), 1, &offset[0]);
    // for calculating color, float
    glUniform3fv(shader->getUniform("campos"), 1, &camera_pos[0]);
    // pass mesh size to shader
    glUniform1i(shader->getUniform("mesh_size"), this->mesh_size);
    // pass tex zoom to shader
    glUniform1i(shader->getUniform("tex_zoom"), this->tex_zoom);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // draw!
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    shader->unbind();
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

