/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MultiShape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define START_CAMERA_TRANS vec3(-1.8,-0.8, -4.2)
#define START_CAMERA_ROT vec3(0.2, -0.8, 0)

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong
	std::shared_ptr<Program> prog;

	//Our shader program for textures
	std::shared_ptr<Program> texProg;

	//our geometry
	shared_ptr<MultiShape> palm_tree, sphere, theBunny, dummy;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
	shared_ptr<Texture> texture0, leaf_texture, water_texture, sand_texture;
	shared_ptr<Texture> tree1_texture;

	//global data (larger program should be encapsulated)
	vec3 gMin;
	vec3 camera_trans=START_CAMERA_TRANS;
	vec3 camera_rot=START_CAMERA_ROT;
	vec3 light_trans=vec3(-2, 2, 2);
	vec3 light_color=vec3(1, 1, 1);
	int vec_toggle = 0;
	float ambient_intensity = 0.2;
	float shine_intensity = 0.5
;
	//animation data
	float gTrans = -3;
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		// Rotation
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			camera_rot.y -= 0.2;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			camera_rot.y += 0.2;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			camera_rot.x -= 0.2;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			camera_rot.x += 0.2;
		}
		if (key == GLFW_KEY_Z&& action == GLFW_PRESS) {
			camera_trans.z -= 0.2;
		}
		if (key == GLFW_KEY_X && action == GLFW_PRESS) {
			camera_trans.z += 0.2;
		}
		// Translation
		if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
			camera_trans.y -= 0.2;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
			camera_trans.y += 0.2;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			camera_trans.x -= 0.2;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			camera_trans.x += 0.2;
		}
		// Light translation
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			light_trans.x -= 0.5;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS) {
			light_trans.x += 0.5;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		// Toggle Material on 'm'
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			vec_toggle = (vec_toggle + 1) % 6;
		}

	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//  --- GLSL PROGRAMS ---
		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("MatDif");
		prog->addUniform("lightPos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addUniform("flip");
		// to silence warning for drawing textured objects
		prog->addAttribute("vertTex");

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		// lighting
		texProg->addUniform("lightColor");
		texProg->addUniform("ambientIntensity");
		texProg->addUniform("shineIntensity");

		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("lightPos");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

		// -- TEXTURES ---
		//read in a load the texture
		water_texture = make_shared<Texture>();
  		water_texture->setFilename(resourceDirectory + "/water.jpeg");
  		water_texture->init();
  		water_texture->setUnit(0);
  		water_texture->setWrapModes(GL_REPEAT, GL_REPEAT);

		// load in sand texture
		sand_texture = make_shared<Texture>();
		sand_texture->setFilename(resourceDirectory + "/sand.jpg");
		sand_texture->init();
		sand_texture->setUnit(0);
		sand_texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// load in tree texture
		tree1_texture = make_shared<Texture>();
		tree1_texture->setFilename(resourceDirectory + "/objects/trees/_1_tree.jpg");
		tree1_texture->init();
		tree1_texture->setUnit(1);
		tree1_texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		// vector<tinyobj::shape_t> TOshapes;
 		// vector<tinyobj::material_t> objMaterials;
		string errStr;

		const std::string& objectDir = resourceDirectory + "/objects";

		//load in the mesh and make the shape(s)
		palm_tree = make_shared<MultiShape>(false);
		//bool rc = palm_tree->loadObjFromFile(errStr, (objectDir + "/palm_tree/palm_tree.obj").c_str());
		bool rc = palm_tree->loadObjFromFile(errStr, (objectDir + "/trees/tree1.obj").c_str());

		theBunny = make_shared<MultiShape>(false);
		rc = theBunny->loadObjFromFile(errStr, (objectDir + "/testing/bunnyNoNorm.obj").c_str());

		sphere = make_shared<MultiShape>(false);
		rc = sphere->loadObjFromFile(errStr, (objectDir + "/testing/SphereWTex.obj").c_str());

		dummy = make_shared<MultiShape>(false);
		rc = dummy->loadObjFromFile(errStr, (objectDir + "/testing/dummy.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
			exit(-1);
		}

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();
	}

	//directly pass quad for the ground to the GPU
	void initGround() {

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

      //code to draw the ground plane
     void drawGround(shared_ptr<Program> curS, shared_ptr<Texture> texture0) {
     	curS->bind();
     	glBindVertexArray(GroundVertexArrayID);
     	texture0->bind(curS->getUniform("Texture0"));
		//draw the ground plane
  		SetModel(vec3(0, -1, 0), 0, 0, 1, curS);
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

     //helper function to pass material data to the GPU
	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch ((i + vec_toggle) % 6) {
    		case 0: //shiny blue plastic
    			glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 120.0);
    		break;
    		case 1: // flat grey
    			glUniform3f(curS->getUniform("MatAmb"), 0.063, 0.038, 0.1);
    			glUniform3f(curS->getUniform("MatDif"), 0.63, 0.38, 1.0);
    			glUniform3f(curS->getUniform("MatSpec"), 0.3, 0.2, 0.5);
    			glUniform1f(curS->getUniform("MatShine"), 4.0);
    		break;
    		case 2: //brass
    			glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
    			glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
    			glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
			case 3: // shiny green
				glUniform3f(curS->getUniform("MatAmb"), 0.0215, 0.1745, 0.0215);
				glUniform3f(curS->getUniform("MatDif"), 0.07568, 0.61424, 0.07568);
				glUniform3f(curS->getUniform("MatSpec"), 0.633, 0.727811, 0.633);
				glUniform1f(curS->getUniform("MatShine"), 76.8);
			break;
			case 4: // skin
				glUniform3f(curS->getUniform("MatAmb"), 0.329412, 0.223529, 0.027451);
				glUniform3f(curS->getUniform("MatDif"), 0.780392, 0.568627, 0.113725);
				glUniform3f(curS->getUniform("MatSpec"), 0.992157, 0.941176, 0.807843);
				glUniform1f(curS->getUniform("MatShine"), 27.9);
			break;
			case 5: // black skin
				glUniform3f(curS->getUniform("MatAmb"), 0.02, 0.02, 0.02);
				glUniform3f(curS->getUniform("MatDif"), 0.01, 0.01, 0.01);
				glUniform3f(curS->getUniform("MatSpec"), 0.4, 0.4, 0.4);
				glUniform1f(curS->getUniform("MatShine"), 10.0);
			break;
  		}
	}

	/* helper function to set model trasnforms */
  	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}

	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
			View->loadIdentity();
			View->rotate(camera_rot.x, vec3(1,0,0));
			View->rotate(camera_rot.y, vec3(0,1,0));
			View->rotate(camera_rot.z, vec3(0,0,1));
			View->translate(camera_trans);

		// --- Draw Solid Colored Items ---
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		glUniform1i(prog->getUniform("flip"), 0);
		glUniform3f(prog->getUniform("lightPos"), light_trans.x, light_trans.y, light_trans.z);

		// draw the array of bunnies
		float sp = 3.0;
		float off = -3.5;
		  for (int i =0; i < 3; i++) {
		  	for (int j=0; j < 3; j++) {
				theBunny->reset_trans();
				theBunny->translate(vec3(off+sp*i, 0, off+sp*j));
				theBunny->center_and_scale();
				SetMaterial(prog, (i+j));
				theBunny->draw(prog);
			}
		  }

		//draw the dummy
		dummy->reset_trans();
		dummy->center_and_scale();
		// rotate by pi/2 rad
		dummy->rotate(-3.14159/2, vec3(1, 0, 0));
		dummy->translate(vec3(-100, -200, 5));
		SetMaterial(prog, 4);
		dummy->draw(prog);

		dummy->reset_trans();
		dummy->center_and_scale();
		// rotate by pi/2 rad
		dummy->rotate(-3.14159/2, vec3(1, 0, 0));
		dummy->translate(vec3(-200, -100, 5));
		SetMaterial(prog, 5);
		dummy->draw(prog);

		prog->unbind();


		// --- Draw Textured Items ---
		//switch shaders to the texture mapping shader and draw the ground
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		glUniform3f(texProg->getUniform("lightPos"), light_trans.x, light_trans.y, light_trans.z);
		glUniform3f(texProg->getUniform("lightColor"), light_color.r, light_color.g, light_color.b);
		glUniform1f(texProg->getUniform("ambientIntensity"), ambient_intensity);
		glUniform1f(texProg->getUniform("shineIntensity"), shine_intensity);

		//draw the palm tree
		glUniform1i(texProg->getUniform("flip"), 1);

		tree1_texture->bind(texProg->getUniform("Texture0"));
		palm_tree->reset_trans();
		palm_tree->center_and_scale();
		palm_tree->translate(vec3(0, 0, 0));
		palm_tree->scale(vec3(2));
		//SetMaterial(prog, 3);
		palm_tree->draw(texProg);

		glUniform1i(texProg->getUniform("flip"), 0);


		// draw the island
		sand_texture->bind(texProg->getUniform("Texture0"));
		sphere->reset_trans();
		sphere->center_and_scale();
		sphere->translate(vec3(0, -20, 0));
		sphere->scale(vec3(20));
		sphere->draw(texProg);

		texProg->unbind();

		//draw the ground plane
		drawGround(texProg, water_texture);

		//animation update example
		sTheta = sin(glfwGetTime());
		eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
		hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";


	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
