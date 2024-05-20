#include "MultiShape.h"
#include <iostream>
#include <assert.h>
#include <algorithm>

using namespace std;

// constructor
MultiShape::MultiShape(bool textured, const shared_ptr<Program> shader, const char* texture_filename) :
min(glm::vec3(0)),
max(glm::vec3(0))
{
    this->texOff = !textured;
    this->shader = shader;

    //read in a load the texture
    this->texture = make_shared<Texture>();
    this->texture->setFilename(texture_filename);
    this->texture->init();
    this->texture->setUnit(0);
    this->texture->setWrapModes(GL_REPEAT, GL_REPEAT);
}

// destructor
MultiShape::~MultiShape(){}

/* Load OBJ from file, add shapes, measure, initialize.
errStr : error string populated on failure
filename: the obj file
mtl_basepath: 'mtl_basepath' is optional, and used for base path for .mtl file.
Return true on success, false on failure*/
bool MultiShape::loadObjFromFile(std::__1::string &errStr, const char* filename, const char *mtl_basepath){
    std::vector<tinyobj::shape_t> TOshapes;
    std::__1::vector<tinyobj::material_t> objMaterials;

    bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, filename, mtl_basepath);
    if (!rc) {
        return false;
    }

    //for now all our shapes will not have textures - change in later labs
    for(auto s: TOshapes){
        this->addShape(s);
    }
    this->measure();
    this->init();

    return true;
}

/* copy the data from the MultiShape to this object */
void MultiShape::addShape(tinyobj::shape_t & shape)
{
    // create a new shape from the file
    shared_ptr<Shape> s = make_shared<Shape>();
    s->createShape(shape);
    // add to the list
    this->shapes.push_back(s);
}

void MultiShape::measure() {
    // measure all the shape objects, and take absolute min and absolute max
    for(auto shape: this->shapes){
        shape->measure();
        this->min.x = std::min(this->min.x, shape->min.x);
        this->min.y = std::min(this->min.y, shape->min.y);
        this->min.z = std::min(this->min.z, shape->min.z);

        this->max.x = std::max(this->max.x, shape->max.x);
        this->max.y = std::max(this->max.y, shape->max.y);
        this->max.z = std::max(this->max.z, shape->max.z);
    }

    // calculate the lengths of each edge
    this->extents.x = this->max.x - this->min.x;
    this->extents.y = this->max.y - this->min.y;
    this->extents.z = this->max.z - this->min.z;
}

/* reset curr mat to get rid of translations*/
void MultiShape::reset_trans(){
    this->curr_mat = glm::mat4(1.0f);
}

/* get the translation required to center the object*/
void MultiShape::center_and_scale(){
    // calculate the lengths of each edge
    this->extents.x = this->max.x - this->min.x;
    this->extents.y = this->max.y - this->min.y;
    this->extents.z = this->max.z - this->min.z;

    // find the center
    glm::vec3 center, trans;
    center.x = this->min.x + 0.5f * extents.x;
    center.y = this->min.y + 0.5f * extents.y;
    center.z = this->min.z + 0.5f * extents.z;

    trans.x = -center.x;
    trans.y = -center.y;
    trans.z = -center.z;
    // find the scaling factor
    float max_extent = std::max(this->extents.x, std::max(this->extents.y, this->extents.z));
    float scale = 2.0f / max_extent;

    // translate and scale
    glm::mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  	glm::mat4 ScaleS = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

    this->scale(glm::vec3(scale));
    this->translate(trans);
}

/* translate model */
void MultiShape::translate(glm::vec3 trans){
	this->curr_mat = glm::translate(this->curr_mat, trans);
}

/* scale model */
void MultiShape::scale(glm::vec3 scale){
    this->curr_mat = glm::scale(this->curr_mat, scale);
}

/* rotate model, `angle` in radians */
void MultiShape::rotate(float angle, glm::vec3 axis){
    this->curr_mat = glm::rotate(this->curr_mat, angle, axis);
}
void MultiShape::init()
{
    // initialize all the shapes
    for(auto shape: this->shapes){
        shape->init();
    }
    this->reset_trans();
}

//always untextured for intro labs until texture mapping
void MultiShape::draw()
{
    // bind the shader
    this->shader->bind();

    // apply the transform matrix
    glUniformMatrix4fv(this->shader->getUniform("M"), 1, GL_FALSE, glm::value_ptr(this->curr_mat));

    // bind the texture
    this->texture->bind(this->shader->getUniform("Texture0"));

    // draw all the shapes
    for(auto shape: this->shapes){
        shape->draw(this->shader);
    }
    // unbind the shader
    this->shader->unbind();
}