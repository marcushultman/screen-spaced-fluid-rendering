#include "Box.h"

Box::Box(float size) {
  // clang-format off
  const float positions[] = {
      -size, size, size,
      -size, -size, size,
      size, size, size,
      size, -size, size,
      -size, size, -size,
      -size, -size, -size,
      size, size, -size,
      size, -size, -size,
  };
  // clang-format on

  const int indices[] = {
      0, 1, 2,           // Front
      2, 1, 3, 6, 7, 4,  // Back
      4, 7, 5, 4, 5, 0,  // Left
      0, 5, 1, 2, 3, 6,  // Right
      6, 3, 7, 4, 0, 6,  // Top
      6, 0, 2, 7, 3, 5,  // Bottom
      5, 3, 1,
  };

  glGenVertexArrays(1, &_vertex_array_object);
  glBindVertexArray(_vertex_array_object);

  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
}

Box::~Box() = default;

void Box::draw() {
  glBindVertexArray(_vertex_array_object);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
