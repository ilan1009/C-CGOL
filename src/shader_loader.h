#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <glad/glad.h>

GLuint compile_shader(const char* path, GLenum type);
GLuint create_shader_program(const char* vert_path, const char* frag_path);

#endif