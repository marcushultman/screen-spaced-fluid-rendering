
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <IL/il.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/vector3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <stdlib.h>
#include "Box.h"
#include "FirstPersonCamera.h"
#include "FluidParticleSystem.h"
#include "Model.h"
#include "OrbitCamera.h"
#include "Plane.h"
#include "SkyBox.h"
#include "config.h"
#include "textfile.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {

const auto SCREEN_WIDTH = 1280;
const auto SCREEN_HEIGHT = 960;

const auto NEAR_PLANE = 0.1f;
const auto FAR_PLANE = 1250.0f;
const auto FOV = 0.75f;

const auto X_AXIS = glm::vec3(1, 0, 0);
const auto Y_AXIS = glm::vec3(0, 1, 0);
const auto Z_AXIS = glm::vec3(0, 0, 1);

const auto kFloorSize = glm::vec2(150.0, 150.0);

const auto kParticleSize = 3.0f;
const auto kParticleSep = 5.0f;

class App {
 public:
  explicit App(GLFWwindow &window);

  void mainLoop(GLFWwindow *window);

 private:
  void onError(int error, const char *description);
  void onKeyDown(int key, int scancode, int action, int mods);
  void onMouseMove(double xpos, double ypos);
  void onMouseDown(int button, int action, int mods);
  void onMouseScroll(double xoffset, double yoffset);

  void initialize(GLFWwindow *window);
  void setupMainFBO(int width, int height);
  void setupPostProcessShader();
  void update(double elapsedTime, GLFWwindow *window);
  void drawQuad();
  void draw(double elapsed_time, GLFWwindow *window);

  GLFWwindow &_window;

  OrbitCamera _camera{glm::vec3(0, 3, 0), glm::vec2(0, .5f)};
  FirstPersonCamera _fps_camera;
  unsigned int _camera_index = 0;
  std::vector<Camera *> _cameras = {&_camera, &_fps_camera};
  glm::mat4 _projection;

  std::unique_ptr<Model> _dwarf;
  std::unique_ptr<SkyBox> _skybox;
  std::unique_ptr<Plane> _plane;

  glm::vec2 _prev_mouse_pos;

  // Particle system
  std::unique_ptr<FluidParticleSystem> _particle_system;

  // Post processing
  GLuint _frame_buffer_object;
  GLuint _main_buffer_texture;
  GLuint _main_buffer_depth;
  GLuint _post_process_program;
};

}  // namespace

#pragma region Event handlers

void onError(int error, const char *description) {
  fputs(description, stderr);
  // _fgetchar();
}

void App::onKeyDown(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(&_window, GL_TRUE);
  }

  // Terminate repeats
  if (action == GLFW_REPEAT) {
    return;
  }

  if (mods & GLFW_MOD_SHIFT) {
    _fps_camera.setSpeed(500);
  } else {
    _fps_camera.setSpeed(250);
  }

  // Set velocity
  switch (key) {
      // Camera movement
    case GLFW_KEY_W:
    case GLFW_KEY_S:
      _fps_camera.setVelocityZ(key == GLFW_KEY_W ? -action : action);
      break;
    case GLFW_KEY_A:
    case GLFW_KEY_D:
      _fps_camera.setVelocityX(key == GLFW_KEY_A ? -action : action);
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
      if (action == GLFW_PRESS) {
        ++_camera_index;
      }
      break;

    default:
      break;
  }
}

void App::onMouseMove(double xpos, double ypos) {
  glm::vec2 dpos = glm::vec2(xpos - _prev_mouse_pos.x, _prev_mouse_pos.y - ypos);

  // Camera control
  if (_camera_index % 2 == 0) {
    if (_camera.getMode() == OrbitCamera::Mode::ARC) {
      _camera.rotate(dpos);
    } else if (_camera.getMode() == OrbitCamera::Mode::PAN) {
      _camera.pan(dpos);
    }
  } else {
    _fps_camera.rotate(dpos);
  }

  _prev_mouse_pos = glm::vec2(xpos, ypos);
}

