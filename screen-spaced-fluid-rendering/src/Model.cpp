#include "Model.h"

#include "config.h"

namespace {

GLuint createShaderProgram(const std::string &vertex_shader_filename,
                           const std::string &fragment_shader_filename) {
  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  auto vv = textFileRead(vertex_shader_filename.c_str());
  auto ff = textFileRead(fragment_shader_filename.c_str());

  auto p = vv.c_str();
  glShaderSource(vertex_shader, 1, &p, NULL);
  p = ff.c_str();
  glShaderSource(fragment_shader, 1, &p, NULL);

  int ok;

  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint infoLogLength;
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar strInfoLog[infoLogLength + 1];
    glGetShaderInfoLog(vertex_shader, infoLogLength, NULL, strInfoLog);

    fprintf(stderr, "Compilation error in vertex shader: %s\n", strInfoLog);

    assert(0);
  }

  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint infoLogLength;
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar strInfoLog[infoLogLength + 1];
    glGetShaderInfoLog(fragment_shader, infoLogLength, NULL, strInfoLog);

    fprintf(stderr, "Compilation error in fragment shader: %s\n", strInfoLog);

    assert(0);
  }

  auto program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  glBindFragDataLocation(program, 0, "fragmentColor");

  glBindAttribLocation(program, 0, "position");
  glBindAttribLocation(program, 1, "normal");
  glBindAttribLocation(program, 2, "texCoord");
  glBindAttribLocation(program, 3, "color");

  glLinkProgram(program);
  glValidateProgram(program);

  glUniformBlockBinding(program, glGetUniformBlockIndex(program, "Material"), 0);

  return program;
}

}  // namespace

Model::Model(const std::string &path)
    : _scene{aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality)},
      _program{createShaderProgram(config::kResourcesDir + "/shaders/assimpmodel.vert",
                                   config::kResourcesDir + "/shaders/assimpmodel.frag")} {
  auto basename = path.substr(0, path.find_last_of("/\\") + 1);
  loadTextures(basename);
  createVAO();
}

Model::~Model() {
  aiReleaseImport(_scene);
}

void Model::loadTextures(const std::string &basename) {
  /* scan scene's materials for textures */
  aiString file;
  for (auto i = 0; i < _scene->mNumMaterials; ++i) {
    auto *material = _scene->mMaterials[i];

    auto texure_index = 0;
    auto found = material->GetTexture(aiTextureType_DIFFUSE, texure_index, &file);
    while (found == AI_SUCCESS) {
      // fill map with textures, OpenGL image ids set placeholder to 0
      _texture_id_map[file.data] = 0;
      // more textures?
      found = material->GetTexture(aiTextureType_DIFFUSE, texure_index++, &file);
    }
  }

  auto num_textures = _texture_id_map.size();

  // create and fill array with DevIL texture ids
  ILuint image_ids[num_textures];
  ilGenImages(num_textures, image_ids);

  // create and fill array with GL texture ids
  GLuint texture_ids[num_textures];
  glGenTextures(num_textures, texture_ids);

  std::string image_path;
  auto i = 0;
  for (auto &[filename, id] : _texture_id_map) {
    // save texture id for filename in map
    auto index = i++;
    id = texture_ids[index];

    ilBindImage(image_ids[index]);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    image_path = basename + filename;

    if (ilLoadImage(image_path.c_str())) {
      /* Convert image to RGBA */
      ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

      /* Create and load textures to OpenGL */
      glBindTexture(GL_TEXTURE_2D, id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RGBA,
                   ilGetInteger(IL_IMAGE_WIDTH),
                   ilGetInteger(IL_IMAGE_HEIGHT),
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   ilGetData());
    } else {
      printf("Couldn't load Image: %s\n", filename.c_str());
    }
  }
  /* Because we have already copied image data into texture data
  we can release memory used by image. */
  ilDeleteImages(num_textures, image_ids);
}

