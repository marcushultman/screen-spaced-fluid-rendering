#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class FluidPositions;

class FluidParticleSystem {
 public:
  FluidParticleSystem(unsigned int width, unsigned int height, float near_plane, float far_plane);
  ~FluidParticleSystem();

  void update(double elapsed_time);

  void preProcessPass(const glm::mat4 &view, const glm::mat4 &projection);
  void postProcessPass(GLuint background_texture, const glm::mat4 &view, const glm::mat4 &proj);

 private:
  void setupPositions();

  void setupVAO();

  void setupDataFBO();
  void setupBuffer(GLuint fbo, GLuint depth_texture, GLuint thickness_texture);

  void setupShaders();
  GLuint setupShader(GLuint type, const std::string &filename);
  void setupCubeMap();

  void dataPass(const glm::mat4 &view, const glm::mat4 &proj);
  void blurPass();

  void drawParticles(GLuint program, const glm::mat4 &view, const glm::mat4 &proj);
  void drawQuad();

  std::unique_ptr<FluidPositions> _positions;

  const unsigned int _width, _height;
  const float _near_plane, _far_plane;

  GLuint _vertex_array_object;
  GLuint _position_array_buffer;

  size_t _num_particles = 0;

  GLuint _data_fbo[2];
  GLuint _depth_texture[2];
  GLuint _thickness_texture[2];

  GLuint _particle_program, _data_program, _blur_program;
  GLuint _reflection_texture;
};
