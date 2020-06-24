#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <GL/glew.h>
#include <IL/il.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/vector3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include "textfile.h"

struct ModelMesh {
  GLuint vertex_array_object;
  GLuint texture_index;
  GLuint uniform_block_index;
  int num_faces;
};

struct Material {
  float diffuse[4];
  float ambient[4];
  float specular[4];
  float emissive[4];
  float shininess;
  int tex_count;
};

class Model {
 public:
  Model(const std::string &path);
  ~Model();

  void draw(const glm::mat4 &view, const glm::mat4 &projection);

 private:
  void loadTextures(const std::string &basename);
  void createVAO();
  Material createMaterial(const aiMesh &mesh, ModelMesh &);

  void drawNode(const aiNode &);

  std::unordered_map<std::string, GLuint> _texture_id_map;

  const aiScene *_scene = nullptr;
  GLuint _scene_list = 0;
  GLuint _program;
  std::vector<ModelMesh> _meshes;
};
