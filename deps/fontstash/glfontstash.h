//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#ifndef GLFONTSTASH_H
#define GLFONTSTASH_H

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "type_ptr.hpp"
#include "matrix_transform.hpp"

#include <iostream>
#include "fontstash.h"

#include <map>
#include <stack>

typedef unsigned int fsuint;

#define BUFFER_SIZE     2
#define N_GLYPH_VERTS   6

struct GLFONSvbo {
    int nverts;
    GLuint buffers[BUFFER_SIZE];
};

typedef struct GLFONSvbo GLFONSvbo;

struct GLStash {
    GLFONSvbo* vbo;
    FONSeffectType effect;
    glm::vec4 bbox;
    float* glyphsXOffset;
    float length;
    int nbGlyph;
};

typedef struct GLStash GLStash;

struct GLFONScontext {
    GLuint tex;
    int width, height;
    // the id counter
    fsuint idct;

    GLuint sdfShaderProgram;
    GLuint defaultShaderProgram;

    GLuint posAttrib;
    GLuint texCoordAttrib;
    GLuint colorAttrib;

    std::map<fsuint, GLStash*>* stashes;

    std::stack<glm::mat4> matrixStack;
    glm::mat4 projectionMatrix;

    glm::vec4 color;
    glm::vec4 outlineColor;

    glm::vec4 sdfProperties;
    float sdfMixFactor;

    glm::mat4 transform;
};

typedef struct GLFONScontext GLFONScontext;

#define STRINGIFY(A) #A

static const GLchar* vertexShaderSrc = STRINGIFY(
#ifdef GL_ES
precision mediump float;
#endif
attribute vec4 a_position;
attribute vec2 a_texCoord;

uniform mat4 u_mvp;
varying vec2 v_uv;

void main() {
    gl_Position = u_mvp * a_position;
    v_uv = a_texCoord;
}
);

static const GLchar* sdfFragShaderSrc = STRINGIFY(
#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
uniform vec4 u_color;
uniform vec4 u_outlineColor;
uniform float u_mixFactor;
uniform float u_minOutlineD;
uniform float u_maxOutlineD;
uniform float u_minInsideD;
uniform float u_maxInsideD;

varying vec2 v_uv;

const float gamma = 2.2;

void main(void) {
    float distance = texture2D(u_tex, v_uv).a;

    float a1 = smoothstep(u_minInsideD, u_maxInsideD, distance);
    float a2 = smoothstep(u_minOutlineD, u_maxOutlineD, distance);

    a1 = pow(a1, 1.0 / gamma);
    a2 = pow(a2, 1.0 / gamma);

    gl_FragColor = mix(u_outlineColor * a2, u_color * a1, u_mixFactor);
}
);

static const GLchar* defaultFragShaderSrc = STRINGIFY(
#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
uniform vec4 u_color;

varying vec2 v_uv;
void main(void) {
    vec4 texColor = texture2D(u_tex, v_uv);
    vec3 invColor = 1.0 - texColor.rgb;
    gl_FragColor = vec4(invColor * u_color.rgb, u_color.a * texColor.a);
}
);

FONScontext* glfonsCreate(int width, int height, int flags);
void glfonsDelete(FONScontext* ctx);

void glfonsBufferText(FONScontext* ctx, const char* s, fsuint* id, FONSeffectType effect);
void glfonsUnbufferText(FONScontext* ctx, fsuint id);

void glfonsRotate(FONScontext* ctx, float angle);
void glfonsTranslate(FONScontext* ctx, float x, float y);
void glfonsScale(FONScontext* ctx, float x, float y);

void glfonsDrawText(FONScontext* ctx, fsuint id);
void glfonsDrawText(FONScontext* ctx, fsuint id, unsigned int from, unsigned int to);
void glfonsSetColor(FONScontext* ctx, float r, float g, float b, float a);
void glfonsSetOutlineColor(FONScontext* ctx, float r, float g, float b, float a);
void glfonsSetSDFProperties(FONScontext* ctx, float minOutlineD, float maxOutlineD, float minInsideD, float maxInsideD, float mixFactor);

void glfonsUpdateViewport(FONScontext* ctx);

void glfonsPushMatrix(FONScontext* ctx);
void glfonsPopMatrix(FONScontext* ctx);