void Model::createVAO() {
  GLuint buffer;
  std::vector<unsigned int> indices;
  std::vector<float> tex_coords;

  for (auto i = 0; i < _scene->mNumMeshes; ++i) {
    auto &mesh = *_scene->mMeshes[i];

    _meshes.emplace_back();
    auto &model_mesh = _meshes.back();

    /* Each face is a triangle.
    We mash the indices together.
    */
    indices.resize(3 * mesh.mNumFaces);

    for (auto j = 0; j < mesh.mNumFaces; ++j) {
      auto &face = mesh.mFaces[j];
      auto *triangle = &indices[j * 3];
      triangle[0] = face.mIndices[0];
      triangle[1] = face.mIndices[1];
      triangle[2] = face.mIndices[2];
    }
    model_mesh.num_faces = mesh.mNumFaces;

    // Create a vertex array object
    glGenVertexArrays(1, &model_mesh.vertex_array_object);
    glBindVertexArray(model_mesh.vertex_array_object);

    // Create index buffer
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 3 * mesh.mNumFaces * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    // Create vertex buffer
    if (mesh.HasPositions()) {
      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(
          GL_ARRAY_BUFFER, 3 * mesh.mNumVertices * sizeof(float), mesh.mVertices, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
    }

    // Create normal buffer
    if (mesh.HasNormals()) {
      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(
          GL_ARRAY_BUFFER, 3 * mesh.mNumVertices * sizeof(float), mesh.mNormals, GL_STATIC_DRAW);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, 0);
    }

    // Create texCoord buffer
    if (mesh.HasTextureCoords(0)) {
      tex_coords.resize(2 * mesh.mNumVertices);
      for (auto v = 0; v < mesh.mNumVertices; ++v) {
        tex_coords[v * 2] = mesh.mTextureCoords[0][v].x;
        tex_coords[v * 2 + 1] = mesh.mTextureCoords[0][v].y;
      }
      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 2 * mesh.mNumVertices,
                   tex_coords.data(),
                   GL_STATIC_DRAW);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, 0, 0, 0);
    }

    // Create color buffer
    if (mesh.HasVertexColors(0)) {
      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(
          GL_ARRAY_BUFFER, 3 * mesh.mNumVertices * sizeof(float), mesh.mColors, GL_STATIC_DRAW);
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 3, GL_FLOAT, 0, 0, 0);
    }

    // Unbind vao and buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Create material
    auto material = createMaterial(mesh, model_mesh);

    glGenBuffers(1, &(model_mesh.uniform_block_index));
    glBindBuffer(GL_UNIFORM_BUFFER, model_mesh.uniform_block_index);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(material), &material, GL_STATIC_DRAW);
  }
}

Material Model::createMaterial(const aiMesh &mesh, ModelMesh &model_mesh) {
  // Setup material
  Material material;
  auto *aiMaterial = _scene->mMaterials[mesh.mMaterialIndex];

  aiString path;  // contains filename of texture
  if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
    // Assign texture
    model_mesh.texture_index = _texture_id_map[path.data];
    material.tex_count = 1;
  } else {
    material.tex_count = 0;
  }

  auto diffuse = aiColor4D(0.8f, 0.8f, 0.8f, 1.0f);
  aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
  memcpy(material.diffuse, &diffuse, sizeof(diffuse));

  auto ambient = aiColor4D(0.2f, 0.2f, 0.2f, 1.0f);
  aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_AMBIENT, &ambient);
  memcpy(material.ambient, &ambient, sizeof(ambient));

  auto specular = aiColor4D(0.0f, 0.0f, 0.0f, 1.0f);
  aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_SPECULAR, &specular);
  memcpy(material.specular, &specular, sizeof(specular));

  auto emission = aiColor4D(0.0f, 0.0f, 0.0f, 1.0f);
  aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_EMISSIVE, &emission);
  memcpy(material.emissive, &emission, sizeof(emission));

  float shininess = 0.0;
  unsigned int max;
  aiGetMaterialFloatArray(aiMaterial, AI_MATKEY_SHININESS, &shininess, &max);
  material.shininess = shininess;

  return material;
}

void Model::drawNode(const aiNode &node) {
  // Send the transform to the vertex shader
  glUniformMatrix4fv(
      glGetUniformLocation(_program, "modelMatrix"), 1, GL_TRUE, &node.mTransformation.a1);

  // Draw all meshes assigned to this node
  for (auto i = 0; i < node.mNumMeshes; ++i) {
    const auto &mesh = _meshes[node.mMeshes[i]];
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mesh.uniform_block_index, 0, sizeof(Material));
    glBindTexture(GL_TEXTURE_2D, mesh.texture_index);
    glBindVertexArray(mesh.vertex_array_object);
    glDrawElements(GL_TRIANGLES, mesh.num_faces * 3, GL_UNSIGNED_INT, 0);
  }

  // Draw all children
  for (auto i = 0; i < node.mNumChildren; ++i) {
    drawNode(*node.mChildren[i]);
  }
}

void Model::draw(const glm::mat4 &view, const glm::mat4 &projection) {
  glUseProgram(_program);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  // Send the VP to the vertex shader
  glUniformMatrix4fv(glGetUniformLocation(_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(
      glGetUniformLocation(_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

  // if the display list has not been made yet, create a new one and
  // fill it with scene contents
  if (!_scene_list) {
    _scene_list = glGenLists(1);
    glNewList(_scene_list, GL_COMPILE);
    drawNode(*_scene->mRootNode);
    glEndList();
  }

  glCallList(_scene_list);

  glUseProgram(0);
}
