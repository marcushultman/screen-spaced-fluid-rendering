#include "FluidParticleSystem.h"

#include <cassert>
#include <optional>
#include <string>
#include <IL/il.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "config.h"
#include "textfile.h"

namespace {

constexpr auto kBlurIterations = 8;
constexpr auto kBlurRadius = 2.0f;

}  // namespace

FluidParticleSystem::FluidParticleSystem(
    float particle_size, unsigned int width, unsigned int height, float near_plane, float far_plane)
    : _width{width}, _height{height}, _near_plane{near_plane}, _far_plane{far_plane} {
  setupVAO(particle_size);

  setupDataFBO();
  setupBlurFBO();

  setupShaders(particle_size);
}

FluidParticleSystem::~FluidParticleSystem() {}

void FluidParticleSystem::setupVAO(float particleSize) {
  const float positions[] = {particleSize,
                             particleSize,
                             0,
                             -particleSize,
                             particleSize,
                             0,
                             particleSize,
                             -particleSize,
                             0,
                             -particleSize,
                             -particleSize,
                             0};
  const float texCoords[] = {1, 1, 0, 1, 1, 0, 0, 0};
  const int indices[] = {0, 1, 2, 2, 1, 3};

  GLuint buffer;

  // Create the vertex array object
  glGenVertexArrays(1, &_vertex_array_object);
  glBindVertexArray(_vertex_array_object);

  // Create the buffer objects
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // Create buffer for instanced particle positions
  glGenBuffers(1, &_position_array_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, _position_array_buffer);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(2, 1);

  // Index buffer
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void FluidParticleSystem::setupDataFBO() {
  glGenFramebuffers(1, &_data_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _data_fbo);

  // Depth data
  glGenTextures(1, &_data_texture);
  glBindTexture(GL_TEXTURE_2D, _data_texture);
  glTexImage2D(
      GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Thickess data
  glGenTextures(1, &_thickness_texture);
  glBindTexture(GL_TEXTURE_2D, _thickness_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _data_texture, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _thickness_texture, 0);
  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  assert(status == GL_FRAMEBUFFER_COMPLETE);
}

void FluidParticleSystem::setupBlurFBO() {
  glGenFramebuffers(2, _blur_fbo);
  glGenTextures(2, _blur_texture);

  setupBlurBuffer(_blur_fbo[0], _blur_texture[0]);
  setupBlurBuffer(_blur_fbo[1], _blur_texture[1]);
}

void FluidParticleSystem::setupBlurBuffer(GLuint fbo, GLuint texture) {
  // generate depth buffer texture
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
      GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // bind as frame_buffer_object depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  assert(status == GL_FRAMEBUFFER_COMPLETE);
}

void FluidParticleSystem::setupShaders(float particle_size) {
  auto particle_vertex_shader = setupShader(GL_VERTEX_SHADER, "particle.vert");
  auto quad_vertex_shader = setupShader(GL_VERTEX_SHADER, "quad.vert");

  auto data_fragment_shader = setupShader(GL_FRAGMENT_SHADER, "particle_data.frag");
  auto blur_fragment_shader = setupShader(GL_FRAGMENT_SHADER, "blur.frag");
  auto particle_fragment_shader = setupShader(GL_FRAGMENT_SHADER, "particle.frag");

  _data_program = glCreateProgram();
  _blur_program = glCreateProgram();
  _particle_program = glCreateProgram();

  glAttachShader(_data_program, particle_vertex_shader);
  glAttachShader(_data_program, data_fragment_shader);
  glLinkProgram(_data_program);
  glValidateProgram(_data_program);

  glAttachShader(_blur_program, quad_vertex_shader);
  glAttachShader(_blur_program, blur_fragment_shader);
  glLinkProgram(_blur_program);
  glValidateProgram(_blur_program);

  glAttachShader(_particle_program, particle_vertex_shader);
  glAttachShader(_particle_program, particle_fragment_shader);
  glLinkProgram(_particle_program);
  glValidateProgram(_particle_program);

  glUseProgram(_data_program);
  glUniform1f(glGetUniformLocation(_data_program, "sphereRadius"), particle_size);
  glUniform1f(glGetUniformLocation(_data_program, "znear"), _near_plane);
  glUniform1f(glGetUniformLocation(_data_program, "zfar"), _far_plane);

  glUseProgram(_particle_program);
  glUniform1f(glGetUniformLocation(_particle_program, "sphereRadius"), particle_size);
  glUniform1f(glGetUniformLocation(_particle_program, "znear"), _near_plane);
  glUniform1f(glGetUniformLocation(_particle_program, "zfar"), _far_plane);

  glUseProgram(0);

  setupCubeMap();
}

GLuint FluidParticleSystem::setupShader(GLuint type, const std::string &filename) {
  auto shader = glCreateShader(type);

  auto src_dir = config::kResourcesDir + "/shaders";
  auto src = textFileRead(src_dir + "/" + filename);
  auto src_c_str = src.c_str();

  auto ok = 0;
  glShaderSource(shader, 1, &src_c_str, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  assert(ok);

  return shader;
}

void FluidParticleSystem::setupCubeMap() {
  glGenTextures(1, &_reflection_texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, _reflection_texture);

  std::vector<std::pair<GLenum, std::string>> faces = {
      {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "miramar_bk.tga"},
      {GL_TEXTURE_CUBE_MAP_POSITIVE_X, "miramar_ft.tga"},

      {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "miramar_dn.tga"},
      {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "miramar_up.tga"},

      {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "miramar_lf.tga"},
      {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "miramar_rt.tga"},
  };

  ILuint imageIds[faces.size()];
  ilGenImages(faces.size(), imageIds);

  for (auto i = 0; i < faces.size(); ++i) {
    auto &[face, filename] = faces[i];
    ilBindImage(imageIds[i]);

    ilLoadImage((config::kResourcesDir + "/envmap_miramar/" + filename).c_str());
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    glTexImage2D(face,
                 0,
                 GL_RGBA,
                 ilGetInteger(IL_IMAGE_WIDTH),
                 ilGetInteger(IL_IMAGE_HEIGHT),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 ilGetData());
  }
  ilDeleteImages(faces.size(), imageIds);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void FluidParticleSystem::setPositions(std::vector<glm::vec3> positions) {
  _num_particles = positions.size();

  glBindVertexArray(_vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, _position_array_buffer);
  glBufferData(
      GL_ARRAY_BUFFER, sizeof(glm::vec3) * _num_particles, positions.data(), GL_DYNAMIC_DRAW);
}

void FluidParticleSystem::update() {
  // TODO: Keep in sync with fluid motion
}

void FluidParticleSystem::preProcessPass(const glm::mat4 &view, const glm::mat4 &proj) {
  dataPass(view, proj);
  blurPass();
}

void FluidParticleSystem::postProcessPass(GLuint backgroundTexture,
                                          const glm::mat4 &view,
                                          const glm::mat4 &proj) {
  glUseProgram(_particle_program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, backgroundTexture);
  glUniform1i(glGetUniformLocation(_particle_program, "backgroundTexture"), 0);

  renderPass(view, proj);

#if defined(DEBUG)
  glBindFramebuffer(GL_READ_FRAMEBUFFER, _data_fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(
      0, 0, _width, _height, 0, 0, _width / 4, _height / 4, GL_COLOR_BUFFER_BIT, GL_LINEAR);
#endif
}

void FluidParticleSystem::dataPass(const glm::mat4 &view, const glm::mat4 &proj) {
  glBindFramebuffer(GL_FRAMEBUFFER, _data_fbo);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Thickness (blend, addative, no depth test)
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glDisable(GL_DEPTH_TEST);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  drawParticles(_data_program, view, proj);
  glBlendFunc(GL_ONE, GL_ZERO);

  // Particle depth
  glEnable(GL_DEPTH_TEST);
  glDrawBuffer(GL_NONE);
  drawParticles(_data_program, view, proj);
}

void FluidParticleSystem::blurPass() {
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(_blur_program, "source"), 0);

  glUseProgram(_blur_program);
  auto direction_loc = glGetUniformLocation(_blur_program, "direction");
  auto flip_loc = glGetUniformLocation(_blur_program, "flip");
  auto screen_size_loc = glGetUniformLocation(_blur_program, "screenSize");

  glUniform2f(screen_size_loc, _width, _height);

  auto write_buffer = _blur_fbo[0];
  auto read_buffer = _blur_fbo[1];
  auto write_texture = _blur_texture[0];
  auto read_texture = _blur_texture[1];

  for (auto i = 0; i < kBlurIterations; ++i) {
    auto radius = (kBlurIterations - i - 1) * kBlurRadius;

    glBindFramebuffer(GL_FRAMEBUFFER, write_buffer);
    glBindTexture(GL_TEXTURE_2D, i == 0 ? _data_texture : read_texture);

    auto direction = i % 2 == 0 ? glm::vec2(radius, 0) : glm::vec2(0, radius);
    glUniform1i(flip_loc, 1);
    glUniform2f(direction_loc, direction.x, direction.y);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawQuad();

    std::swap(write_buffer, read_buffer);
    std::swap(write_texture, read_texture);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, _data_fbo);
  glClear(GL_DEPTH_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, read_texture);
  glUniform1i(flip_loc, kBlurIterations % 2 != 0);
  glUniform2f(direction_loc, 0, 0);
  drawQuad();

  glUseProgram(0);
}

void FluidParticleSystem::renderPass(const glm::mat4 &view, const glm::mat4 &proj) {
  glUseProgram(_particle_program);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _data_texture);
  glUniform1i(glGetUniformLocation(_particle_program, "depthTexture"), 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, _thickness_texture);
  glUniform1i(glGetUniformLocation(_particle_program, "thicknessTexture"), 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_CUBE_MAP, _reflection_texture);
  glUniform1i(glGetUniformLocation(_particle_program, "reflectionTexture"), 3);

  drawParticles(_particle_program, view, proj);
}

void FluidParticleSystem::drawParticles(GLuint program,
                                        const glm::mat4 &view,
                                        const glm::mat4 &proj) {
  glUseProgram(program);

  // Send the VP to the vertex shader
  glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(
      glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

  // Pre-compute inverses
  auto inv_view = glm::inverse(view);
  glUniformMatrix4fv(
      glGetUniformLocation(program, "invView"), 1, GL_FALSE, glm::value_ptr(inv_view));
  auto inv_proj = glm::inverse(proj);
  glUniformMatrix4fv(
      glGetUniformLocation(program, "invProjection"), 1, GL_FALSE, glm::value_ptr(inv_proj));

  glBindVertexArray(_vertex_array_object);
  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, _num_particles);

  glUseProgram(0);
}

void FluidParticleSystem::drawQuad() {
  static std::optional<GLuint> _quad_vao;
  if (!_quad_vao) {
    const float positions[] = {1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f};
    const int indices[] = {0, 1, 2, 2, 1, 3};

    _quad_vao = 0;
    glGenVertexArrays(1, &*_quad_vao);
    glBindVertexArray(*_quad_vao);

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

  glBindVertexArray(*_quad_vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}