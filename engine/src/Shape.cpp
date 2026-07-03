#include "plasma/Shape.hpp"
#include "plasma/Engine.hpp"
#include "plasma/Font.hpp"
#include "plasma/Tessellate.hpp"
#include "plasma/UiDraw.hpp"
#include "plasma/Widget.hpp"

#include <cmath>

namespace plasma {

namespace {

void pushSceneVertex(std::vector<float>& out, const SceneVertex& v) {
    out.push_back(v.px);
    out.push_back(v.py);
    out.push_back(v.pz);
    out.push_back(v.nx);
    out.push_back(v.ny);
    out.push_back(v.nz);
    out.push_back(v.r);
    out.push_back(v.g);
    out.push_back(v.b);
}

} // namespace

Shape::Shape(Transformation* transform) : Node(transform) {}
Shape::~Shape() = default;

SceneVertex Shape::makeVertex(const Vec3& p, const Vec3& n) const {
    SceneVertex v{};
    v.px = p[0];
    v.py = p[1];
    v.pz = p[2];
    v.nx = n[0];
    v.ny = n[1];
    v.nz = n[2];
    v.r = color_[0];
    v.g = color_[1];
    v.b = color_[2];
    return v;
}

void Shape::appendSceneTriangle(std::vector<float>& out, const SceneVertex& a, const SceneVertex& b,
                                const SceneVertex& c) const {
    pushSceneVertex(out, a);
    pushSceneVertex(out, b);
    pushSceneVertex(out, c);
}

void Shape::appendSceneQuad(std::vector<float>& out, const SceneVertex& a, const SceneVertex& b,
                            const SceneVertex& c, const SceneVertex& d) const {
    appendSceneTriangle(out, a, b, c);
    appendSceneTriangle(out, a, c, d);
}

void Shape::drawShape(Engine&) {
    if (!visible_) {
        return;
    }
    buildGeometry();
}

void GenericShape::setVertices(std::vector<float> verts, std::vector<uint32_t> indices) {
    verts_ = std::move(verts);
    indices_ = std::move(indices);
}

void GenericShape::buildGeometry() {}

void GenericShape::drawShape(Engine& engine) {
    Shape::drawShape(engine);
    if (!visible_ || verts_.empty() || indices_.empty()) {
        return;
    }
    DrawCommand cmd;
    cmd.type = primitive_;
    cmd.layout = DrawCommand::Layout::Scene;
    cmd.vertices = verts_;
    cmd.indices = indices_;
    cmd.transform = transform() ? transform()->worldMatrix() : Mat4::identity();
    cmd.color = color_;
    engine.drawing().addCommand(std::move(cmd));
}

void MeshShape::setMesh(std::vector<float> interleaved, std::vector<uint32_t> indices) {
    interleaved_ = std::move(interleaved);
    indices_ = std::move(indices);
}

void MeshShape::buildGeometry() {}

void MeshShape::drawShape(Engine& engine) {
    Shape::drawShape(engine);
    if (!visible_ || interleaved_.empty() || indices_.empty()) {
        return;
    }
    DrawCommand cmd;
    cmd.type = DrawCommand::Type::Triangles;
    cmd.layout = DrawCommand::Layout::Scene;
    cmd.vertices = interleaved_;
    cmd.indices = indices_;
    cmd.transform = transform() ? transform()->worldMatrix() : Mat4::identity();
    cmd.color = color_;
    engine.drawing().addCommand(std::move(cmd));
}

void StaticMeshShape::buildGeometry() {
    MeshShape::buildGeometry();
}

void SmoothMeshShape::tessellatePolygon(const std::vector<Vec3>& contour) {
  if (contour.size() < 3) {
        return;
    }
    std::vector<Vec2> flat;
    flat.reserve(contour.size());
    for (const Vec3& p : contour) {
        flat.push_back({p[0], p[1]});
    }
    const std::vector<uint32_t> tris = Tessellate::triangulatePolygon2D(flat);
    const Vec3 normal = {0.f, 0.f, 1.f};
    std::vector<float> verts;
    verts.reserve(contour.size() * 9);
    for (const Vec3& p : contour) {
        pushSceneVertex(verts, makeVertex(p, normal));
    }
    setMesh(std::move(verts), tris);
}

TextShape::TextShape(Font* font, Transformation* transform) : Shape(transform), font_(font) {}

void TextShape::setText(std::wstring text) {
    text_ = std::move(text);
}

void TextShape::drawShape(Engine& engine) {
    Shape::drawShape(engine);
    if (!visible_ || !font_ || text_.empty()) {
        return;
    }
    font_->drawText(engine, text_, offset_[0], offset_[1], color_, 1280, 720);
}

void CurveShape::setControlPoints(std::vector<Vec3> points) {
    controlPoints_ = std::move(points);
    buildGeometry();
}

void CurveShape::buildGeometry() {
    tessellated_ = Tessellate::catmullRomSpline(controlPoints_, segmentsPerSpan_, closed_);
    lineVerts_.clear();
    lineIndices_.clear();
    if (tessellated_.size() < 2) {
        return;
    }
    const Vec3 normal = {0.f, 0.f, 1.f};
    for (const Vec3& p : tessellated_) {
        pushSceneVertex(lineVerts_, makeVertex(p, normal));
    }
    for (size_t i = 1; i < tessellated_.size(); ++i) {
        lineIndices_.push_back(static_cast<uint32_t>(i - 1));
        lineIndices_.push_back(static_cast<uint32_t>(i));
    }
    if (closed_ && tessellated_.size() > 2) {
        lineIndices_.push_back(static_cast<uint32_t>(tessellated_.size() - 1));
        lineIndices_.push_back(0);
    }
}

void CurveShape::drawShape(Engine& engine) {
    Shape::drawShape(engine);
    if (!visible_ || lineVerts_.empty() || lineIndices_.empty()) {
        return;
    }
    DrawCommand cmd;
    cmd.type = DrawCommand::Type::Lines;
    cmd.layout = DrawCommand::Layout::Scene;
    cmd.vertices = lineVerts_;
    cmd.indices = lineIndices_;
    cmd.transform = transform() ? transform()->worldMatrix() : Mat4::identity();
    cmd.color = color_;
    engine.drawing().addCommand(std::move(cmd));
}

Movie::Movie() = default;

Movie::Movie(Texture* frame) {
    if (frame) {
        frames_.push_back(frame);
    }
}

void Movie::setFrames(std::vector<Texture*> frames) {
    frames_ = std::move(frames);
    frameIndex_ = 0;
    frameTime_ = 0.f;
}

void Movie::update(float dt) {
    if (frames_.size() <= 1) {
        return;
    }
    frameTime_ += dt;
    const float frameDur = 1.f / fps_;
    while (frameTime_ >= frameDur) {
        frameTime_ -= frameDur;
        frameIndex_ = (frameIndex_ + 1) % static_cast<int>(frames_.size());
    }
}

Texture* Movie::currentFrame() const {
    if (frames_.empty()) {
        return nullptr;
    }
    return frames_[static_cast<size_t>(frameIndex_)];
}

void Movie::draw(Engine& engine, int screenW, int screenH) {
    Texture* frame = currentFrame();
    if (!frame) {
        return;
    }
    Widget::Rect r{0.f, 0.f, static_cast<float>(screenW), static_cast<float>(screenH)};
    drawTexturedQuad(engine, r, screenW, screenH, frame);
}

} // namespace plasma
