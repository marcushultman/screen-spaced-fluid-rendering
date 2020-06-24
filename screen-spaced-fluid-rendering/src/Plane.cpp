#include "Plane.h"

#include <IL/il.h>
#include <glm/gtc/type_ptr.hpp>
#include "textfile.h"

Plane::Plane(float size) : Plane(size, size) {}

Plane::Plane(float width, float height) {
  const float positions[] = {
      width, 0, height, -width, 0, height, width, 0, -height, -width, 0, -height};
  const float tex_coords[] = {0, 0, 1, 0, 0, 1, 1, 1};
  const int indices[] = {0, 1, 2, 2, 1, 3};

  glGenVertexArrays(1, &_vertex_array_object);
  glBindVertexArray(_vertex_array_object);

  GLuint buffer;

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Load shader

  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  auto vv = textFileRead("screen-spaced-fluid-rendering/resource/shaders/simple.vert");
  auto ff = textFileRead("screen-spaced-fluid-rendering/resource/shaders/simple.frag");

  auto p = vv.c_str();
  glShaderSource(vertex_shader, 1, &p, NULL);
  p = ff.c_str();
  glShaderSource(fragment_shader, 1, &p, NULL);

  glCompileShader(vertex_shader);
  glCompileShader(fragment_shader);

  int ok;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ok);
  assert(ok);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ok);
  assert(ok);

  _program = glCreateProgram();
  glAttachShader(_program, vertex_shader);
  glAttachShader(_program, fragment_shader);
  glLinkProgram(_program);
  glValidateProgram(_program);

  // Load texture

  glGenTextures(1, &_texture);
  glBindTexture(GL_TEXTURE_2D, _texture);

  ILuint image = ilGenImage();
  ilBindImage(image);

  ilLoadImage("screen-spaced-fluid-rendering/resource/ground.jpg");
  ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB,
               ilGetInteger(IL_IMAGE_WIDTH),
               ilGetInteger(IL_IMAGE_HEIGHT),
               0,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               ilGetData());
  ilDeleteImage(image);

  // Set the filtering mode
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Plane::~Plane() = default;

void Plane::draw(const glm::mat4 &view, const glm::mat4 &proj) {
  glUseProgram(_program);

  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  glUniformMatrix4fv(glGetUniformLocation(_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(
      glGetUniformLocation(_program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texture);
  glUniform1i(glGetUniformLocation(_program, "uTexture"), 0);

  glBindVertexArray(_vertex_array_object);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glUseProgram(0);
}
