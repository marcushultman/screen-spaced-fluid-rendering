#pragma once

#include <GL/glew.h>

class Box {
 public:
  Box(float size);
  ~Box();

  void draw();

 private:
  GLuint _vertex_array_object;
};
