#pragma once

#include "plasma/Math.hpp"
#include "plasma/NamedObject.hpp"
#include "plasma/Object.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace plasma {

class Node;
class Display;
class Engine;

struct DrawCommand {
    enum class Type : uint8_t { Triangles, Lines, TexturedQuad };
    enum class Layout : uint8_t { Scene, Ui };

    Type type{Type::Triangles};
    Layout layout{Layout::Scene};
    Texture* texture{nullptr};
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    Mat4 transform{Mat4::identity()};
    Vec4 color{1.f, 1.f, 1.f, 1.f};
};

class Drawing : public Object {
public:
    ~Drawing() override = default;
    virtual void clear() = 0;
    virtual void addCommand(DrawCommand cmd) = 0;
    virtual void flush(Engine& engine) = 0;
};

class Texture : public NamedObject {
public:
    ~Texture() override = default;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void bind(int unit = 0) const = 0;
};

class RenderSurface : public Object {
public:
    ~RenderSurface() override = default;
    virtual void begin(Engine& engine, int w, int h) = 0;
    virtual void end(Engine& engine) = 0;
    virtual Texture* colorTexture() = 0;
};

class Engine : public Object {
public:
    ~Engine() override = default;

    virtual Drawing& drawing() = 0;
    virtual RenderSurface* createRenderSurface(bool depth = true) = 0;
    virtual Texture* createTexture(int w, int h, const uint8_t* rgba, bool rgba8 = true) = 0;

    virtual void setViewMatrix(const Mat4& m) = 0;
    virtual void setProjectionMatrix(const Mat4& m) = 0;
    virtual void setWorldMatrix(const Mat4& m) = 0;

    virtual void beginFrame(Display& display) = 0;
    virtual void endFrame(Display& display) = 0;
    virtual void renderScene(Node& root, Display& display) = 0;

    ObjectManager& objects() { return objects_; }

protected:
    ObjectManager objects_;
};

class Display : public Object {
public:
    Display();
    ~Display() override;

    void attachWindow(void* glfwWindow);
    void* window() const { return window_; }

    int width() const { return width_; }
    int height() const { return height_; }
    void setSize(int w, int h);

    Engine* engine() const { return engine_.get(); }
    void setEngine(std::unique_ptr<Engine> engine);

    void pollEvents();
    bool shouldClose() const;
    void swapBuffers();

private:
    void* window_{nullptr};
    int width_{0};
    int height_{0};
    std::unique_ptr<Engine> engine_;
};

} // namespace plasma
