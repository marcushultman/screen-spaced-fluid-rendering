
#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

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
#define FAR_PLANE 500.0f
#define FOV 0.75f

static const vec3 X_AXIS(1, 0, 0);
static const vec3 Y_AXIS(0, 1, 0);
static const vec3 Z_AXIS(0, 0, 1);

// TODO: Build camera class
//float camRotationSpeed = 0.005f;
//float camNormalSpeed = 250.0f, camHighSpeed = 500.0f, 
	//camSpeed = camNormalSpeed;

static unsigned int			s_cameraIndex = 0;
static OrbitCamera			s_camera;
static FirstPersonCamera	s_camera2;
static glm::mat4			s_proj;

Model*	s_dwarf;
SkyBox*	s_skybox;
Plane*	s_plane;
glm::vec2 floorSize = glm::vec2(150.0, 150.0);

glm::vec2 previousMousePos;


// Particle system
static FluidParticleSystem* s_particleSystem;
float particleSize = 5.0f;
float particleSep = 8.0f;

bool renderDepth = 0;


//// Post processing NOT CURRENTLY USED
//GLuint mainBuffer, mainBufferTexture
//GLuint frameBufferObject, 
//	frameBufferTexture, 
//	postProcessShaderProgram;

#pragma endregion

#pragma region Event handlers

