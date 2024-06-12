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
#include "ProcTerrain.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define START_CAMERA_TRANS vec3(3,3,3)
#define START_CAMERA_ROT vec3(10, 10, 10)
#define CAMERA_SPEED 0.5
#define CAMERA_ROT_SPEED 0.3

// the duration of a day in seconds
#define DAY_DURATION_S (1 * 60)

#define MAX_PHI (80.0)

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

class camera
{
public:
	glm::vec3 pos;
	float rotAngle;
	float phi, theta;
	int w, a, s, d;

	camera()
	{
		w = a = s = d = 0;
		rotAngle = START_CAMERA_ROT.x;//0.0;
		phi = START_CAMERA_ROT.y;
		theta = START_CAMERA_ROT.z;
		pos = START_CAMERA_TRANS;//glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float f_speed = 0;
		if (w == 1)
			f_speed = 10*ftime;
		else if (s == 1)
			f_speed = -10*ftime;

		float r_speed=0;
		if (a == 1)
			r_speed = -3*ftime;
		else if(d==1)
			r_speed = 3*ftime;

		vec3 front;
		front.x = cos(glm::radians(theta)) * cos(glm::radians(phi));
		front.y = -1 * sin(glm::radians(phi));
		front.z = sin(glm::radians(theta)) * cos(glm::radians(phi));

		vec3 right = normalize(cross(front, vec3(0, 1, 0)));
		vec3 up = normalize(cross(right, front));

		pos += f_speed * front;
		pos += r_speed * right;

		return glm::lookAt(pos, pos + front, up);
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong
	std::shared_ptr<Program> prog, texProg, heightShader, skysphere_shader, water_shader;

	//our geometry
	shared_ptr<MultiShape> tree1, campfire, rock;
	shared_ptr<SkySphere> skysphere;
	shared_ptr<Shape> mesh;

	// Terrain Generation
	ProcTerrain ground;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
	shared_ptr<Texture> texture0, water_texture, textureDaySky, textureNightSky;

	//global data (larger program should be encapsulated)
	vec3 gMin;
	vec3 light_trans=vec3(0, 4.5, 0);
	vec3 light_color=vec3(1, 1, 1);
	int vec_toggle = 0;
	float ambient_intensity = 0.6;
	float shine_intensity = 0.7;
	//animation data
	float gTrans = -3;
	float sTheta = 0;

	void scrollCallback(GLFWwindow * window, double in_deltaX, double in_deltaY){
		// TODO: implement
		mycam.phi = clamp(mycam.phi + (in_deltaY * 10), -MAX_PHI, MAX_PHI);
		mycam.theta = mycam.theta + (in_deltaX * 10);
	}

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}

