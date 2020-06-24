#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Box.h"

class SkyBox {
 public:
  SkyBox();
  ~SkyBox();

  void draw(const glm::mat4 &view, const glm::mat4 &proj);

 private:
  void load();

  Box _box;
  GLuint _texture;
  GLuint _program;
};
