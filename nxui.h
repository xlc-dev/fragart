/* Copyright (C) 2024 Nexus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Mozilla Public License, version 2.0.
 *
 * This program is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 * See the Mozilla Public License for more details.
 *
 * You should have received a copy of the Mozilla Public License
 * along with this program. If not, see <https://www.mozilla.org/MPL/2.0/>.
 *
 * ===== DOCUMENTATION ====================================================
 *
 * The goal single header library written in ANSI C is to provide a simple
 * and easy to use API for creating OpenGL UIs and dealing with most of the
 * boilerplate code. With this header you can make your own UI framework much
 * easier.
 *
 * The only dependency is glad, which is included in the repository.
 * But you can disable glad by defining NXUI_NO_GLAD.
 *
 * ===== USAGE ============================================================
 *
 * To use the implementation of this library, define the following macro
 * before including this file:
 *
 *    #define NXUI_IMPLEMENTATION
 *
 * i.e. it should look like this:
 *    #include ...
 *    #include ...
 *    #define NXUI_IMPLEMENTATION
 *    #include "nexus.h"
 *
 * ===== OPTIONS ==========================================================
 *
 *    #define NXUI_NO_GLAD
 *        Disables the inclusion of glad.h. This is useful if you want to
 *        use your own OpenGL loader.
 *
 * ===== VERSIONING =======================================================
 * Version: 0.0.2
 * Release Date: 05-04-2025
 *
 * Changelog:
 * - Add mode to NXUIMesh
 * - Add vec2 and vec3 uniform setters
 * - Add clear function
 *
 * Version: 0.0.1
 * Release Date: 21-12-2024
 *
 * Changelog:
 * - Initial version (v0.0.1)
 *   - NXUIContext
 *   - NXUIShaderProgram
 *   - NXUIMesh
 *   - nxui_context_init
 *   - nxui_context_add_shader
 *   - nxui_context_add_mesh
 *   - nxui_context_destroy
 *   - nxui_render_ui
 *   - nxui_create_shader_program
 *   - nxui_create_shader_program_from_files
 *   - nxui_use_shader_program
 *   - nxui_delete_shader_program
 *   - nxui_create_mesh
 *   - nxui_set_uniform_float
 *   - nxui_set_uniform_int
 *   - nxui_set_uniform_vec4
 */
#ifndef NXUI_H
#define NXUI_H

#include "nexus.h"

#include <stddef.h>

#ifndef NXUI_NO_GLAD
#include "glad/glad.h"
#endif

typedef struct {
    GLuint program_id;
} NXUIShaderProgram;

typedef struct {
    GLenum             mode;
    GLuint             vao;
    GLuint             vbo;
    GLuint             ebo;
    int                index_count;
    NXUIShaderProgram *shader;
} NXUIMesh;

typedef struct {
    NXUIShaderProgram *shaders;
    size_t             shader_count;
    NXUIMesh          *meshes;
    size_t             mesh_count;
} NXUIContext;

typedef struct {
    GLuint      index;
    GLint       size;
    GLenum      type;
    GLboolean   normalized;
    GLsizei     stride;
    const void *offset;
} NXUIAttribute;

NXUIContext *nxui_context_init(void);
void         nxui_context_add_shader(NXUIContext *context, NXUIShaderProgram shader);
void         nxui_context_add_mesh(NXUIContext *context, NXUIMesh mesh);
void         nxui_context_destroy(NXUIContext *context);
void         nxui_render_ui(NXUIContext *context);

NXUIShaderProgram nxui_create_shader_program_from_files(const char *vertex_path,
                                                        const char *fragment_path);
NXUIShaderProgram nxui_create_shader_program(const char *vertex_source,
                                             const char *fragment_source);
void              nxui_use_shader_program(NXUIShaderProgram *shader);
void              nxui_delete_shader_program(NXUIShaderProgram *shader);

NXUIMesh nxui_create_mesh(const void *vertex_data, size_t vertex_size, const unsigned int *indices,
                          size_t index_size, int attribute_count, const NXUIAttribute *attributes,
                          GLenum usage);

void nxui_set_uniform_float(NXUIShaderProgram *shader, const char *name, float value);
void nxui_set_uniform_int(NXUIShaderProgram *shader, const char *name, int value);
void nxui_set_uniform_vec2(NXUIShaderProgram *shader, const char *name, float x, float y);
void nxui_set_uniform_vec3(NXUIShaderProgram *shader, const char *name, float x, float y, float z);
void nxui_set_uniform_vec4(NXUIShaderProgram *shader, const char *name, float x, float y, float z,
                           float w);

