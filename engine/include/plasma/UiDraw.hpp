#pragma once

#include "plasma/Engine.hpp"
#include "plasma/Math.hpp"
#include "plasma/Widget.hpp"

namespace plasma {

void drawSolidQuad(Engine& engine, const Widget::Rect& r, int screenW, int screenH, const Vec4& color);
void drawTexturedQuad(Engine& engine, const Widget::Rect& r, int screenW, int screenH, Texture* texture,
                      const Vec4& tint = {1.f, 1.f, 1.f, 1.f}, float u0 = 0.f, float v0 = 0.f, float u1 = 1.f,
                      float v1 = 1.f);

} // namespace plasma