void glfonsGetBBox(FONScontext* ctx, fsuint id, float* x0, float* y0, float* x1, float* y1);
float glfonsGetGlyphOffset(FONScontext* ctx, fsuint id, int i);
float glfonsGetLength(FONScontext* ctx, fsuint id);

unsigned int glfonsRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif

#ifdef GLFONTSTASH_IMPLEMENTATION

#define FONTSTASH_IMPLEMENTATION
#include "fontstash.h"

static void printShaderInfoLog(GLuint shader) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        char* log = (char*)malloc(sizeof(char) * length);
        glGetShaderInfoLog(shader, length, NULL, log);
        printf("Log: %s\n", log);
        free(log);
    }
}

static GLuint compileShader(const GLchar* src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(!status) {
        printShaderInfoLog(shader);
        glDeleteShader(shader);
        exit(-1);
    }
    return shader;
}

static GLuint linkShaderProgram(const GLchar* vertexSrc, const GLchar* fragmentSrc) {
    GLuint program = glCreateProgram();
    GLuint vertex = compileShader(vertexSrc, GL_VERTEX_SHADER);
    GLuint fragment = compileShader(fragmentSrc, GL_FRAGMENT_SHADER);
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if(!linkStatus) {
        printShaderInfoLog(program);
        glDeleteProgram(program);
        exit(-1);
    }
    return program;
}

static int glfons__renderCreate(void* userPtr, int width, int height) {
    GLFONScontext* gl = (GLFONScontext*)userPtr;
    // Create may be called multiple times, delete existing texture.
    if (gl->tex != 0) {
        glDeleteTextures(1, &gl->tex);
        gl->tex = 0;
    }
    gl->idct = 0;
    gl->stashes = new std::map<fsuint, GLStash*>();

    // create texture
    glGenTextures(1, &gl->tex);
    if (!gl->tex) return 0;
    gl->width = width;
    gl->height = height;

    glBindTexture(GL_TEXTURE_2D, gl->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, gl->width, gl->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gl->projectionMatrix = glm::ortho(0.0, (double)viewport[2], (double)viewport[3], 0.0, -1.0, 1.0);

    // create shader
    gl->defaultShaderProgram = linkShaderProgram(vertexShaderSrc, defaultFragShaderSrc);
    gl->sdfShaderProgram = linkShaderProgram(vertexShaderSrc, sdfFragShaderSrc);

    gl->posAttrib = glGetAttribLocation(gl->defaultShaderProgram, "a_position");
    gl->texCoordAttrib = glGetAttribLocation(gl->defaultShaderProgram, "a_texCoord");
    gl->colorAttrib = glGetAttribLocation(gl->defaultShaderProgram, "a_color");

    gl->posAttrib = glGetAttribLocation(gl->sdfShaderProgram, "a_position");
    gl->texCoordAttrib = glGetAttribLocation(gl->sdfShaderProgram, "a_texCoord");
    gl->colorAttrib = glGetAttribLocation(gl->sdfShaderProgram, "a_color");

    gl->matrixStack.push(glm::mat4(1.0));
    gl->color = glm::vec4(1.0);
    gl->transform = glm::mat4(1.0);

    return 1;
}

static int glfons__renderResize(void* userPtr, int width, int height) {
    // Reuse create to resize too.
    return glfons__renderCreate(userPtr, width, height);
}

static void glfons__renderUpdate(void* userPtr, int* rect, const unsigned char* data) {
    GLFONScontext* gl = (GLFONScontext*)userPtr;
    if (gl->tex == 0) return;

    int h = rect[3] - rect[1];

    glBindTexture(GL_TEXTURE_2D, gl->tex);
    const unsigned char* subdata = data + rect[1] * gl->width;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, rect[1], gl->width, h, GL_ALPHA, GL_UNSIGNED_BYTE, subdata);

    glBindTexture(GL_TEXTURE_2D, 0);
}

static void glfons__renderDraw(void* userPtr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts) {
    // called by fontstash, but has nothing to do
}

