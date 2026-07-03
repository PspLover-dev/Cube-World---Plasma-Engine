#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "plasma/OpenGLEngine.hpp"
#include "plasma/Node.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace plasma {

namespace {

std::string loadTextFile(const char* path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error(std::string("Failed to open shader: ") + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

unsigned compileShader(unsigned type, const char* src) {
    const unsigned s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof log, nullptr, log);
        throw std::runtime_error(log);
    }
    return s;
}

unsigned linkProgram(const char* vsPath, const char* fsPath) {
    const std::string vs = loadTextFile(vsPath);
    const std::string fs = loadTextFile(fsPath);
    const unsigned v = compileShader(GL_VERTEX_SHADER, vs.c_str());
    const unsigned f = compileShader(GL_FRAGMENT_SHADER, fs.c_str());
    const unsigned p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    int ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, sizeof log, nullptr, log);
        throw std::runtime_error(log);
    }
    return p;
}

void uploadMesh(unsigned& vao, unsigned& vbo, unsigned& ebo, const std::vector<float>& verts,
                const std::vector<uint32_t>& indices, int& indexCount, DrawCommand::Layout layout) {
    if (!vao) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)), indices.data(),
                 GL_STATIC_DRAW);
    const GLsizei stride = 9 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    if (layout == DrawCommand::Layout::Ui) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(5 * sizeof(float)));
    } else {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
    }
    glBindVertexArray(0);
    indexCount = static_cast<int>(indices.size());
}

void deleteMesh(unsigned vao, unsigned vbo, unsigned ebo) {
    if (vbo) {
        glDeleteBuffers(1, &vbo);
    }
    if (ebo) {
        glDeleteBuffers(1, &ebo);
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
    }
}

} // namespace

OpenGLTexture::OpenGLTexture(int w, int h, const uint8_t* pixels, bool rgba8) : width_(w), height_(h) {
    glGenTextures(1, &texId_);
    glBindTexture(GL_TEXTURE_2D, texId_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, rgba8 ? GL_RGBA8 : GL_RGB8, w, h, 0, rgba8 ? GL_RGBA : GL_RGB,
                 GL_UNSIGNED_BYTE, pixels);
}

OpenGLTexture::~OpenGLTexture() {
    if (texId_) {
        glDeleteTextures(1, &texId_);
    }
}

void OpenGLTexture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texId_);
}

OpenGLDrawing::OpenGLDrawing() = default;
OpenGLDrawing::~OpenGLDrawing() = default;

void OpenGLDrawing::clear() {
    commands_.clear();
}

void OpenGLDrawing::addCommand(DrawCommand cmd) {
    commands_.push_back(std::move(cmd));
}

void OpenGLDrawing::flush(Engine& engine) {
    auto& gl = static_cast<OpenGLEngine&>(engine);
    gl.initShaders();

    const int uiLocUse = glGetUniformLocation(gl.uiProgram(), "uUseTexture");
    const int uiLocTex = glGetUniformLocation(gl.uiProgram(), "uTex");
    const int modelLocM = glGetUniformLocation(gl.modelProgram(), "uModel");
    const int modelLocV = glGetUniformLocation(gl.modelProgram(), "uView");
    const int modelLocP = glGetUniformLocation(gl.modelProgram(), "uProjection");

    for (const DrawCommand& cmd : commands_) {
        if (cmd.vertices.empty() || cmd.indices.empty()) {
            continue;
        }

        unsigned vao = 0, vbo = 0, ebo = 0;
        int icount = 0;
        uploadMesh(vao, vbo, ebo, cmd.vertices, cmd.indices, icount, cmd.layout);

        if (cmd.layout == DrawCommand::Layout::Ui ||
            cmd.type == DrawCommand::Type::TexturedQuad) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUseProgram(gl.uiProgram());
            if (uiLocTex >= 0) {
                glUniform1i(uiLocTex, 0);
            }
            const bool useTex = cmd.texture != nullptr;
            if (uiLocUse >= 0) {
                glUniform1i(uiLocUse, useTex ? 1 : 0);
            }
            if (useTex) {
                cmd.texture->bind(0);
            }
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, icount, GL_UNSIGNED_INT, nullptr);
        } else {
            glEnable(GL_DEPTH_TEST);
            glUseProgram(gl.modelProgram());
            glUniformMatrix4fv(modelLocM, 1, GL_FALSE, cmd.transform.m);
            glUniformMatrix4fv(modelLocV, 1, GL_FALSE, gl.viewMatrix().m);
            glUniformMatrix4fv(modelLocP, 1, GL_FALSE, gl.projectionMatrix().m);
            glBindVertexArray(vao);
            if (cmd.type == DrawCommand::Type::Lines) {
                glDrawElements(GL_LINES, icount, GL_UNSIGNED_INT, nullptr);
            } else {
                glDrawElements(GL_TRIANGLES, icount, GL_UNSIGNED_INT, nullptr);
            }
        }
        deleteMesh(vao, vbo, ebo);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    clear();
}