static void onError(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
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

		if (action == GLFW_PRESS)
			renderDepth = true;
		else if (action == GLFW_RELEASE)
			renderDepth = false;
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

//static void setupDefaultFBO(int width, int height){
//	
//	// Set up renderbuffer
//	glGenFramebuffers(1, &frameBufferObject);
//	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
//
//	// Color buffer
//	glGenTextures(1, &frameBufferTexture);
//	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	
//	// Depth buffer
//	GLuint depthTexture;
//	glGenTextures(1, &depthTexture);
//	glBindTexture(GL_TEXTURE_2D, depthTexture);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//
//	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//		frameBufferTexture, 0);
//	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
//		depthTexture, 0);
//
//	// Always check that our framebuffer is ok
//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//		throw;
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//#pragma region Post process shader
//	{
//		GLuint vertexShader, fragmentShader;
//
//		vertexShader = glCreateShader(GL_VERTEX_SHADER);
//		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//
//		const char * vv = textFileRead("resource/shaders/postprocess.vert");
//		const char * ff = textFileRead("resource/shaders/postprocess.frag");
//
//		glShaderSource(vertexShader, 1, &vv, NULL);
//		glShaderSource(fragmentShader, 1, &ff, NULL);
//
//		int compileOK;
//
//		glCompileShader(vertexShader);
//		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK);
//		if (!compileOK) {
//			GLint infoLogLength;
//			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
//
//			GLchar* strInfoLog = new GLchar[infoLogLength + 1];
//			glGetShaderInfoLog(vertexShader, infoLogLength, NULL, strInfoLog);
//
//			fprintf(stderr, "Compilation error in vertexShader: %s\n", strInfoLog);
//			delete [] strInfoLog;
//
//			throw;
//		}
//
//		glCompileShader(fragmentShader);
//		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK);
//		if (!compileOK) {
//			GLint infoLogLength;
//			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
//
//			GLchar* strInfoLog = new GLchar[infoLogLength + 1];
//			glGetShaderInfoLog(fragmentShader, infoLogLength, NULL, strInfoLog);
//
//			fprintf(stderr, "Compilation error in fragmentShader: %s\n", strInfoLog);
//			delete [] strInfoLog;
//
//			throw;
//		}
//
//		postProcessShaderProgram = glCreateProgram();
//		glAttachShader(postProcessShaderProgram, vertexShader);
//		glAttachShader(postProcessShaderProgram, fragmentShader);
//
//		glBindAttribLocation(postProcessShaderProgram, 0, "position");
//		glBindFragDataLocation(postProcessShaderProgram, 0, "fragmentColor");
//
//		glLinkProgram(postProcessShaderProgram);
//		glValidateProgram(postProcessShaderProgram);
//	}
//#pragma endregion
//}


static void initialize(GLFWwindow* window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	// Set up projection matrix
	s_proj = glm::perspective(FOV, float(width) / float(height),
		NEAR_PLANE, FAR_PLANE);

	//setupDefaultFBO(width, height);
	//setupParticleDataFBO(width, height);


	s_skybox = new SkyBox();
	s_plane = new Plane(floorSize.x, floorSize.y);

	//Model::Load("resource/models/X/Testwuson.X");
	//Model::Load("resource/models/X/dwarf.x");
	s_dwarf = Model::Load("resource/models/X/dwarf.x");


	// Create fluid particle system
	s_particleSystem = new FluidParticleSystem(particleSize,
		width, height, NEAR_PLANE, FAR_PLANE);
	int num = 8;
	std::vector<glm::vec3> positions;
	for (int x = -num; x < num; x++){
		for (int y = 0; y < 3; y++){
			for (int z = -num; z < num; z++){
				positions.push_back(particleSep * glm::vec3(x, .75f + y, z));
			}
		}
	}		
	s_particleSystem->setPositions(positions);
	printf("Number of particles: %d\n", positions.size());
}

static void update(double elapsedTime, GLFWwindow* window){

	// Update camera position
	/*camPos += camSpeed * (float) elapsedTime * camVelocity *
		glm::angleAxis(camPitch, X_AXIS) *
		glm::angleAxis(camYaw, Y_AXIS);*/

	if (s_cameraIndex == 1){
		s_camera2.update((float) elapsedTime);
	}

	// TODO: Perform particle movement 

	//s_particleSystem->update();
}

static void drawQuad()
{
	static GLuint vertexArrayObject = 0;

	// TODO: Move to initialization
	// do this initialization first time the function is called... somewhat dodgy, but works for demonstration purposes
	if (vertexArrayObject == 0)
	{
		static const float positions [] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f,
			-1.0f, 1.0f,
		};

		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);

		GLuint buffer = 0;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);
	}

	glBindVertexArray(vertexArrayObject);
	glDrawArrays(GL_QUADS, 0, 4);
}
static void draw(double elapsed_time, GLFWwindow* window)
{
	// Clear the main buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.390625f, 0.582031f, 0.925781f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: Create camera class, obtain view from camera
	// Update view matrix
	/*vec3 center = camPos - Z_AXIS * 
		glm::angleAxis(camPitch, X_AXIS) * 
		glm::angleAxis(camYaw, Y_AXIS);
	view = glm::lookAt(camPos, center, Y_AXIS);*/

	glm::vec3 cameraPosition = s_camera.getPosition();
	glm::mat4 view;

	if (s_cameraIndex == 0){
		view = s_camera.getView();
	}
	else{
		view = s_camera2.getView();
	}

	// Draw backgound scene
	s_skybox->draw(view, s_proj);
	s_plane->draw(view, s_proj);
	s_dwarf->Draw(view, s_proj);

	// Draw fluid
	if (!glfwGetKey(window, GLFW_KEY_Z))
		s_particleSystem->draw(0, view, s_proj);


	glUseProgram(0);
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
	// the time of the previous frame
	double old_time = glfwGetTime();
	// this just loops as long as the program runs
	while (!glfwWindowShouldClose(window))
	{
		// calculate time elapsed, and the amount by which stuff rotates
		double current_time = glfwGetTime(),
			elapsed_time = (current_time - old_time);
		old_time = current_time;

		// Update
		update(elapsed_time, window);

		// Draw

		//glDisable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		draw(elapsed_time, window);

		/*
		if (renderDepth)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glClearColor(0.690625, 0.582031, 0.925781, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glUseProgram(postProcessShaderProgram);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, particleDataDepth);
			glProgramUniform1i(postProcessShaderProgram,
				glGetUniformLocation(postProcessShaderProgram, "frameBufferTexture"), 0);
			drawQuad();
			glUseProgram(0);
		}
		*/

		//Swap buffers
		glfwSwapBuffers(window);
		//Get and organize events, like keyboard and mouse input, window resizing, etc...
		glfwPollEvents();
		//glfwWaitEvents();
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
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Set the GLFW window creation hints - these are optional
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Declare a window object
	GLFWwindow* window;

	//Create a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
		"Screen Space Fluid Rendering",
		NULL /* glfwGetPrimaryMonitor() */,
		NULL);

	//If the window couldn't be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	// Output some information
	puts(glfwGetVersionString());
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	// --------------------------------------------
	//			Set callbacks
	// --------------------------------------------

	setupCallback(window);
	
	// --------------------------------------------
	//			AssImp init
	// --------------------------------------------

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
	if (err != GLEW_OK)
	{
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

	//Main Loop
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