static void glfons__renderDelete(void* userPtr) {
    GLFONScontext* gl = (GLFONScontext*)userPtr;

    if (gl->tex != 0) {
        glDeleteTextures(1, &gl->tex);
    }

    for(auto& elmt : *gl->stashes) {
        GLStash* stash = elmt.second;
        if(stash != NULL) {
            glDeleteBuffers(BUFFER_SIZE, stash->vbo->buffers);
            delete stash->vbo;
            delete[] stash->glyphsXOffset;
            delete stash;
        }
    }

    glDeleteProgram(gl->defaultShaderProgram);
    glDeleteProgram(gl->sdfShaderProgram);

    delete gl->stashes;
    delete gl;
}

void glfonsUpdateViewport(FONScontext* ctx) {
    GLFONScontext* gl = (GLFONScontext*) ctx->params.userPtr;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gl->projectionMatrix = glm::ortho(0.0, (double)viewport[2], (double)viewport[3], 0.0, -1.0, 1.0);
}

FONScontext* glfonsCreate(int width, int height, int flags) {
    FONSparams params;
    GLFONScontext* gl = new GLFONScontext;

    params.width = width;
    params.height = height;
    params.flags = (unsigned char)flags;
    params.renderCreate = glfons__renderCreate;
    params.renderResize = glfons__renderResize;
    params.renderUpdate = glfons__renderUpdate;
    params.renderDraw = glfons__renderDraw;
    params.renderDelete = glfons__renderDelete;
    params.userPtr = gl;

    return fonsCreateInternal(&params);
}

void glfonsDelete(FONScontext* ctx) {
    fonsDeleteInternal(ctx);
}

unsigned int glfonsRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return (r) | (g << 8) | (b << 16) | (a << 24);
}

void glfonsBufferText(FONScontext* ctx, const char* s, fsuint* id, FONSeffectType effect) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    GLStash* stash = new GLStash;

    stash->vbo = new GLFONSvbo;

    *id = ++glctx->idct;

    fonsDrawText(ctx, 0, 0, s, NULL, 0);

    stash->effect = effect;
    stash->glyphsXOffset = new float[ctx->nverts / N_GLYPH_VERTS];

    int j = 0;
    for(int i = 0; i < ctx->nverts * 2; i += N_GLYPH_VERTS * 2) {
        stash->glyphsXOffset[j++] = ctx->verts[i];
    }

    float inf = std::numeric_limits<float>::infinity();
    float x0 = inf, x1 = -inf, y0 = inf, y1 = -inf;
    for(int i = 0; i < ctx->nverts * 2; i += 2) {
        x0 = ctx->verts[i] < x0 ? ctx->verts[i] : x0;
        x1 = ctx->verts[i] > x1 ? ctx->verts[i] : x1;
        y0 = ctx->verts[i+1] < y0 ? ctx->verts[i+1] : y0;
        y1 = ctx->verts[i+1] > y1 ? ctx->verts[i+1] : y1;
    }

    stash->bbox[0] = x0; stash->bbox[1] = y0;
    stash->bbox[2] = x1; stash->bbox[3] = y1;

    stash->length = ctx->verts[(ctx->nverts*2)-2];

    glGenBuffers(BUFFER_SIZE, stash->vbo->buffers);

    glBindBuffer(GL_ARRAY_BUFFER, stash->vbo->buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * ctx->nverts, ctx->verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, stash->vbo->buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * ctx->nverts, ctx->tcoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    stash->vbo->nverts = ctx->nverts;

    // hack : reset fontstash state
    ctx->nverts = 0;

    glctx->stashes->insert(std::pair<fsuint, GLStash*>(*id, stash));
}

void glfonsUnbufferText(FONScontext* ctx, fsuint id) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;

    if(glctx->stashes->find(id) != glctx->stashes->end()) {
        GLStash* stash = glctx->stashes->at(id);

        glDeleteBuffers(BUFFER_SIZE, stash->vbo->buffers);
        // WIP : maybe release cpu mem
    }
}

void glfonsRotate(FONScontext* ctx, float angle) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    glm::vec3 raxis = glm::vec3(0.0, 0.0, -1.0);
    glctx->transform = glm::rotate(glctx->transform, angle, raxis);
}

void glfonsTranslate(FONScontext* ctx, float x, float y) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    glctx->transform = glm::translate(glctx->transform, glm::vec3(x, y, 0.0));
}

void glfonsScale(FONScontext* ctx, float x, float y) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    glctx->transform = glm::scale(glctx->transform, glm::vec3(x, y, 1.0));
}