OpenGLRenderSurface::OpenGLRenderSurface() = default;

OpenGLRenderSurface::~OpenGLRenderSurface() {
    if (colorTex_) {
        glDeleteTextures(1, &colorTex_);
    }
    if (depthRb_) {
        glDeleteRenderbuffers(1, &depthRb_);
    }
    if (fbo_) {
        glDeleteFramebuffers(1, &fbo_);
    }
}

void OpenGLRenderSurface::begin(Engine& engine, int w, int h) {
    (void)engine;
    width_ = w;
    height_ = h;
    if (!fbo_) {
        glGenFramebuffers(1, &fbo_);
        glGenTextures(1, &colorTex_);
        glGenRenderbuffers(1, &depthRb_);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glBindTexture(GL_TEXTURE_2D, colorTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex_, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRb_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRb_);
}

void OpenGLRenderSurface::end(Engine& engine) {
    (void)engine;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Texture* OpenGLRenderSurface::colorTexture() {
    if (!texture_) {
        std::vector<uint8_t> px(4, 255);
        texture_ = std::make_unique<OpenGLTexture>(1, 1, px.data(), true);
    }
    return texture_.get();
}

OpenGLEngine::OpenGLEngine() = default;

OpenGLEngine::~OpenGLEngine() {
    if (modelVao_) {
        glDeleteVertexArrays(1, reinterpret_cast<unsigned*>(&modelVao_));
    }
    if (modelVbo_) {
        glDeleteBuffers(1, reinterpret_cast<unsigned*>(&modelVbo_));
    }
    if (modelEbo_) {
        glDeleteBuffers(1, reinterpret_cast<unsigned*>(&modelEbo_));
    }
    if (uiProgram_) {
        glDeleteProgram(uiProgram_);
    }
    if (modelProgram_) {
        glDeleteProgram(modelProgram_);
    }
}

RenderSurface* OpenGLEngine::createRenderSurface(bool) {
    return new OpenGLRenderSurface();
}

Texture* OpenGLEngine::createTexture(int w, int h, const uint8_t* rgba, bool rgba8) {
    return new OpenGLTexture(w, h, rgba, rgba8);
}

void OpenGLEngine::setViewMatrix(const Mat4& m) { view_ = m; }
void OpenGLEngine::setProjectionMatrix(const Mat4& m) { proj_ = m; }
void OpenGLEngine::setWorldMatrix(const Mat4& m) { world_ = m; }

void OpenGLEngine::initShaders() {
    if (!uiProgram_) {
        uiProgram_ = linkProgram("shaders/ui.vert", "shaders/ui.frag");
        modelProgram_ = linkProgram("shaders/model.vert", "shaders/model.frag");
    }
}

void OpenGLEngine::beginFrame(Display& display) {
    (void)display;
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.12f, 0.14f, 0.18f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLEngine::endFrame(Display& display) {
    drawing_.flush(*this);
    display.swapBuffers();
}

void OpenGLEngine::renderScene(Node& root, Display& display) {
    (void)display;
    root.draw(*this);
}

void OpenGLEngine::drawVoxelMesh(unsigned vao, int indexCount, const Mat4& model) {
    initShaders();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glUseProgram(modelProgram_);
    const int locM = glGetUniformLocation(modelProgram_, "uModel");
    const int locV = glGetUniformLocation(modelProgram_, "uView");
    const int locP = glGetUniformLocation(modelProgram_, "uProjection");
    glUniformMatrix4fv(locM, 1, GL_FALSE, model.m);
    glUniformMatrix4fv(locV, 1, GL_FALSE, view_.m);
    glUniformMatrix4fv(locP, 1, GL_FALSE, proj_.m);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
}

} // namespace plasma
