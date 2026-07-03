#pragma once

#include "plasma/Engine.hpp"

#include <memory>
#include <vector>

namespace plasma {

class OpenGLTexture;

class OpenGLDrawing : public Drawing {
public:
    OpenGLDrawing();
    ~OpenGLDrawing() override;

    void clear() override;
    void addCommand(DrawCommand cmd) override;
    void flush(Engine& engine) override;

private:
    std::vector<DrawCommand> commands_;
};

class OpenGLRenderSurface : public RenderSurface {
public:
    OpenGLRenderSurface();
    ~OpenGLRenderSurface() override;

    void begin(Engine& engine, int w, int h) override;
    void end(Engine& engine) override;
    Texture* colorTexture() override;

private:
    unsigned fbo_{0};
    unsigned colorTex_{0};
    unsigned depthRb_{0};
    int width_{0};
    int height_{0};
    std::unique_ptr<OpenGLTexture> texture_;
};

class OpenGLTexture : public Texture {
public:
    OpenGLTexture(int w, int h, const uint8_t* pixels, bool rgba8);
    ~OpenGLTexture() override;

    int width() const override { return width_; }
    int height() const override { return height_; }
    void bind(int unit = 0) const override;

    unsigned glId() const { return texId_; }

private:
    int width_{0};
    int height_{0};
    unsigned texId_{0};
};

class OpenGLEngine : public Engine {
public:
    OpenGLEngine();
    ~OpenGLEngine() override;

    Drawing& drawing() override { return drawing_; }
    RenderSurface* createRenderSurface(bool depth = true) override;
    Texture* createTexture(int w, int h, const uint8_t* rgba, bool rgba8 = true) override;

    void setViewMatrix(const Mat4& m) override;
    void setProjectionMatrix(const Mat4& m) override;
    void setWorldMatrix(const Mat4& m) override;

    void beginFrame(Display& display) override;
    void endFrame(Display& display) override;
    void renderScene(Node& root, Display& display) override;

    const Mat4& viewMatrix() const { return view_; }
    const Mat4& projectionMatrix() const { return proj_; }
    const Mat4& worldMatrix() const { return world_; }

    void initShaders();
    void drawVoxelMesh(unsigned vao, int indexCount, const Mat4& model);
    unsigned uiProgram() const { return uiProgram_; }
    unsigned modelProgram() const { return modelProgram_; }

private:
    OpenGLDrawing drawing_;
    Mat4 view_{Mat4::identity()};
    Mat4 proj_{Mat4::identity()};
    Mat4 world_{Mat4::identity()};
    unsigned uiProgram_{0};
    unsigned modelProgram_{0};
    int modelVao_{0};
    int modelVbo_{0};
    int modelEbo_{0};
    int modelIndexCount_{0};
};

} // namespace plasma
