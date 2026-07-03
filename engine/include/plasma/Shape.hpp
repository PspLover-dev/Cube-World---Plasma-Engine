#pragma once

#include "plasma/Node.hpp"
#include "plasma/Engine.hpp"

#include <string>
#include <vector>

namespace plasma {

class Font;

/// Interleaved scene vertex: position, normal, RGB color (9 floats).
struct SceneVertex {
    float px, py, pz;
    float nx, ny, nz;
    float r, g, b;
};

class Shape : public Node {
public:
    explicit Shape(Transformation* transform = nullptr);
    ~Shape() override;

    virtual void buildGeometry() {}
    virtual void drawShape(Engine& engine);

    Vec4& color() { return color_; }
    const Vec4& color() const { return color_; }

    Vec2& size() { return size_; }
    const Vec2& size() const { return size_; }

    Vec2& offset() { return offset_; }
    const Vec2& offset() const { return offset_; }

    bool visible() const { return visible_; }
    void setVisible(bool v) { visible_ = v; }

    Shape* asShape() override { return this; }

protected:
    void appendSceneQuad(std::vector<float>& out, const SceneVertex& a, const SceneVertex& b,
                         const SceneVertex& c, const SceneVertex& d) const;
    void appendSceneTriangle(std::vector<float>& out, const SceneVertex& a, const SceneVertex& b,
                             const SceneVertex& c) const;
    SceneVertex makeVertex(const Vec3& p, const Vec3& n) const;

    Vec4 color_{1.f, 1.f, 1.f, 1.f};
    Vec2 size_{100.f, 100.f};
    Vec2 offset_{};
    bool visible_{true};
};

class GenericShape : public Shape {
public:
    using Shape::Shape;

    void setVertices(std::vector<float> verts, std::vector<uint32_t> indices);
    void setPrimitive(DrawCommand::Type primitive) { primitive_ = primitive; }
    void buildGeometry() override;
    void drawShape(Engine& engine) override;

private:
    std::vector<float> verts_;
    std::vector<uint32_t> indices_;
    DrawCommand::Type primitive_{DrawCommand::Type::Triangles};
};

class MeshShape : public Shape {
public:
    using Shape::Shape;

    void setMesh(std::vector<float> interleaved, std::vector<uint32_t> indices);
    void buildGeometry() override;
    void drawShape(Engine& engine) override;

protected:
    std::vector<float> interleaved_;
    std::vector<uint32_t> indices_;
};

class StaticMeshShape : public MeshShape {
public:
    using MeshShape::MeshShape;
    void buildGeometry() override;
};

class SmoothMeshShape : public MeshShape {
public:
    using MeshShape::MeshShape;

    void tessellatePolygon(const std::vector<Vec3>& contour);
    void setSmoothNormals(bool v) { smoothNormals_ = v; }

private:
    bool smoothNormals_{true};
};

class TextShape : public Shape {
public:
    TextShape(Font* font = nullptr, Transformation* transform = nullptr);
    void setText(std::wstring text);
    void setFont(Font* font) { font_ = font; }
    void drawShape(Engine& engine) override;

private:
    Font* font_{nullptr};
    std::wstring text_;
};

class CurveShape : public Shape {
public:
    using Shape::Shape;

    void setControlPoints(std::vector<Vec3> points);
    void setClosed(bool closed) { closed_ = closed; }
    void setSegmentsPerSpan(int n) { segmentsPerSpan_ = n > 1 ? n : 2; }
    void buildGeometry() override;
    void drawShape(Engine& engine) override;

    const std::vector<Vec3>& controlPoints() const { return controlPoints_; }
    const std::vector<Vec3>& tessellatedPoints() const { return tessellated_; }

private:
    std::vector<Vec3> controlPoints_;
    std::vector<Vec3> tessellated_;
    std::vector<float> lineVerts_;
    std::vector<uint32_t> lineIndices_;
    bool closed_{false};
    int segmentsPerSpan_{12};
};

class Movie : public NamedObject {
public:
    Movie();
    explicit Movie(Texture* frame);

    void setFrame(Texture* t) { frames_.clear(); if (t) frames_.push_back(t); }
    void setFrames(std::vector<Texture*> frames);
    void setFps(float fps) { fps_ = fps > 0.f ? fps : 1.f; }

    void update(float dt);
    void draw(Engine& engine, int screenW, int screenH);

    Texture* currentFrame() const;
    int currentFrameIndex() const { return frameIndex_; }

private:
    std::vector<Texture*> frames_;
    float fps_{12.f};
    float frameTime_{0.f};
    int frameIndex_{0};
};

} // namespace plasma