void nxui_clear(float r, float g, float b, float a);
#endif /* NXUI_H */

#ifdef NXUI_IMPLEMENTATION
static char *_nxui_read_shader_source(const char *file_path) {
    char *source = nx_file_read_all(file_path);
    if (!source) {
        nx_die1("Failed to read shader file: %s", file_path);
    }
    return source;
}

static GLuint _nxui_compile_shader(const char *source, GLenum shader_type) {
    GLuint shader;
    GLint  success;
    char   infoLog[512];
    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (shader_type == GL_VERTEX_SHADER) {
            nx_die1("Vertex shader compilation failed:\n%s", infoLog);
        } else if (shader_type == GL_FRAGMENT_SHADER) {
            nx_die1("Fragment shader compilation failed:\n%s", infoLog);
        } else {
            nx_die1("Unknown shader type compilation failed:\n%s", infoLog);
        }
    }
    return shader;
}

static GLuint _nxui_create_vao(void) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    if (!vao) {
        nx_die("Failed to generate VAO");
    }
    glBindVertexArray(vao);
    return vao;
}

static GLuint _nxui_create_vbo(const void *data, size_t size, GLenum usage) {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    if (!vbo) {
        nx_die("Failed to generate VBO");
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) size, data, usage);
    return vbo;
}

static GLuint _nxui_create_ebo(const void *data, size_t size, GLenum usage) {
    GLuint ebo;
    glGenBuffers(1, &ebo);
    if (!ebo) {
        nx_die("Failed to generate EBO");
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) size, data, usage);
    return ebo;
}

NXUIShaderProgram nxui_create_shader_program(const char *vertex_source,
                                             const char *fragment_source) {
    NXUIShaderProgram shader_program;
    GLuint            vertex_shader, fragment_shader;
    GLint             success;
    char              infoLog[512];
    vertex_shader             = _nxui_compile_shader(vertex_source, GL_VERTEX_SHADER);
    fragment_shader           = _nxui_compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    shader_program.program_id = glCreateProgram();
    glAttachShader(shader_program.program_id, vertex_shader);
    glAttachShader(shader_program.program_id, fragment_shader);
    glLinkProgram(shader_program.program_id);
    glGetProgramiv(shader_program.program_id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program.program_id, 512, NULL, infoLog);
        nx_die1("Shader program linking failed:\n%s", infoLog);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader_program;
}

NXUIShaderProgram nxui_create_shader_program_from_files(const char *vertex_path,
                                                        const char *fragment_path) {
    char             *vertex_source   = _nxui_read_shader_source(vertex_path);
    char             *fragment_source = _nxui_read_shader_source(fragment_path);
    NXUIShaderProgram shader_program  = nxui_create_shader_program(vertex_source, fragment_source);
    nx_free(vertex_source);
    nx_free(fragment_source);
    return shader_program;
}

void nxui_use_shader_program(NXUIShaderProgram *shader) {
    glUseProgram(shader->program_id);
}

void nxui_delete_shader_program(NXUIShaderProgram *shader) {
    if (shader->program_id) {
        glDeleteProgram(shader->program_id);
        shader->program_id = 0;
    }
}

NXUIMesh nxui_create_mesh(const void *vertex_data, size_t vertex_size, const unsigned int *indices,
                          size_t index_size, int attribute_count, const NXUIAttribute *attributes,
                          GLenum usage) {
    NXUIMesh      mesh;
    unsigned int *auto_indices = NULL;
    size_t        i;

    mesh.mode = GL_TRIANGLES;
    mesh.vao  = _nxui_create_vao();
    mesh.vbo  = _nxui_create_vbo(vertex_data, vertex_size, usage);

    if (!indices && vertex_size > 0) {
        size_t stride_sz = attributes[0].stride ? (size_t) attributes[0].stride : sizeof(float);
        size_t count     = vertex_size / stride_sz;
        auto_indices     = nx_malloc(sizeof(unsigned int) * count);

        if (!auto_indices) {
            nx_die("Failed to allocate auto-generated indices");
        }

        for (i = 0; i < count; i++) {
            auto_indices[i] = (unsigned int) i;
        }

        indices    = auto_indices;
        index_size = count * sizeof(unsigned int);
    }

    mesh.ebo         = 0;
    mesh.index_count = 0;

    if (indices && index_size) {
        mesh.ebo         = _nxui_create_ebo(indices, index_size, usage);
        mesh.index_count = (int) (index_size / sizeof(unsigned int));
    }

    for (i = 0; i < (size_t) attribute_count; i++) {
        glEnableVertexAttribArray(attributes[i].index);
        glVertexAttribPointer(attributes[i].index, attributes[i].size, attributes[i].type,
                              attributes[i].normalized, attributes[i].stride, attributes[i].offset);
    }

    glBindVertexArray(0);
    mesh.shader = NULL;

    if (auto_indices) {
        nx_free(auto_indices);
    }

    return mesh;
}

