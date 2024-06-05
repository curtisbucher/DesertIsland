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

    shader(nullptr)
{
}


ProcTerrain::~ProcTerrain()
{
}


void ProcTerrain::init(const shared_ptr<Program> shader, const vector<std::string> texture_filenames)
{
    // init the quad
    initQuad();
    // store the shader
    this->shader = shader;
    // load the textures
    int i = 0;
    for(auto t_fname : texture_filenames){
        this->textures.push_back(make_shared<Texture>());
        this->textures.back()->setFilename(t_fname);
        this->textures.back()->init();
        this->textures.back()->setUnit(i++);
        this->textures.back()->setWrapModes(GL_REPEAT, GL_REPEAT);
    }

    // // the dimensions of the grid (in points)
    // int points_w = 20;
    // int points_h = 20;

    // int num_points = points_w * points_h;
    // int num_tris = (points_w - 1) * (points_h - 1) * 2;

    // GLfloat tex_zoom = 20;
    // float g_groundSize = 20;
    // float g_groundY = -0.25;

    // // TODO: programmatically fill in positions, and idxs.
    // float* GrndPos = (float*) malloc(sizeof(float) * num_points * 3);
    // float* GrndNorm = (float*) malloc(sizeof(float) * num_points * 3);
    // static GLfloat* GrndTex = (float*) malloc(sizeof(float) * num_points * 2);

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

    // heightmap stuff
    glGenFramebuffers(1, &this->frameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuf);

    // Create texture to store the height map
    // GLuint this->texBuf;
    glGenTextures(1, &(this->texBuf));
    glBindTexture(GL_TEXTURE_2D, this->texBuf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->mesh_size, this->mesh_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texBuf, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // Handle framebuffer not complete
        std::cerr << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

//     //make an FBO and a textures
//     glGenFramebuffers(1, &this->frameBuf);
//     glGenTextures(1, &this->texBuf);
//     glGenRenderbuffers(1, &this->depthBuf);

//     //create one FBO
//     createFBO(this->frameBuf, this->texBuf, this->mesh_size, this->mesh_size);
}

// randomely generate terrain
void ProcTerrain::draw(glm::vec3 camera_pos){
    // // generate the heightmap
    // this->gen_heightmap(camera_pos, this->shader);

    this->shader->bind();
    glBindVertexArray(GroundVertexArrayID);
    this->textures.at(0)->bind(this->shader->getUniform("Texture0"));
    this->textures.at(1)->bind(this->shader->getUniform("Texture1"));
    this->textures.at(2)->bind(this->shader->getUniform("Texture2"));

    /* --- bind the heightmap texture --- */
    int unit = 4;
    // set wrap modes
    glBindTexture(GL_TEXTURE_2D, this->texBuf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // bind
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, this->texBuf);
	glUniform1i(3, unit);

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

    // // draw!
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

    // // render to screem (framebuff = 0)
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // // glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);
    // // Render full-screen quad or plane
    // glBindVertexArray(quad_VertexArrayID);
    // glDrawArrays(GL_TRIANGLES, 0, 6);

    // glDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);

    // unbind heightmap texture
    glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, 0);

    this->shader->unbind();
}

/**** geometry set up for a quad *****/
void ProcTerrain::initQuad() {
    //now set up a simple quad for rendering FBO

    float quadVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    // Generate and bind the VAO and VBO
    GLuint quadVBO;
    glGenVertexArrays(1, &(this->quad_VertexArrayID));
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(this->quad_VertexArrayID);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

}

void ProcTerrain::gen_heightmap(glm::vec3 camera_pos, const shared_ptr<Program> heightmap_shader){
    // Render the height map using the simplex noise shader to the texture attached to the FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuf);
    // get the current viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // Set the viewport to the size of the texture
    glViewport(0, 0, this->mesh_size, this->mesh_size);

    heightmap_shader->bind();

    // Render full-screen quad or plane
    glBindVertexArray(this->quad_VertexArrayID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    heightmap_shader->unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    static int firsttime = 1;
    if (firsttime) {
    assert(GLTextureWriter::WriteImage(this->texBuf,"Texture_output.png"));
    firsttime = 0;
    }

    // clean up
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);
    glUseProgram(0);
    // Reset viewport to window size
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

// Draw a plane, for the waterline
void ProcTerrain::drawPlane(const shared_ptr<Program> plane_shader, const shared_ptr<Texture> texture, glm::vec3 camera_pos) {
    // draw plane at y = 0, using same xy coords
    plane_shader->bind();
    glBindVertexArray(GroundVertexArrayID);
    texture->bind(plane_shader->getUniform("Texture0"));

    // center the ground plane
    SetModel(glm::vec3(-this->mesh_size/2, 0, -this->mesh_size/2), 0, 0, 1, plane_shader);

    // pass camera position to shader
    glm::vec3 offset = camera_pos;
    offset.y = 0;
    offset.x = (int)offset.x;
    offset.z = (int)offset.z;
    // for calculating vertices, decimal
    glUniform3fv(plane_shader->getUniform("camoff"), 1, &offset[0]);
    // for calculating color, float
    glUniform3fv(plane_shader->getUniform("campos"), 1, &camera_pos[0]);
    // pass mesh size to shader
    glUniform1i(plane_shader->getUniform("mesh_size"), this->mesh_size);
    // pass tex zoom to shader
    glUniform1i(plane_shader->getUniform("tex_zoom"), this->tex_zoom);

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
    plane_shader->unbind();
}

/* get the height at an xy position */
float ProcTerrain::get_altitude(glm::vec3 pos, glm::vec3 camera_pos){
    // get reletive position from camera
    glm::vec3 rel_pos = pos - camera_pos;
    // get the height at the position
    return get_height(rel_pos);
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

/*
Helper function to create the framebuffer object and associated texture to write to
*/
void createFBO(GLuint& fb, GLuint& tex, int width, int height) {
    //set up framebuffer
     glBindFramebuffer(GL_FRAMEBUFFER, fb);
     //set up texture
     glBindTexture(GL_TEXTURE_2D, tex);

     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

     if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
       cout << "Error setting up frame buffer - exiting" << endl;
      exit(0);
    }
  }