		// Light translation
		if (key == GLFW_KEY_Q) {
			light_trans.x -= 0.5;
		}
		if (key == GLFW_KEY_E) {
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
		if(!prog->init()) {
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
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

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		if(!texProg->init()) {
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
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

		// Initialize the GLSL program that we will use for texture mapping
		heightShader = make_shared<Program>();
		heightShader->setVerbose(true);
		heightShader->setShaderNames(resourceDirectory + "/height_vertex.glsl", resourceDirectory + "/height_frag.glsl");
		if(!heightShader->init()) {
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		heightShader->addUniform("P");
		heightShader->addUniform("V");
		heightShader->addUniform("M");
		// lighting
		heightShader->addUniform("lightColor");
		heightShader->addUniform("ambientIntensity");
		heightShader->addUniform("shineIntensity");
		heightShader->addUniform("flip");
		heightShader->addUniform("Texture0");
		heightShader->addUniform("Texture1");
		heightShader->addUniform("Texture2");
		heightShader->addUniform("lightPos");
		heightShader->addAttribute("vertPos");
		heightShader->addAttribute("vertNor");
		heightShader->addAttribute("vertTex");
		// camera position and offset
		heightShader->addUniform("camoff");
		heightShader->addUniform("campos");
		// mesh size
		heightShader->addUniform("mesh_size");
		// texture zoom
		heightShader->addUniform("tex_zoom");

		skysphere_shader = make_shared<Program>();
		skysphere_shader->setVerbose(true);
		skysphere_shader->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		skysphere_shader->init();
		skysphere_shader->addUniform("P");
		skysphere_shader->addUniform("V");
		skysphere_shader->addUniform("M");
		skysphere_shader->addUniform("tex");
		skysphere_shader->addUniform("tex2");
		skysphere_shader->addUniform("day_night_ratio");
		skysphere_shader->addAttribute("vertPos");
		skysphere_shader->addAttribute("vertNor");
		skysphere_shader->addAttribute("vertTex");

		// water shader
		water_shader = make_shared<Program>();
		water_shader->setVerbose(true);
		water_shader->setShaderNames(resourceDirectory + "/water_vertex.glsl", resourceDirectory + "/water_frag.glsl");
		if(!water_shader->init()) {
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		water_shader->addUniform("P");
		water_shader->addUniform("V");
		water_shader->addUniform("M");
		// lighting
		water_shader->addUniform("lightColor");
		water_shader->addUniform("ambientIntensity");
		water_shader->addUniform("shineIntensity");
		water_shader->addUniform("flip");
		water_shader->addUniform("Texture0");
		water_shader->addUniform("lightPos");
		water_shader->addAttribute("vertPos");
		water_shader->addAttribute("vertNor");
		water_shader->addAttribute("vertTex");
		// camera position and offset
		water_shader->addUniform("camoff");
		water_shader->addUniform("campos");
		// mesh size
		water_shader->addUniform("mesh_size");
		// texture zoom
		water_shader->addUniform("tex_zoom");

		/* --- Terrain Textures --- */
		//read in a load the texture
		water_texture = make_shared<Texture>();
  		water_texture->setFilename(resourceDirectory + "/water.jpeg");
  		water_texture->init();
  		water_texture->setUnit(0);
  		water_texture->setWrapModes(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh
		// Load geometry
		string errStr;
		const std::string& objectDir = resourceDirectory + "/objects";

		// trees
		tree1 = make_shared<MultiShape>(false, texProg, (objectDir + "/trees/_1_tree.jpg").c_str());
		bool rc = tree1->loadObjFromFile(errStr, (objectDir + "/trees/tree1.obj").c_str());
		// error handling
		if (!rc) {
			cerr << errStr << endl;
			exit(-1);
		}

		// rocks
		rock = make_shared<MultiShape>(false, texProg, (objectDir + "/rocks/rock1.jpeg").c_str());
		rc = rock->loadObjFromFile(errStr, (objectDir + "/rocks/rock1.obj").c_str());
		// error handling
		if (!rc) {
			cerr << errStr << endl;
			exit(-1);
		}

		// campfire
		campfire = make_shared<MultiShape>(false, texProg, (objectDir + "/campfire/campfire1.jpg").c_str());
		rc = campfire->loadObjFromFile(errStr, (objectDir + "/campfire/campfire1.obj").c_str());
		// error handling
		if (!rc) {
			cerr << errStr << endl;
			exit(-1);
		}

		// skysphere
		skysphere = make_shared<SkySphere>(skysphere_shader, (resourceDirectory + "/skysphere/sphere-day.jpg").c_str(), (resourceDirectory + "/skysphere/sphere-night.jpeg").c_str());
		rc = skysphere->loadObjFromFile(errStr, (resourceDirectory + "/skysphere/sphereWTex.obj").c_str());
		// error handling
		if (!rc) {
			cerr << errStr << endl;
			exit(-1);
		}

		// hierarchical model
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		// string errStr;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (objectDir + "/testing/SmoothSphere.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			mesh = make_shared<Shape>();
			mesh->createShape(TOshapes[0]);
			mesh->measure();
			mesh->init();
		}

		//code to load in the ground plane (CPU defined data passed to GPU)
		std::vector<std::string> tex_filenames = {
			(resourceDirectory + "/sand.jpg").c_str(),
			(resourceDirectory + "/grass.jpg").c_str(),
			(resourceDirectory + "/stone.jpg").c_str()
		};
		ground.init(heightShader, tex_filenames);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}

	void draw_HM(std::shared_ptr<MatrixStack> Model, shared_ptr<Program> prog, shared_ptr<Texture> texture) {
		#define HM_ROT_ANGLE	7.80
		#define HM_ROT_VECTOR	vec3(0,1,0)

		#define TORSO_SIZE	vec3(1.25, 1.35, 1.25)
		#define TORSO_POS 	vec3(0, 1, 0)

		#define HEAD_SIZE 	vec3(0.5, 0.5, 0.5)
		#define HEAD_POS 	vec3(0, 2.4, 0)

		#define BICEP_SIZE 	vec3(0.4, 0.25, 0.25)
		#define FORARM_SIZE vec3(0.6, 0.25, 0.25)
		#define HAND_SIZE 	vec3(0.3, 0.25, 0.25)

		#define RIGHT_SHOULDER_POS vec3(0.8, 1.8, 0)
		#define LEFT_SHOULDER_POS vec3(-0.8, 1.8, 0)

		// Draw a heirarchical model
		//animation update example
		float sTheta = sin(glfwGetTime());

		float left_bicep_angle, left_forearm_angle, left_hand_angle;
		left_bicep_angle = 0.5 + (sTheta * 0.1);
		left_forearm_angle = 1 + (sTheta * 0.1);
		left_hand_angle = 0 + (sTheta * 0.1);

		float right_bicep_angle, right_forearm_angle, right_hand_angle;
		right_bicep_angle = -left_bicep_angle;
		right_forearm_angle = -left_forearm_angle;
		right_hand_angle = -left_hand_angle;

		prog->bind();

		// draw mesh
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(gTrans, 2, 0));
			/* draw top sphere - aka head */
			Model->pushMatrix();
				Model->translate(HEAD_POS);
				Model->scale(HEAD_SIZE);
				setModel(prog, Model);
				mesh->draw(prog);
				// mesh->draw();
			Model->popMatrix();

			// TORSO
			Model->pushMatrix();
				Model->translate(TORSO_POS);
			  	Model->scale(TORSO_SIZE);
			  	setModel(prog, Model);
			  	mesh->draw(prog);
				// mesh->draw();
			Model->popMatrix();

			// LEFT ARM - BICEP
			Model->pushMatrix();
				Model->rotate(HM_ROT_ANGLE, HM_ROT_VECTOR);
				//place at shoulder
				Model->translate(LEFT_SHOULDER_POS);
				//rotate shoulder joint
				Model->rotate(left_bicep_angle, vec3(0, 1, 0));
				//move to shoulder joint
				Model->translate(vec3(-BICEP_SIZE.x, 0, 0));

			    // FOREARM
			  	Model->pushMatrix();
					// place at elbow
					Model->translate(vec3(-BICEP_SIZE.x, 0, 0));
					// rotate elbow joint
					Model->rotate(left_forearm_angle, vec3(0, 1, 1));
					// move to elbow - releative to rotation
					Model->translate(vec3(-FORARM_SIZE.x, 0, 0));

					// Hand
					Model->pushMatrix();
						// place at wrist
						Model->translate(vec3(-FORARM_SIZE.x, 0, 0));
						// rotate wrist joint
						Model->rotate(left_hand_angle, vec3(0, 0, 1));
						// move to wrist - releative to rotation
						Model->translate(vec3(-HAND_SIZE.x, 0, 0));


						// Do scale only to forearm
						Model->scale(HAND_SIZE);
						setModel(prog, Model);
						mesh->draw(prog);
						// mesh->draw();
					Model->popMatrix();

					// Do scale only to forearm
			      	Model->scale(FORARM_SIZE);
					setModel(prog, Model);
					mesh->draw(prog);
					// mesh->draw();
			  	Model->popMatrix();

			// BICEP
			Model->scale(BICEP_SIZE);
			setModel(prog, Model);
			mesh->draw(prog);
			// mesh->draw();
			Model->popMatrix();

			// RIGHT ARM - BICEP
			Model->pushMatrix();
				Model->rotate(HM_ROT_ANGLE, HM_ROT_VECTOR);
				//place at shoulder
				Model->translate(RIGHT_SHOULDER_POS);
				//rotate shoulder joint
				Model->rotate(right_bicep_angle, vec3(0, 1, 0));
				//move to shoulder joint
				Model->translate(vec3(BICEP_SIZE.x, 0, 0));

			    // FOREARM
			  	Model->pushMatrix();
					// place at elbow
					Model->translate(vec3(BICEP_SIZE.x, 0, 0));
					// rotate elbow joint
					Model->rotate(right_forearm_angle, vec3(0, 1, 1));
					// move to elbow - releative to rotation
					Model->translate(vec3(FORARM_SIZE.x, 0, 0));

					// Hand
					Model->pushMatrix();
						// place at wrist
						Model->translate(vec3(FORARM_SIZE.x, 0, 0));
						// rotate wrist joint
						Model->rotate(right_hand_angle, vec3(0, 0, 1));
						// move to wrist - releative to rotation
						Model->translate(vec3(HAND_SIZE.x, 0, 0));


						// Do scale only to forearm
						Model->scale(HAND_SIZE);
						setModel(prog, Model);
						mesh->draw(prog);
						// mesh->draw();
					Model->popMatrix();

					// Do scale only to forearm
			      	Model->scale(FORARM_SIZE);
					setModel(prog, Model);
					mesh->draw(prog);
					// mesh->draw();
			  	Model->popMatrix();

			// BICEP
			Model->scale(BICEP_SIZE);
			setModel(prog, Model);
			mesh->draw(prog);
			// mesh->draw();
		Model->popMatrix();

		prog->unbind();
	}

	void render() {
		double frametime = get_last_elapsed_time();

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
		//auto View = make_shared<MatrixStack>();
		mat4 View = mycam.process(frametime);
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// --- Initialize Textures ---
		// Initialize Prog
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View));

		glUniform1i(prog->getUniform("flip"), 0);
		glUniform3f(prog->getUniform("lightPos"), light_trans.x, light_trans.y, light_trans.z);
		prog->unbind();

		// Initialize texProg
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		glUniform3f(texProg->getUniform("lightPos"), light_trans.x, light_trans.y, light_trans.z);
		glUniform3f(texProg->getUniform("lightColor"), light_color.r, light_color.g, light_color.b);
		glUniform1f(texProg->getUniform("ambientIntensity"), ambient_intensity);
		glUniform1f(texProg->getUniform("shineIntensity"), shine_intensity);
		texProg->unbind();

		// Height Shader
		heightShader->bind();
		glUniformMatrix4fv(heightShader->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(heightShader->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glUniformMatrix4fv(heightShader->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		glUniform3f(heightShader->getUniform("lightPos"), light_trans.x, light_trans.y, light_trans.z);
		glUniform3f(heightShader->getUniform("lightColor"), light_color.r, light_color.g, light_color.b);
		glUniform1f(heightShader->getUniform("ambientIntensity"), ambient_intensity);
		glUniform1f(heightShader->getUniform("shineIntensity"), shine_intensity);
		heightShader->unbind();

		// Height Shader
		water_shader->bind();
		glUniformMatrix4fv(water_shader->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(water_shader->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		glUniformMatrix4fv(water_shader->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		glUniform3f(water_shader->getUniform("lightPos"), light_trans.x, light_trans.y, light_trans.z);
		glUniform3f(water_shader->getUniform("lightColor"), light_color.r, light_color.g, light_color.b);
		glUniform1f(water_shader->getUniform("ambientIntensity"), ambient_intensity);
		glUniform1f(water_shader->getUniform("shineIntensity"), shine_intensity);
		water_shader->unbind();

		// Sky Sphere Shader
		skysphere_shader->bind();
		// strip the camera translation from the view matrix for the skybox, so it stays put
		// https://learnopengl.com/Advanced-OpenGL/Cubemaps
		glm::mat4 ViewBox = glm::mat4(glm::mat3(View));
		// pass modified view and projection matrices
		glUniformMatrix4fv(skysphere_shader->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(skysphere_shader->getUniform("V"), 1, GL_FALSE, value_ptr(ViewBox));
		skysphere_shader->unbind();

		/** --- Draw The Skysphere --- **/
		// disable the z buffer test
		glDisable(GL_DEPTH_TEST);
		// draw big background sphere
		skysphere->reset_trans();
		skysphere->center_and_scale();
		skysphere->translate(vec3(0, 0, 0));
		skysphere->scale(vec3(1));
		skysphere->draw(sTheta);
		// reendable the z buffer test for drawing the rest of the scene
		glEnable(GL_DEPTH_TEST);
		/* --- */

		// --- Draw Scene ---
		// draw hierarchical model
		draw_HM(Model, texProg, texture0);

		// draw the campfire
		campfire->reset_trans();
		campfire->center_and_scale();
		campfire->scale(vec3(1));
		campfire->translate(this->light_trans);
		campfire->draw();

		// draw the rocks
		std::vector<glm::vec3> rock_vectors = {
			vec3(	0,		1.65, 	5),
			vec3(	-2.5,	1.5, 	-5),
			vec3(	10,		1.25, 	5),
			vec3(	-10,	1.5, 	-5),
			vec3(	0,		1, 		-10),
		};

		for(auto v : rock_vectors) {
			// if distance from camera is greater than 50, dont draw
			if(glm::distance(v, mycam.pos) > 50) {
				continue;
			}
			rock->reset_trans();
			rock->center_and_scale();
			rock->translate(v);
			rock->scale(vec3(0.5));
			rock->draw();
		}

		//draw the trees
		std::vector<glm::vec3> tree_vectors = {
			vec3(5,2,0),
			vec3(10,1.5,5),
			vec3(0,2,5),
			vec3(-10,1.5,5),
			vec3(0,1.5,-8),
			vec3(20, 2.3, 25),
			vec3(30, 3.5, 30)
		};

		for(auto v : tree_vectors) {
			// if distance from camera is greater than 50, dont draw
			if(glm::distance(v, mycam.pos) > 30) {
				continue;
			}
			tree1->reset_trans();
			tree1->center_and_scale();
			tree1->translate(v);
			tree1->scale(vec3(2));
			tree1->draw();
		}

		// draw the ground
		ground.draw(-mycam.pos);
		ground.drawPlane(water_shader, water_texture, -mycam.pos);

		//animation update example
		sTheta = (0.5f * sin(glfwGetTime() * 1 / 5)) + 0.5f;
		ambient_intensity = std::min(std::max(sTheta, 0.3f), 0.6f);
		shine_intensity = std::min(std::max(sTheta, 0.3f), 0.6f);
		light_color = vec3(1, 1, 1) * std::min(std::max(sTheta, 0.3f), 0.6f);

		// Pop matrix stacks.
		Projection->popMatrix();
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
	windowManager->init(1280, 960);
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
