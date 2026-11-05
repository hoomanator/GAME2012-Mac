#pragma once
#include <glad/gl.h>
#include "raymath.h"

GLuint GenVertexArray();
GLuint GenVertexBuffer();

void BindVertexBuffer(GLuint vbo);
void UnbindVertexBuffer();

void BindVertexArray(GLuint vao);
void UnbindVertexArray();

void EnableVertexAttribute(GLuint index);
void DisableVertexAttribute(GLuint index);

void SetVertexAttribute(GLuint index, GLint compSize, GLenum type, GLsizei stride);
void UpdateVertexBuffer(GLuint vbo, void* data, int data_size);