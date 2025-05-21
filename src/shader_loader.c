#include "shader_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open shader: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    
    fclose(file);
    return buffer;
}

GLuint compile_shader(const char* path, GLenum type) {
    char* source = read_file(path);
    if (!source) {
        return 0;
    }
    printf("reading shader...\n");

    printf("creating shader...\n");
    GLuint shader = glCreateShader(type);
    printf("1...");
    glShaderSource(shader, 1, (const char* const*)&source, NULL);
    printf("2...");
    glCompileShader(shader);
    printf("3! shader compiled\n");

    free(source);

    // check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "Shader compilation failed (%s):\n%s\n", path, info_log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint create_shader_program(const char* vert_path, const char* frag_path) {
    GLuint vert_shader = compile_shader(vert_path, GL_VERTEX_SHADER);
    printf("vert shader compiled\n");
    if (!vert_shader) return 0;

    GLuint frag_shader = compile_shader(frag_path, GL_FRAGMENT_SHADER);
    printf("frag shader compiled\n");

    if (!frag_shader) {
        glDeleteShader(vert_shader);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    // cleanup shaders
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    // check linking status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "Shader program linking failed:\n%s\n", info_log);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}