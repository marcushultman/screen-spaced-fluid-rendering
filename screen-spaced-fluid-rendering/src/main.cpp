
// #include "Camera.h"

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

#include <string>
#include <fstream>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <assimp\cimport.h>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <assimp\vector3.h>

#include <IL\il.h>

#include "textfile.h"

#include "Model.h"
#include "Box.h"
#include "SkyBox.h"
#include "Plane.h"
#include "FluidParticle.h"

#pragma region Field

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 960

#define NEAR_PLANE 0.1f
#define FAR_PLANE 500.0f
#define FOV 0.75f

// TODO: Build camera class
const vec3 X_AXIS(1, 0, 0);
const vec3 Y_AXIS(0, 1, 0);
const vec3 Z_AXIS(0, 0, 1);
vec3 camPos;
float camRotationSpeed = 0.005f;
float camPitch, camYaw;
float camNormalSpeed = 100.0f, camHighSpeed = 250.0f, 
	camSpeed = camNormalSpeed;
vec3 camVelocity;
mat4 view, projection;
float nearPlane = NEAR_PLANE; // 0.1f,
float farPlane = FAR_PLANE;

//static Camera		s_camera;
static glm::mat4	s_proj;

Model* dwarf;
SkyBox* skybox;
glm::vec2 floorSize = glm::vec2(150.0, 150.0);
Plane* plane;

glm::vec2 previousMousePos;


// Particle system
static FluidParticle* s_particleSystem;
float particleSize = 6.0f;

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

	if (mods & GLFW_MOD_SHIFT) camSpeed = camHighSpeed;
	else camSpeed = camNormalSpeed;

	// Set velocity
	switch (key)
	{
		// Camera movement
	case GLFW_KEY_W:
		camVelocity.z += (action == GLFW_PRESS ? -1.0f : 1.0f);
		break;
	case GLFW_KEY_A:
		camVelocity.x += (action == GLFW_PRESS ? -1.0f : 1.0f);
		break;
	case GLFW_KEY_S:
		camVelocity.z += (action == GLFW_PRESS ? 1.0f : -1.0f);
		break;
	case GLFW_KEY_D:
		camVelocity.x += (action == GLFW_PRESS ? 1.0f : -1.0f);
		break;

		// Arrow
	case GLFW_KEY_UP:
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
		break;

		// RESET CAMERA
	case GLFW_KEY_R: 
		camPitch = camYaw = 0;
		camPos = camVelocity = vec3(0);
		break;

		// RESET CAMERA
	case GLFW_KEY_P:
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
	camYaw += camRotationSpeed * ((float) xpos - previousMousePos.x);
	camPitch += camRotationSpeed * ((float) ypos - previousMousePos.y);

	previousMousePos = glm::vec2(xpos, ypos);
}
static void onMouseDown(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_1)
	{
		if (action == GLFW_PRESS){
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			previousMousePos = glm::vec2(x, y);
			glfwSetCursorPosCallback(window, onMouseMove);
		}
		else
			glfwSetCursorPosCallback(window, NULL);
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
	//camPos = vec3(0, 50, 200);
	camPos = vec3(80, 23, 82);

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	// Set up projection matrix
	projection = glm::perspective(FOV, float(width) / float(height),
		NEAR_PLANE, FAR_PLANE);

	//setupDefaultFBO(width, height);
	//setupParticleDataFBO(width, height);

	//Skybox
	skybox = new SkyBox();
	skybox->Load();

	plane = new Plane(floorSize.x, floorSize.y);

	//Model::Load("resource/models/X/Testwuson.X");
	//Model::Load("resource/models/X/dwarf.x");
	dwarf = Model::Load("resource/models/X/dwarf.x");

	s_particleSystem = new FluidParticle(particleSize);
	int num = 8;
	float distance = 10.0f;
	std::vector<glm::vec3> positions;
	for (int x = -num; x < num; x++){
		for (int y = 0; y < 3; y++){
			for (int z = -num; z < num; z++){
				positions.push_back(distance * glm::vec3(x, y, z));
			}
		}
	}		
	s_particleSystem->SetPositions(positions);
	printf("Number of particles: %d\n", positions.size());

	// Set up render-to-texture
	s_particleSystem->setupFBO(width, height);

}

static void update(double elapsedTime, GLFWwindow* window){

	// Update camera position
	camPos += camSpeed * (float) elapsedTime * camVelocity *
		glm::angleAxis(camPitch, X_AXIS) *
		glm::angleAxis(camYaw, Y_AXIS);

	// TODO: Perform particle movement 

}

static void drawFullScreenQuad()
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
	vec3 center = camPos - Z_AXIS * 
		glm::angleAxis(camPitch, X_AXIS) * 
		glm::angleAxis(camYaw, Y_AXIS);
	view = glm::lookAt(camPos, center, Y_AXIS);

	// Draw backgound scene
	skybox->Draw(view, projection);
	plane->Draw(view, projection);
	dwarf->Draw(view, projection);

	/////////////////////////////
	// DATA PASS
	/////////////////////////////

	s_particleSystem->DrawData(view, projection);


	/////////////////////////////
	// BLUR PASS
	/////////////////////////////

	// -- CODE GOES HERE --



	/////////////////////////////
	// RENDER PASS
	/////////////////////////////

	s_particleSystem->Draw(view, projection);

	glUseProgram(0);
}

static void unload(GLFWwindow* window)
{

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
			drawFullScreenQuad();
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

	//delete dwarf;
	//delete plane;
	//delete skybox;

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