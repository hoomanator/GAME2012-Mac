#include "Buffer.h"
#include <cassert>

static GLuint f_vao = GL_NONE;
static GLuint f_vbo = GL_NONE;

GLuint GenVertexArray()
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	return vao;
}

GLuint GenVertexBuffer()
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	return vbo;
}

void BindVertexBuffer(GLuint vbo)
{
	assert(f_vbo == GL_NONE);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	f_vbo = vbo;
}

void UnbindVertexBuffer()
{
	assert(f_vbo != GL_NONE);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	f_vbo = GL_NONE;
}

void BindVertexArray(GLuint vao)
{
	assert(f_vao == GL_NONE);
	glBindVertexArray(vao);
	f_vao = vao;
}

void UnbindVertexArray()
{
	assert(f_vao != GL_NONE);
	glBindVertexArray(GL_NONE);
	f_vao = GL_NONE;
}

void EnableVertexAttribute(GLuint index)
{
	glEnableVertexAttribArray(index);
}

void DisableVertexAttribute(GLuint index)
{
	glDisableVertexAttribArray(index);
}

void SetVertexAttribute(GLuint index, GLint compSize, GLenum type, GLsizei stride/*, int offset*/)
{
	// No interleaved vertex data in order to simplify our interface (offset is always nullptr)
	glVertexAttribPointer(index, compSize, type, GL_FALSE, stride, nullptr);
}

void UpdateVertexBuffer(GLuint vbo, void* data, int data_size)
{
	assert(f_vbo != GL_NONE);
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
}
