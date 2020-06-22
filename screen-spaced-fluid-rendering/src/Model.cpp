#include "Model.h"

Model* Model::Load(string path)
{
	Model* model = new Model(path);
	return model;
}

Model::Model(string path)
{
	string folderPath(path);
	folderPath = folderPath.substr(0, folderPath.find_last_of("/\\") + 1);

	scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

	LoadTextures(folderPath);
	GenerateVAO();

	vertexFileName = "screen-spaced-fluid-rendering/resource/shaders/assimpmodel.vert";
	fragmentFileName = "screen-spaced-fluid-rendering/resource/shaders/assimpmodel.frag";

	shaderProgram = CreateShaderProgram();
}

Model::~Model()
{
	aiReleaseImport(scene);
}

void Model::LoadTextures(string folderPath)
{
	ILboolean success;

	/* scan scene's materials for textures */
	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		int texureIndex = 0;
		aiString file;
		aiReturn textureFound = material->GetTexture(aiTextureType_DIFFUSE, texureIndex, &file);
		while (textureFound == AI_SUCCESS) {
			// fill map with textures, OpenGL image ids set placeholder to 0
			textureIdMap[file.data] = 0;
			// more textures?
			textureFound = material->GetTexture(aiTextureType_DIFFUSE, texureIndex++, &file);
		}
	}

	int numTextures = textureIdMap.size();

	/* create and fill array with DevIL texture ids */
	ILuint* imageIds = new ILuint[numTextures];
	ilGenImages(numTextures, imageIds);

	/* create and fill array with GL texture ids */
	GLuint* textureIds = new GLuint[numTextures];
	glGenTextures(numTextures, textureIds); /* Texture name generation */

	/* get iterator */
	std::map<string, GLuint>::iterator itr = textureIdMap.begin();
	for (unsigned int i = 0; itr != textureIdMap.end(); i++, itr++)
	{
		//save IL image ID
		string filename = (*itr).first;  // get filename
		(*itr).second = textureIds[i];	  // save texture id for filename in map

		ilBindImage(imageIds[i]); /* Binding of DevIL image name */
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		string imagePath = folderPath + filename;
		success = ilLoadImage(imagePath.c_str());

		if (success) {
			/* Convert image to RGBA */
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

			/* Create and load textures to OpenGL */
			glBindTexture(GL_TEXTURE_2D, textureIds[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
				ilGetData());
		}
		else
			printf("Couldn't load Image: %s\n", filename.c_str());
	}
	/* Because we have already copied image data into texture data
	we can release memory used by image. */
	ilDeleteImages(numTextures, imageIds);

	//Cleanup
	delete [] imageIds;
	delete [] textureIds;
}

void Model::GenerateVAO()
{
	ModelMesh modelMesh;
	GLuint buffer;

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];

		/* Each face is a triangle.
		We mash the indices together.
		*/
		unsigned int *indices;
		indices = (unsigned int *) malloc(
			3 * mesh->mNumFaces *
			sizeof(unsigned int));

		for (unsigned int j = 0, index = 0;
			j < mesh->mNumFaces; j++,
			index += 3)
		{
			aiFace* face = &mesh->mFaces[j];
			memcpy(&indices[index], face->mIndices,
				3 * sizeof(unsigned int));
		}
		modelMesh.numFaces = mesh->mNumFaces;

		// Create a vertex array object
		glGenVertexArrays(1, &modelMesh.vertexArrayObject);
		glBindVertexArray(modelMesh.vertexArrayObject);

		// Create index buffer
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			3 * mesh->mNumFaces * sizeof(unsigned int),
			indices,
			GL_STATIC_DRAW);

		// Create vertex buffer
		if (mesh->HasPositions()) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER,
				3 * mesh->mNumVertices * sizeof(float),
				mesh->mVertices,
				GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
		}

		// Create normal buffer
		if (mesh->HasNormals()) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER,
				3 * mesh->mNumVertices * sizeof(float),
				mesh->mNormals,
				GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, 0);
		}

		// Create texCoord buffer
		if (mesh->HasTextureCoords(0)) {
			float *texCoord = (float *) malloc(
				2 * mesh->mNumVertices *
				sizeof(float));
			for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
				texCoord[v * 2] = mesh->mTextureCoords[0][v].x;
				texCoord[v * 2 + 1] = mesh->mTextureCoords[0][v].y;
			}
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, texCoord, GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, 0, 0, 0);
		}

		// Create color buffer
		if (mesh->HasVertexColors(0)) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER,
				3 * mesh->mNumVertices *
				sizeof(float),
				mesh->mColors,
				GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, 0, 0, 0);
		}

		// Unbind vao and buffers
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Generate material
		Material material = GenerateMaterial(mesh, &modelMesh);

		glGenBuffers(1, &(modelMesh.uniformBlockIndex));
		glBindBuffer(GL_UNIFORM_BUFFER, modelMesh.uniformBlockIndex);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(material), (void *) (&material), GL_STATIC_DRAW);

		// Copy material to list
		meshes.push_back(modelMesh);
	}

}

