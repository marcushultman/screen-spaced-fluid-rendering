#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class FluidParticleSystem {
 public:
  FluidParticleSystem(float particle_size,
                      unsigned int width,
                      unsigned int height,
                      float near_plane,
                      float far_plane);
  ~FluidParticleSystem();

  void setPositions(std::vector<glm::vec3> positions);

  void update();

  void preProcessPass(const glm::mat4 &view, const glm::mat4 &projection);
  void postProcessPass(GLuint background_texture, const glm::mat4 &view, const glm::mat4 &proj);

 private:
  void setupVAO(float size);

  void setupDataFBO();
  void setupBlurFBO();
  void setupBlurBuffer(GLuint fbo, GLuint texture);

  void setupShaders(float particle_size);
  GLuint setupShader(GLuint type, const std::string &filename);
  void setupCubeMap();

  void dataPass(const glm::mat4 &view, const glm::mat4 &proj);
  void blurPass();
  void renderPass(const glm::mat4 &view, const glm::mat4 &proj);

  void drawParticles(GLuint program, const glm::mat4 &view, const glm::mat4 &proj);
  void drawQuad();

  GLuint _vertex_array_object;
  GLuint _position_array_buffer;

  size_t _num_particles = 0;

  GLuint _data_fbo;
  GLuint _blur_fbo[2];

  const unsigned int _width, _height;
  const float _near_plane, _far_plane;

  GLuint _data_texture, _thickness_texture;
  GLuint _blur_texture[2];

  GLuint _particle_program, _data_program, _blur_program;
  GLuint _reflection_texture;
};