void glfonsDrawText(FONScontext* ctx, fsuint id, unsigned int from, unsigned int to) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;

    if(glctx->tex == 0)
        return;

    int d = (to - from) + 1;
    int iFirst = from * N_GLYPH_VERTS;
    int count = d * N_GLYPH_VERTS;

    GLStash* stash = glctx->stashes->at(id);

    glm::mat4 mvp = glctx->projectionMatrix * glctx->transform;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glctx->tex);
    glEnable(GL_TEXTURE_2D);

    GLuint program;
    switch (stash->effect) {
        case FONS_EFFECT_DISTANCE_FIELD:
            program = glctx->sdfShaderProgram;
            break;
        default:
            program = glctx->defaultShaderProgram;
    }

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "u_tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform4f(glGetUniformLocation(program, "u_color"), glctx->color.r, glctx->color.g, glctx->color.b, glctx->color.a);

    if(stash->effect == FONS_EFFECT_DISTANCE_FIELD) {
        glUniform4f(glGetUniformLocation(program, "u_outlineColor"), glctx->outlineColor.r, glctx->outlineColor.g, glctx->outlineColor.b, glctx->outlineColor.a);
        glUniform1f(glGetUniformLocation(program, "u_mixFactor"), glctx->sdfMixFactor);
        glUniform1f(glGetUniformLocation(program, "u_minOutlineD"), glctx->sdfProperties[0]);
        glUniform1f(glGetUniformLocation(program, "u_maxOutlineD"), glctx->sdfProperties[1]);
        glUniform1f(glGetUniformLocation(program, "u_minInsideD"), glctx->sdfProperties[2]);
        glUniform1f(glGetUniformLocation(program, "u_maxInsideD"), glctx->sdfProperties[3]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, stash->vbo->buffers[0]);
    glVertexAttribPointer(glctx->posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glctx->posAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, stash->vbo->buffers[1]);
    glVertexAttribPointer(glctx->texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glctx->texCoordAttrib);

    glDrawArrays(GL_TRIANGLES, iFirst, count);

    glDisableVertexAttribArray(glctx->posAttrib);
    glDisableVertexAttribArray(glctx->texCoordAttrib);
    glDisableVertexAttribArray(glctx->colorAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
}

void glfonsDrawText(FONScontext* ctx, fsuint id) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    GLStash* stash = glctx->stashes->at(id);
    unsigned int last = (stash->vbo->nverts / N_GLYPH_VERTS) - 1;

    glfonsDrawText(ctx, id, 0, last);
}

void glfonsSetColor(FONScontext* ctx, float r, float g, float b, float a) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    glctx->color = glm::vec4(r, g, b, a);
}

void glfonsPushMatrix(FONScontext* ctx) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    glctx->matrixStack.push(glctx->transform);
}

void glfonsPopMatrix(FONScontext* ctx) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;

    if(glctx->matrixStack.size() > 0) {
        glctx->transform = glctx->matrixStack.top();
        glctx->matrixStack.pop();
    }
}

float glfonsGetGlyphOffset(FONScontext* ctx, fsuint id, int i) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    GLStash* stash = glctx->stashes->at(id);

    return stash->glyphsXOffset[i];
}

void glfonsGetBBox(FONScontext* ctx, fsuint id, float* x0, float* y0, float* x1, float* y1) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    GLStash* stash = glctx->stashes->at(id);

    *x0 = stash->bbox[0]; *y0 = stash->bbox[1];
    *x1 = stash->bbox[2]; *y1 = stash->bbox[3];
}

float glfonsGetLength(FONScontext* ctx, fsuint id) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    GLStash* stash = glctx->stashes->at(id);

    return stash->length;
}

void glfonsSetOutlineColor(FONScontext* ctx, float r, float g, float b, float a) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;
    glctx->outlineColor = glm::vec4(r, g, b, a);
}

void glfonsSetSDFProperties(FONScontext* ctx, float minOutlineD, float maxOutlineD, float minInsideD, float maxInsideD, float mixFactor) {
    GLFONScontext* glctx = (GLFONScontext*) ctx->params.userPtr;

    glctx->sdfProperties[0] = minOutlineD; glctx->sdfProperties[1] = maxOutlineD;
    glctx->sdfProperties[2] = minInsideD;  glctx->sdfProperties[3] = maxInsideD;

    glctx->sdfMixFactor = mixFactor;
}

#endif
