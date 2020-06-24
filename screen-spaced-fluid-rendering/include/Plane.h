#pragma once

#include <GL/glew.h>
#include <glm/matrix.hpp>

class Plane {
 public:
  Plane(float size);
  Plane(float width, float height);
  ~Plane();

  void draw(const glm::mat4 &view, const glm::mat4 &proj);

 private:
  GLuint _vertex_array_object;
  GLuint _texture;
  GLuint _program;
};