void App::onMouseDown(int button, int action, int mods) {
  // Rotation mode
  if (button == GLFW_MOUSE_BUTTON_1) {
    if (action == GLFW_PRESS) {
      _camera.setMode(glfwGetKey(&_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS
                          ? OrbitCamera::Mode::ARC
                          : OrbitCamera::Mode::PAN);
    } else if (action == GLFW_RELEASE) {
      _camera.setMode(OrbitCamera::Mode::NONE);
    }
  }
  // Enable mouse move
  if (button == GLFW_MOUSE_BUTTON_1) {
    if (action == GLFW_PRESS) {
      double x, y;
      glfwGetCursorPos(&_window, &x, &y);
      _prev_mouse_pos = glm::vec2(x, y);
      glfwSetCursorPosCallback(&_window, [](auto window, double xpos, double ypos) {
        static_cast<App *>(glfwGetWindowUserPointer(window))->onMouseMove(xpos, ypos);
      });
    } else {
      glfwSetCursorPosCallback(&_window, NULL);
    }
  }
}

void App::onMouseScroll(double xoffset, double yoffset) {
  if (_camera_index % 2 == 0) {
    _camera.zoom((float)yoffset);
  }
}

#pragma endregion

App::App(GLFWwindow &window) : _window{window} {
  glfwSetWindowUserPointer(&_window, this);

  // Keyboard strokes
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetKeyCallback(&_window, [](auto window, auto key, auto scancode, auto action, auto mods) {
    static_cast<App *>(glfwGetWindowUserPointer(window))->onKeyDown(key, scancode, action, mods);
  });

  // Mouse button (which in turn activates mouse movement)
  glfwSetMouseButtonCallback(&_window, [](auto window, auto button, auto action, auto mods) {
    static_cast<App *>(glfwGetWindowUserPointer(window))->onMouseDown(button, action, mods);
  });

  glfwSetScrollCallback(&_window, [](auto window, auto xoffset, auto yoffset) {
    static_cast<App *>(glfwGetWindowUserPointer(window))->onMouseScroll(xoffset, yoffset);
  });

  initialize(&_window);
}

void App::initialize(GLFWwindow *window) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  // Set up projection matrix
  const auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
  _projection = glm::perspective(FOV, aspect_ratio, NEAR_PLANE, FAR_PLANE);

  // Post process rendering
  setupMainFBO(width, height);
  setupPostProcessShader();

  _skybox = std::make_unique<SkyBox>();
  _plane = std::make_unique<Plane>(kFloorSize.x, kFloorSize.y);
  _dwarf = std::make_unique<Model>(config::kResourcesDir + "/models/X/dwarf.x");

  // Create fluid particle system
  _particle_system =
      std::make_unique<FluidParticleSystem>(kParticleSize, width, height, NEAR_PLANE, FAR_PLANE);
  int num = 10;
  std::vector<glm::vec3> positions;
  for (int x = -num; x < num; x++) {
    for (int y = 0; y < 5; y++) {
      for (int z = -num; z < num; z++) {
        positions.push_back(kParticleSep * glm::vec3(x, .75f + y, z));
      }
    }
  }
  _particle_system->setPositions(positions);
  printf("Number of particles: %lu\n", positions.size());
}

void App::setupMainFBO(int width, int height) {
  // Set up renderbuffer
  glGenFramebuffers(1, &_frame_buffer_object);
  glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);

  // Color buffer
  glGenTextures(1, &_main_buffer_texture);
  glBindTexture(GL_TEXTURE_2D, _main_buffer_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Depth buffer
  glGenTextures(1, &_main_buffer_depth);
  glBindTexture(GL_TEXTURE_2D, _main_buffer_depth);
  glTexImage2D(
      GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _main_buffer_texture, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _main_buffer_depth, 0);

  // Always check that our framebuffer is ok
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void App::setupPostProcessShader() {
  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  auto vv = textFileRead(config::kResourcesDir + "/shaders/quad.vert");
  auto ff = textFileRead(config::kResourcesDir + "/shaders/postprocess.frag");

  auto p = vv.c_str();
  glShaderSource(vertex_shader, 1, &p, NULL);
  p = ff.c_str();
  glShaderSource(fragment_shader, 1, &p, NULL);

  int ok;

  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint len;
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &len);
    GLchar *s = new GLchar[len + 1];
    glGetShaderInfoLog(vertex_shader, len, NULL, s);
    printf("%s\n", s);
    throw;
  }

  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ok);
  assert(ok);

  _post_process_program = glCreateProgram();
  glAttachShader(_post_process_program, vertex_shader);
  glAttachShader(_post_process_program, fragment_shader);
  glLinkProgram(_post_process_program);
  glValidateProgram(_post_process_program);
}