NXUIContext *nxui_context_init(void) {
    NXUIContext *context = nx_malloc(sizeof(NXUIContext));
    if (!context) {
        nx_die("Failed to allocate NXUIContext");
    }
    context->shaders      = NULL;
    context->shader_count = 0;
    context->meshes       = NULL;
    context->mesh_count   = 0;
    return context;
}

void nxui_context_add_shader(NXUIContext *context, NXUIShaderProgram shader) {
    context->shaders =
        nx_realloc(context->shaders, sizeof(NXUIShaderProgram) * (context->shader_count + 1));
    if (!context->shaders) {
        nx_die("Failed to allocate memory for shaders");
    }
    context->shaders[context->shader_count] = shader;
    context->shader_count++;
}

void nxui_context_add_mesh(NXUIContext *context, NXUIMesh mesh) {
    context->meshes = nx_realloc(context->meshes, sizeof(NXUIMesh) * (context->mesh_count + 1));
    if (!context->meshes) {
        nx_die("Failed to allocate memory for meshes");
    }
    context->meshes[context->mesh_count] = mesh;
    context->mesh_count++;
}

void nxui_context_destroy(NXUIContext *context) {
    size_t i;
    if (!context) {
        return;
    }
    for (i = 0; i < context->shader_count; i++) {
        nxui_delete_shader_program(&context->shaders[i]);
    }
    for (i = 0; i < context->mesh_count; i++) {
        glDeleteVertexArrays(1, &context->meshes[i].vao);
        glDeleteBuffers(1, &context->meshes[i].vbo);
        glDeleteBuffers(1, &context->meshes[i].ebo);
    }
    nx_free(context->shaders);
    nx_free(context->meshes);
    nx_free(context);
}

void nxui_render_ui(NXUIContext *context) {
    size_t i, j;
    for (i = 0; i < context->shader_count; i++) {
        nxui_use_shader_program(&context->shaders[i]);
        for (j = 0; j < context->mesh_count; j++) {
            if (context->meshes[j].shader == &context->shaders[i]) {
                glBindVertexArray(context->meshes[j].vao);
                glDrawElements(context->meshes[j].mode, context->meshes[j].index_count,
                               GL_UNSIGNED_INT, 0);
            }
        }
    }
    glBindVertexArray(0);
}

void nxui_set_uniform_float(NXUIShaderProgram *shader, const char *name, float value) {
    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location == -1) {
        nx_die1("Uniform '%s' not found in shader program.", name);
    }
    glUniform1f(location, value);
}

void nxui_set_uniform_int(NXUIShaderProgram *shader, const char *name, int value) {
    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location == -1) {
        nx_die1("Uniform '%s' not found in shader program.", name);
    }
    glUniform1i(location, value);
}

void nxui_set_uniform_vec2(NXUIShaderProgram *shader, const char *name, float x, float y) {
    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location == -1) {
        nx_die1("Uniform '%s' not found in shader program.", name);
    }
    glUniform2f(location, x, y);
}

void nxui_set_uniform_vec3(NXUIShaderProgram *shader, const char *name, float x, float y, float z) {
    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location == -1) {
        nx_die1("Uniform '%s' not found in shader program.", name);
    }
    glUniform3f(location, x, y, z);
}

void nxui_set_uniform_vec4(NXUIShaderProgram *shader, const char *name, float x, float y, float z,
                           float w) {
    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location == -1) {
        nx_die1("Uniform '%s' not found in shader program.", name);
    }
    glUniform4f(location, x, y, z, w);
}

void nxui_clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}
#endif /* NXUI_IMPLEMENTATION */
