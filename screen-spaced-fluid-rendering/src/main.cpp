
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <fstream>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>

#include <IL/il.h>

#include "textfile.h"

#include "Model.h"
#include "Box.h"
#include "SkyBox.h"
#include "Plane.h"
#include "FluidParticleSystem.h"

#include "OrbitCamera.h"
#include "FirstPersonCamera.h"

#pragma region Field

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 960

#define NEAR_PLANE 0.1f
#define FAR_PLANE 1250.0f
#define FOV 0.75f

static const glm::vec3 X_AXIS(1, 0, 0);
static const glm::vec3 Y_AXIS(0, 1, 0);
static const glm::vec3 Z_AXIS(0, 0, 1);


static unsigned int			s_cameraIndex = 0;
static OrbitCamera			s_camera = OrbitCamera(glm::vec3(0, 3, 0), glm::vec2(0, .5f));
static FirstPersonCamera	s_camera2;
static glm::mat4			s_proj;


Model*	s_dwarf;
SkyBox*	s_skybox;
Plane*	s_plane;
glm::vec2 floorSize = glm::vec2(150.0, 150.0);

glm::vec2 previousMousePos;


// Particle system
static FluidParticleSystem* s_particleSystem;
float particleSize = 3.0f;
float particleSep = 5.0f;

// Post processing
static GLuint mainBuffer;
static GLuint mainBufferTexture, mainBufferDepth;
static GLuint postProcessShader;

#pragma endregion

#pragma region Event handlers

static void onError(int error, const char* description)
{
	fputs(description, stderr);
	// _fgetchar();
}

static void onKeyDown(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Terminate repeats
	if (action == GLFW_REPEAT)
		return;

	if (mods & GLFW_MOD_SHIFT){
		s_camera2.setSpeed(500);
	}
	else{
		s_camera2.setSpeed(250);
	}


	// Set velocity
	switch (key)
	{
		// Camera movement
	case GLFW_KEY_W:
	case GLFW_KEY_S:
		s_camera2.setVelocityZ(key == GLFW_KEY_W ? -action : action);
		break;
	case GLFW_KEY_A:
	case GLFW_KEY_D:
		s_camera2.setVelocityX(key == GLFW_KEY_A ? -action : action);
		break;

		// Arrow
	/*case GLFW_KEY_UP:
		if (action == GLFW_PRESS)
			camPitch -= .6f;
		break;
	case GLFW_KEY_DOWN:
		if (action == GLFW_PRESS)
			camPitch += .6f;
		break;
	case GLFW_KEY_LEFT:
		if (action == GLFW_PRESS)
			camYaw -= .6f;
		break;
	case GLFW_KEY_RIGHT:
		if (action == GLFW_PRESS)
			camYaw += .6f;
		break;*/

		// RESET CAMERA
	/*case GLFW_KEY_R: 
		camPitch = camYaw = 0;
		camPos = camVelocity = vec3(0);
		break;*/

	case GLFW_KEY_P:
		// Toggle camera
		if (action == GLFW_PRESS)
			s_cameraIndex = 1 - s_cameraIndex;
		break;

	default:
		break;
	}
}

static void onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
	glm::vec2 dpos = glm::vec2(xpos - previousMousePos.x,
		previousMousePos.y - ypos);

	// Camera control
	if (s_cameraIndex == 0){
		if (s_camera.getMode() == OrbitCamera::Mode::ARC){
			s_camera.rotate(dpos);
		}
		else if (s_camera.getMode() == OrbitCamera::Mode::PAN){
			s_camera.pan(dpos);
		}
	}
	else{
		s_camera2.rotate(dpos);
	}

	previousMousePos = glm::vec2(xpos, ypos);
}
static void onMouseDown(GLFWwindow* window, int button, int action, int mods)
{
	// Rotation mode
	if (button == GLFW_MOUSE_BUTTON_1){
		if (action == GLFW_PRESS){
			s_camera.setMode(glfwGetKey(window, GLFW_KEY_LEFT_ALT) ==
				GLFW_PRESS ? OrbitCamera::Mode::ARC : OrbitCamera::Mode::PAN);
		}
		else if (action == GLFW_RELEASE){
			s_camera.setMode(OrbitCamera::Mode::NONE);
		}
	}
	// Enable mouse move
	if (button == GLFW_MOUSE_BUTTON_1)
	{
		if (action == GLFW_PRESS){
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			previousMousePos = glm::vec2(x, y);
			glfwSetCursorPosCallback(window, onMouseMove);
		}
		else{
			glfwSetCursorPosCallback(window, NULL);
		}
	}	
}

