#pragma once
#include <glad/gl.h>
#include <vector>
#include "raymath.h"

enum ShapeType
{
	PLANE,
	CUBE,
	SPHERE,
	KNOT
};

struct Mesh
{
	// Number of triangle points in our mesh
	// ie if we have 1 triangle, then count is 3 because 1 triangle is 3 points
	int count = 0;

	// CPU data
	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> tcoords;
	std::vector<uint16_t> indices;

	// GPU data
	GLuint vao = GL_NONE;	// Vertex array object
	GLuint pbo = GL_NONE;	// Position buffer object
	GLuint nbo = GL_NONE;	// Normals buffer object
	GLuint tbo = GL_NONE;	// Tcoords buffer object
	GLuint ebo = GL_NONE;	// Element buffer object (indices)
};

void CreateMesh(Mesh* mesh, const char* path);
void CreateMesh(Mesh* mesh, ShapeType shape);
void DestroyMesh(Mesh* mesh);

void DrawMesh(const Mesh& mesh);