void color4_to_float4(const aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

Material Model::GenerateMaterial(aiMesh* mesh, ModelMesh* modelMesh)
{
	// Setup material
	Material material;
	aiMaterial *aiMaterial = scene->mMaterials[mesh->mMaterialIndex];

	aiString path;	//contains filename of texture
	if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path)){
		//Assign texture
		modelMesh->textureIndex = textureIdMap[path.data];
		material.texCount = 1;
	}
	else
		material.texCount = 0;

	float c[4];
	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	aiColor4D diffuse;
	if (AI_SUCCESS == aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		color4_to_float4(&diffuse, c);
	memcpy(material.diffuse, c, sizeof(c));

	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	aiColor4D ambient;
	if (AI_SUCCESS == aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_AMBIENT, &ambient))
		color4_to_float4(&ambient, c);
	memcpy(material.ambient, c, sizeof(c));

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	aiColor4D specular;
	if (AI_SUCCESS == aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_SPECULAR, &specular))
		color4_to_float4(&specular, c);
	memcpy(material.specular, c, sizeof(c));

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	aiColor4D emission;
	if (AI_SUCCESS == aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
	memcpy(material.emissive, c, sizeof(c));

	float shininess = 0.0;
	unsigned int max;
	aiGetMaterialFloatArray(aiMaterial, AI_MATKEY_SHININESS, &shininess, &max);
	material.shininess = shininess;

	return material;
}

GLuint Model::CreateShaderProgram()
{
	GLuint program, vertexShader, fragmentShader;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	auto vv = textFileRead(vertexFileName.c_str());
	auto ff = textFileRead(fragmentFileName.c_str());

	glShaderSource(vertexShader, 1, &vv, NULL);
	glShaderSource(fragmentShader, 1, &ff, NULL);

	int compileOK;

	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK) {
		GLint infoLogLength;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(vertexShader, infoLogLength, NULL, strInfoLog);

		fprintf(stderr, "Compilation error in vertexShader: %s\n", strInfoLog);
		delete [] strInfoLog;

		return NULL;
	}

	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK) {
		GLint infoLogLength;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(fragmentShader, infoLogLength, NULL, strInfoLog);

		fprintf(stderr, "Compilation error in fragmentShader: %s\n", strInfoLog);
		delete [] strInfoLog;

		return NULL;
	}

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glBindFragDataLocation(program, 0, "fragmentColor");

	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "normal");
	glBindAttribLocation(program, 2, "texCoord");
	glBindAttribLocation(program, 3, "color");

	glLinkProgram(program);
	glValidateProgram(program);

	/*glUniformBlockBinding(program,
	glGetUniformBlockIndex(program, "Matrices"),
	matricesUniLoc);*/
	glUniformBlockBinding(program,
		glGetUniformBlockIndex(program, "Material"),
		0);

	return(program);
}

void Model::DrawNode(const aiNode* node)
{
	// Send the transform to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"),
		1, GL_TRUE, &node->mTransformation.a1);

	// Draw all meshes assigned to this node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		unsigned int meshIndex = node->mMeshes[i];
		ModelMesh mesh = meshes[meshIndex];

		// Bind material
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, mesh.uniformBlockIndex, 0, sizeof(struct Material));
		// Bind texture
		glBindTexture(GL_TEXTURE_2D, mesh.textureIndex);
		// Bind vao
		glBindVertexArray(mesh.vertexArrayObject);
		// Draw
		glDrawElements(GL_TRIANGLES, mesh.numFaces * 3, GL_UNSIGNED_INT, 0);
	}

	// Draw all children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		DrawNode(node->mChildren[i]);
	}
}

void Model::Draw(mat4 view, mat4 projection)
{
	glUseProgram(shaderProgram);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, 
		"view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram,
		"projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	// if the display list has not been made yet, create a new one and
	// fill it with scene contents
	if (scene_list == 0) {
		glNewList((scene_list = glGenLists(1)), GL_COMPILE);
		DrawNode(scene->mRootNode);
		glEndList();
	}

	glCallList(scene_list);

	glUseProgram(0);
}