static void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (s_cameraIndex == 0){
		s_camera.zoom((float) yoffset);
	}
}

#pragma endregion

static void setupCallback(GLFWwindow* window)
{
	// Keyboard strokes
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, onKeyDown);

	// Mouse button (which in turn activates mouse movement)
	glfwSetMouseButtonCallback(window, onMouseDown);

	glfwSetScrollCallback(window, onMouseScroll);
}

static void setupMainFBO(int width, int height){

	// Set up renderbuffer
	glGenFramebuffers(1, &mainBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mainBuffer);

	// Color buffer
	glGenTextures(1, &mainBufferTexture);
	glBindTexture(GL_TEXTURE_2D, mainBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Depth buffer
	glGenTextures(1, &mainBufferDepth);
	glBindTexture(GL_TEXTURE_2D, mainBufferDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0, mainBufferTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT, mainBufferDepth, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupPostProcessShader()
{
	GLuint vertexShader, fragmentShader;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	auto vv = textFileRead("screen-spaced-fluid-rendering/resource/shaders/quad.vert");
	auto ff = textFileRead("screen-spaced-fluid-rendering/resource/shaders/postprocess.frag");

  auto p = vv.c_str();
	glShaderSource(vertexShader, 1, &p, NULL);
  p = ff.c_str();
	glShaderSource(fragmentShader, 1, &p, NULL);

	int compileOK;

	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK) {
      GLint len;
	    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &len);
	    GLchar* s = new GLchar[len + 1];
	    glGetShaderInfoLog(vertexShader, len, NULL, s);
      printf("%s\n", s);
		throw;
  }

	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK)
		throw;

	postProcessShader = glCreateProgram();
	glAttachShader(postProcessShader, vertexShader);
	glAttachShader(postProcessShader, fragmentShader);
	glLinkProgram(postProcessShader);
	glValidateProgram(postProcessShader);
}


static void initialize(GLFWwindow* window)
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// Set up projection matrix
	s_proj = glm::perspective(FOV, float(width) / float(height),
		NEAR_PLANE, FAR_PLANE);

	// Post process rendering
	setupMainFBO(width, height);
	setupPostProcessShader();

	s_skybox = new SkyBox();
	s_plane = new Plane(floorSize.x, floorSize.y);

	//Model::Load("screen-spaced-fluid-rendering/resource/models/X/Testwuson.X");
	//Model::Load("screen-spaced-fluid-rendering/resource/models/X/dwarf.x");
	s_dwarf = Model::Load("screen-spaced-fluid-rendering/resource/models/X/dwarf.x");


	// Create fluid particle system
	s_particleSystem = new FluidParticleSystem(particleSize,
		width, height, NEAR_PLANE, FAR_PLANE);
	int num = 10;
	std::vector<glm::vec3> positions;
	for (int x = -num; x < num; x++){
		for (int y = 0; y < 5; y++){
			for (int z = -num; z < num; z++){
				positions.push_back(particleSep * glm::vec3(x, .75f + y, z));
			}
		}
	}		
	s_particleSystem->setPositions(positions);
	printf("Number of particles: %lu\n", positions.size());
}

static void update(double elapsedTime, GLFWwindow* window){

	// Update camera position
	if (s_cameraIndex == 0){
		if (s_camera.getMode() == OrbitCamera::Mode::NONE){
			s_camera.rotate(100 * elapsedTime, 0);
		}
	}
	else{
		s_camera2.update((float) elapsedTime);
	}

	// TODO: Perform particle movement
	//s_particleSystem->update();
}

