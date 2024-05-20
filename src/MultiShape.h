#pragma once
#ifndef _MULTI_SHAPE_H_
#define _MULTI_SHAPE_H_

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Shape.h"
#include "GLSL.h"
#include "Program.h"
#include "Texture.h"


class MultiShape
{
public:
	MultiShape(bool textured, const std::shared_ptr<Program> shader, const char* texture_filename) ;
	virtual ~MultiShape();
    bool loadObjFromFile(std::__1::string &errStr,
        const char* filename,
        const char *mtl_basepath = (const char *)__null);
	void addShape(tinyobj::shape_t & shape);
	void init();
	void measure();
    /* reset the stored translations*/
    void reset_trans();
    /* translate model */
    void translate(glm::vec3 trans);
    /* scale model */
    void scale(glm::vec3 scale);
    /* rotate model, `angle` in radians */
    void rotate(float angle, glm::vec3 axis);

    void center_and_scale();
	void draw();

	glm::vec3 min;
	glm::vec3 max;
    glm::vec3 extents;
    std::vector<std::shared_ptr<Shape>> shapes;

    glm::mat4 curr_mat;   // holds past rotation, scale etc.

    // for drawing
    std::shared_ptr<Program> shader;
    std::shared_ptr<Texture> texture;


private:
	bool texOff;
};

#endif
