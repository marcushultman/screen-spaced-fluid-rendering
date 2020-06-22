#pragma once

#include <GL/glew.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <vector>
#include <map>

//Include Assimp
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>

//Include DevIL for image loading
#include <IL/il.h>

#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "textfile.h"

using glm::mat4;
using std::string;
using std::vector;
using std::map;

struct ModelMesh{
	GLuint vertexArrayObject;
	GLuint textureIndex;
	GLuint uniformBlockIndex;
	int numFaces;
};

struct Material{
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	int texCount;
};

class Model
{
private:
	map<string, GLuint> textureIdMap;

	const aiScene* scene = NULL;
	GLuint scene_list = 0;

	GLuint 
		shaderProgram,
		vertexArrayObject;
	// MESHES
	mat4 transform;
	
	string vertexFileName;
	string fragmentFileName;

	Model(string);

	void LoadTextures(string);
	void GenerateVAO();
	Material GenerateMaterial(aiMesh*, ModelMesh *);

	GLuint CreateShaderProgram();

	void DrawNode(const aiNode*);
public:
	vector<ModelMesh> meshes;

	~Model();
	static Model* Load(string);

	void Draw(mat4 view, mat4 projection);
	mat4 GetTransform();
	void SetTransform(mat4);
	void Transform(mat4);
};