static void drawQuad()
{
  static auto init = false;
	static GLuint s_quadVAO = 0;
	if (!init) {
    init = true;
		static const float positions [] = {
			1.0f, 1.0f,
			-1.0f, 1.0f,
			1.0f, -1.0f,
			-1.0f, -1.0f,
		};
    const int indices [] = {
      0, 1, 2,
      2, 1, 3,
    };

		glGenVertexArrays(1, &s_quadVAO);
		glBindVertexArray(s_quadVAO);

		GLuint buffer;

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindVertexArray(s_quadVAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
static void draw(double elapsed_time, GLFWwindow* window)
{
	// Clear the main buffer
	glBindFramebuffer(GL_FRAMEBUFFER, mainBuffer);
	glClearColor(0.390625f, 0.582031f, 0.925781f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get view from camera
	glm::vec3 cameraPosition = s_camera.getPosition();
	glm::mat4 view = (s_cameraIndex == 0 ? 
		s_camera.getView() : s_camera2.getView());

	// Draw backgound scene
	s_skybox->draw(view, s_proj);
	s_plane->draw(view, s_proj);
	s_dwarf->Draw(view, s_proj);

	// Pre-process fluid
	s_particleSystem->preProcessPass(view, s_proj);

	// Render background
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(postProcessShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainBufferTexture);
	glUniform1i(glGetUniformLocation(postProcessShader, "source"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mainBufferDepth);
	glUniform1i(glGetUniformLocation(postProcessShader, "depth"), 1);
	glDepthFunc(GL_LEQUAL);
	drawQuad();

	// Post-process fluid
	s_particleSystem->postProcessPass(
		mainBufferTexture, view, s_proj);
}

static void unload(GLFWwindow* window)
{
	delete s_skybox;
	delete s_plane;
	delete s_dwarf;

	delete s_particleSystem;
}

#pragma region Main Loop

void mainLoop(GLFWwindow* window)
{
	double old_time = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		double current_time = glfwGetTime(),
			elapsed_time = (current_time - old_time);
		old_time = current_time;

		update(elapsed_time, window);
		draw(elapsed_time, window);

		glfwSwapBuffers(window);
		glfwPollEvents(); //glfwWaitEvents();
	}
}

#pragma endregion


#pragma region Entry Point

// Very complex stuff, impossible to explain.
int main(int argc, char *argv [])
{
	//Set the error callback
	glfwSetErrorCallback(onError);

	//Initialize GLFW
	if (!glfwInit()){
		exit(EXIT_FAILURE);
	}

	//Set the GLFW window creation hints - these are optional
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Declare a window object
	GLFWwindow* window;

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
		"Screen Space Fluid Rendering",
		NULL /* glfwGetPrimaryMonitor() */,
		NULL);

	if (!window){
		fprintf(stderr, "\nFailed to open GLFW window.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

  int frameBufferWidth, frameBufferHeight;
  glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
  glViewport(0, 0, frameBufferWidth, frameBufferHeight);

	// Output some information
	puts(glfwGetVersionString());
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	
	setupCallback(window);
	
	// AssImp init
	// get a handle to the predefined STDOUT log stream and attach
	// it to the logging system. It remains active for all further
	// calls to aiImportFile(Ex) and aiApplyPostProcessing.
	aiLogStream aiStream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
	aiAttachLogStream(&aiStream);

	// --------------------------------------------
	//			Initialize GLEW
	// --------------------------------------------

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK){
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	// --------------------------------------------
	//			DevIL init
	// --------------------------------------------

	ilInit();

	/* 
	* Now that we should have a valid GL context, perform our OpenGL
	* initialization, before we enter our main loop.
	*/
	initialize(window);

	mainLoop(window);

	unload(window);

	// We added a log stream to the library, it's our job to disable it
	// again. This will definitely release the last resources allocated
	// by Assimp.
	aiDetachAllLogStreams();

	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	//Finalize and clean up GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

#pragma endregion