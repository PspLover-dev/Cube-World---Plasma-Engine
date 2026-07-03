#include "plasma/UiDraw.hpp"

namespace plasma {

namespace {

void screenRectToNdc(const Widget::Rect& r, int screenW, int screenH, float& x0, float& y0, float& x1,
                     float& y1) {
    x0 = (r.x / screenW) * 2.f - 1.f;
    y0 = 1.f - ((r.y + r.h) / screenH) * 2.f;
    x1 = ((r.x + r.w) / screenW) * 2.f - 1.f;
    y1 = 1.f - (r.y / screenH) * 2.f;
}

void pushQuad(DrawCommand& cmd, float x0, float y0, float x1, float y1, float u0, float v0, float u1,
              float v1, const Vec4& color) {
    cmd.vertices = {
        x0, y0, 0.f, u0, v0, color[0], color[1], color[2], color[3],
        x1, y0, 0.f, u1, v0, color[0], color[1], color[2], color[3],
        x1, y1, 0.f, u1, v1, color[0], color[1], color[2], color[3],
        x0, y1, 0.f, u0, v1, color[0], color[1], color[2], color[3],
    };
    cmd.indices = {0, 1, 2, 0, 2, 3};
}

} // namespace

void drawSolidQuad(Engine& engine, const Widget::Rect& r, int screenW, int screenH, const Vec4& color) {
    float x0 = 0.f, y0 = 0.f, x1 = 0.f, y1 = 0.f;
    screenRectToNdc(r, screenW, screenH, x0, y0, x1, y1);
    DrawCommand cmd;
    cmd.type = DrawCommand::Type::TexturedQuad;
    cmd.layout = DrawCommand::Layout::Ui;
    cmd.texture = nullptr;
    pushQuad(cmd, x0, y0, x1, y1, 0.f, 0.f, 0.f, 0.f, color);
    engine.drawing().addCommand(std::move(cmd));
}

void drawTexturedQuad(Engine& engine, const Widget::Rect& r, int screenW, int screenH, Texture* texture,
                      const Vec4& tint, float u0, float v0, float u1, float v1) {
    if (!texture) {
        return;
    }
    float x0 = 0.f, y0 = 0.f, x1 = 0.f, y1 = 0.f;
    screenRectToNdc(r, screenW, screenH, x0, y0, x1, y1);
    DrawCommand cmd;
    cmd.type = DrawCommand::Type::TexturedQuad;
    cmd.layout = DrawCommand::Layout::Ui;
    cmd.texture = texture;
    pushQuad(cmd, x0, y0, x1, y1, u0, v0, u1, v1, tint);
    engine.drawing().addCommand(std::move(cmd));
}

} // namespace plasma
