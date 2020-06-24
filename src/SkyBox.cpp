#include "SkyBox.h"

#include <string>
#include <vector>
#include <IL/il.h>
#include <glm/gtc/type_ptr.hpp>
#include "config.h"
#include "textfile.h"

SkyBox::SkyBox() : _box(1) {
  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  auto vv = textFileRead(config::kResourcesDir + "/shaders/skybox.vert");
  auto ff = textFileRead(config::kResourcesDir + "/shaders/skybox.frag");

  auto p = vv.c_str();
  glShaderSource(vertex_shader, 1, &p, NULL);
  p = ff.c_str();
  glShaderSource(fragment_shader, 1, &p, NULL);

  glCompileShader(vertex_shader);
  glCompileShader(fragment_shader);

  _program = glCreateProgram();
  glAttachShader(_program, vertex_shader);
  glAttachShader(_program, fragment_shader);
  glLinkProgram(_program);
  glValidateProgram(_program);

  // Load texture cube

  glGenTextures(1, &_texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);

  std::vector<std::pair<GLenum, std::string>> faces = {
      {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "miramar_bk.tga"},
      {GL_TEXTURE_CUBE_MAP_POSITIVE_X, "miramar_ft.tga"},

      {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "miramar_dn.tga"},
      {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "miramar_up.tga"},

      {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "miramar_lf.tga"},
      {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "miramar_rt.tga"},
  };

  ILuint image_ids[faces.size()];
  ilGenImages(faces.size(), image_ids);

  for (auto i = 0; i < faces.size(); ++i) {
    auto &[face, filename] = faces[i];
    ilBindImage(image_ids[i]);

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
  ilDeleteImages(faces.size(), image_ids);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

SkyBox::~SkyBox() = default;

void SkyBox::draw(const glm::mat4 &view, const glm::mat4 &proj) {
  glUseProgram(_program);

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  glUniformMatrix4fv(glGetUniformLocation(_program, "view"), 1, GL_FALSE, glm::value_ptr(view));

  glUniformMatrix4fv(
      glGetUniformLocation(_program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

  // Extract camera position
  auto camera_pos = glm::vec3(glm::inverse(view)[3]);

  glUniform3fv(glGetUniformLocation(_program, "cameraPosition"), 1, glm::value_ptr(camera_pos));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
  glUniform1i(glGetUniformLocation(_program, "uTexture"), 0);

  _box.draw();

  glUseProgram(0);
}
