#include "FluidParticle.h"

extern float nearPlane, farPlane;

// Contructor
FluidParticle::FluidParticle(float size)
{
	// Vertex positions
	const float positions [] = {
		// X	Y	Z
		size, size, 0,
		-size, size, 0,
		size, -size, 0,
		-size, -size, 0,
	};

	const float texCoords [] = {
		1, 1,
		0, 1,
		1, 0,
		0, 0,
	};

	const int indices [] = {
		0, 1, 2,
		2, 1, 3,
	};

	GLuint buffer;

	// Create the vertex array object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	// Create the buffer objects
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Create buffer for instanced transforms
	glGenBuffers(1, &transformBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
	for (unsigned int i = 0; i < 4; i++)
	{
		unsigned int attrib_index = 2 + i;
		glEnableVertexAttribArray(attrib_index);
		glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), 
			(const void*) (sizeof(GL_FLOAT) * i * 4));
		glVertexAttribDivisor(attrib_index, 1);
	}

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	

#pragma region Shader
	{
		GLuint vertexShader, fragmentShader;

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const char * vv = textFileRead(vertexShaderFile);
		const char * ff = textFileRead(fragmentShaderFile);

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

			throw;
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

			throw;
		}

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);

		glLinkProgram(shaderProgram);
		glValidateProgram(shaderProgram);

		glUseProgram(shaderProgram);
		glUniform1f(glGetUniformLocation(shaderProgram, "sphereRadius"), size);
		glUniform1f(glGetUniformLocation(shaderProgram, "znear"), nearPlane);
		glUniform1f(glGetUniformLocation(shaderProgram, "zfar"), farPlane);
		glUseProgram(0);
	}
#pragma endregion

#pragma region Particle data
	{
		GLuint vertexShader, fragmentShader;

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const char * vv = textFileRead(particleDataVertexShaderFile);
		const char * ff = textFileRead(particleDataFragmentShaderFile);

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

			throw;
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

			throw;
		}

		particleDataShaderProgram = glCreateProgram();
		glAttachShader(particleDataShaderProgram, vertexShader);
		glAttachShader(particleDataShaderProgram, fragmentShader);

		glLinkProgram(particleDataShaderProgram);
		glValidateProgram(particleDataShaderProgram);

		glUseProgram(particleDataShaderProgram);
		glUniform1f(glGetUniformLocation(particleDataShaderProgram, "sphereRadius"), size);
		glUniform1f(glGetUniformLocation(particleDataShaderProgram, "znear"), nearPlane);
		glUniform1f(glGetUniformLocation(particleDataShaderProgram, "zfar"), farPlane);
		glUseProgram(0);
	}
#pragma endregion

}

FluidParticle::~FluidParticle()
{
}

mat4 CreateBillboardTransform(vec3 position, vec3 cameraPosition, vec3 cameraUp) {
	vec3 look = glm::normalize(cameraPosition - position);
	vec3 right = glm::cross(cameraUp, look);
	vec3 up2 = glm::cross(look, right);
	mat4 transform(1);
	transform[0] = vec4(right, 0);
	transform[1] = vec4(up2, 0);
	transform[2] = vec4(look, 0);
	// Uncomment this line to translate the position as well
	// (without it, it's just a rotation)
	//transform[3] = vec4(position, 1);
	return transform;
}

void FluidParticle::DrawParticleData(mat4 view, mat4 projection)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(particleDataShaderProgram);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(particleDataShaderProgram, "view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(particleDataShaderProgram, "projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	// Extract camera position

	vec3 cameraPosition = vec3(glm::column(glm::inverse(view), 3));
	vec3 cameraPosition2 = vec3(glm::inverse(view)[3]);
	mat4 transform = CreateBillboardTransform(cameraPosition, cameraPosition, vec3(0, 1, 0));

	// Send the world to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(particleDataShaderProgram, "world"),
		1, GL_FALSE, glm::value_ptr(transform));

	glBindVertexArray(vertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
}

void FluidParticle::DrawParticleDataInstanced(mat4 view, mat4 projection, unsigned int numelem, const mat4* transforms)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(particleDataShaderProgram);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(particleDataShaderProgram, "view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(particleDataShaderProgram, "projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	// Apply billboard to each transfrom
	/*vec3 cameraPosition(glm::column(glm::inverse(view), 3));
	vec3 up(0, 1, 0);
	mat4* t = new mat4[numelem];
	for (int i = 0; i < numelem; i++)
	{
		vec3 position = vec3(glm::column(transforms[i], 3));
		t[i] = transforms[i] * CreateBillboardTransform(position, cameraPosition, up);
	}*/

	glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * numelem, t, GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * numelem, transforms, GL_DYNAMIC_DRAW);

	glBindVertexArray(vertexArrayObject);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numelem);

	glUseProgram(0);
}

void FluidParticle::Draw(mat4 view, mat4 projection)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(shaderProgram);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	// Extract camera position

	vec3 cameraPosition = vec3(glm::inverse(view)[3]);
	mat4 transform = CreateBillboardTransform(position, cameraPosition, vec3(0, 1, 0));

	// Send the world to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "world"),
		1, GL_FALSE, glm::value_ptr(transform));

	glBindVertexArray(vertexArrayObject);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
}

void FluidParticle::DrawInstanced(mat4 view, mat4 projection, unsigned int numelem, const mat4* transforms)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(shaderProgram);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	// Apply billboard to each transfrom
	/*vec3 cameraPosition(glm::column(glm::inverse(view), 3));
	vec3 up(0, 1, 0);
	mat4* t = new mat4[numelem];
	for (int i = 0; i < numelem; i++)
	{
		vec3 position = vec3(glm::column(transforms[i], 3));
		t[i] = transforms[i] * CreateBillboardTransform(position, cameraPosition, up);
	}*/

	glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * numelem, transforms, GL_DYNAMIC_DRAW);

	glBindVertexArray(vertexArrayObject);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numelem);

	glUseProgram(0);
}