void App::update(double elapsed_time, GLFWwindow *window) {
  // Update camera position
  _cameras[_camera_index % _cameras.size()]->update(elapsed_time);

  // TODO: Perform particle movement
  // _particle_system->update();
}

void App::drawQuad() {
  static auto init = false;
  static GLuint s_quadVAO = 0;
  if (!init) {
    init = true;
    static const float positions[] = {
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
    };
    const int indices[] = {
        0,
        1,
        2,
        2,
        1,
        3,
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

void App::draw(double elapsed_time, GLFWwindow *window) {
  // Clear the main buffer
  glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_object);
  glClearColor(0.390625f, 0.582031f, 0.925781f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Get view from camera
  auto view = _cameras[_camera_index % _cameras.size()]->getView();

  // Draw backgound scene
  _skybox->draw(view, _projection);
  _plane->draw(view, _projection);
  _dwarf->draw(view, _projection);

  // Pre-process fluid
  _particle_system->preProcessPass(view, _projection);

  // Render background
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(_post_process_program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _main_buffer_texture);
  glUniform1i(glGetUniformLocation(_post_process_program, "source"), 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _main_buffer_depth);
  glUniform1i(glGetUniformLocation(_post_process_program, "depth"), 1);
  glDepthFunc(GL_LEQUAL);
  drawQuad();

  // Post-process fluid
  _particle_system->postProcessPass(_main_buffer_texture, view, _projection);
}

void App::mainLoop(GLFWwindow *window) {
  auto old_time = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    auto current_time = glfwGetTime();
    auto elapsed_time = (current_time - old_time);
    old_time = current_time;

    update(elapsed_time, window);
    draw(elapsed_time, window);

    glfwSwapBuffers(window);
    glfwPollEvents();  // glfwWaitEvents();
  }
}

#pragma region Entry Point

// Very complex stuff, impossible to explain.
int main(int argc, char *argv[]) {
  // Set the error callback
  glfwSetErrorCallback(onError);

  // Initialize GLFW
  if (!glfwInit()) {
    return 1;
  }

  // Set the GLFW window creation hints - these are optional
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto *window = glfwCreateWindow(SCREEN_WIDTH,
                                  SCREEN_HEIGHT,
                                  "Screen Space Fluid Rendering",
                                  NULL /* glfwGetPrimaryMonitor() */,
                                  NULL);

  if (!window) {
    fprintf(stderr, "Failed to open GLFW window.\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // Output some information
  printf("%s\nOpenGL version supported %s\n", glfwGetVersionString(), glGetString(GL_VERSION));

#if defined(DEBUG)
  // AssImp init
  // get a handle to the predefined STDOUT log stream and attach
  // it to the logging system. It remains active for all further
  // calls to aiImportFile(Ex) and aiApplyPostProcessing.
  aiLogStream aiStream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
  aiAttachLogStream(&aiStream);
#endif

  glewExperimental = GL_TRUE;
  auto err = glewInit();

  // If GLEW hasn't initialized
  if (err != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    glfwTerminate();
    return 1;
  }

  ilInit();

  auto app = std::make_unique<App>(*window);
  app->mainLoop(window);
  app.reset();

#if defined(DEBUG)
  aiDetachAllLogStreams();
#endif

  glfwDestroyWindow(window);
  glfwTerminate();
}

#pragma endregion
