#include "plasma/Widget.hpp"
#include "plasma/UiDraw.hpp"
#include "plasma/Filters.hpp"
#include "plasma/OpenGLEngine.hpp"
#include "plasma/Font.hpp"

#include <algorithm>
#include <string>

namespace plasma {

namespace {

bool pointInRect(const Widget::Rect& r, float x, float y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

} // namespace

Widget::Widget() = default;
Widget::~Widget() = default;

void Widget::addChild(Widget* child) {
    if (child) {
        child->setParent(this);
        children_.push_back(child);
    }
}

void Widget::update(float) {}

bool Widget::hitTest(float x, float y) const {
    return visible_ && pointInRect(bounds_, x, y);
}

void Widget::onClick(float, float) {}

void Widget::onDrag(float, float) {}

bool Widget::onMouseWheel(float) { return false; }

bool Widget::dispatchClick(float x, float y) {
    if (!visible_) {
        return false;
    }
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (*it && (*it)->dispatchClick(x, y)) {
            return true;
        }
    }
    if (hitTest(x, y)) {
        onClick(x, y);
        return true;
    }
    return false;
}

void Widget::releasePressed() {
    for (Widget* c : children_) {
        if (c) {
            c->releasePressed();
        }
    }
}

void Widget::draw(Engine& engine, int screenW, int screenH) {
    if (!visible_) {
        return;
    }
    if (background_) {
        drawTexturedQuad(engine, bounds_, screenW, screenH, background_);
    } else {
        drawSolidQuad(engine, bounds_, screenW, screenH, {0.2f, 0.22f, 0.28f, 0.85f});
    }
    for (Widget* c : children_) {
        if (c) {
            c->draw(engine, screenW, screenH);
        }
    }
}

namespace {

std::wstring buttonCaption(const Button& btn) {
    if (!btn.wlabel.empty()) {
        return btn.wlabel;
    }
    return std::wstring(btn.label.begin(), btn.label.end());
}

} // namespace

Button::Button() = default;

void Button::draw(Engine& engine, int screenW, int screenH) {
    if (!visible_) {
        return;
    }
    const Vec4 base = !enabled_   ? Vec4{0.2f, 0.2f, 0.22f, 0.7f}
                      : pressed_  ? Vec4{0.22f, 0.28f, 0.4f, 1.f}
                      : Vec4{0.28f, 0.35f, 0.5f, 0.95f};
    drawSolidQuad(engine, bounds_, screenW, screenH, base);
    if (icon_) {
        drawTexturedQuad(engine, bounds_, screenW, screenH, icon_);
    }
    const std::wstring caption = buttonCaption(*this);
    if (!caption.empty()) {
        uiFont().drawText(engine, caption, bounds_.x + 8.f, bounds_.y + 6.f, {0.95f, 0.97f, 1.f, 1.f},
                          screenW, screenH);
    }
    for (Widget* c : children_) {
        if (c) {
            c->draw(engine, screenW, screenH);
        }
    }
}

void Button::onClick(float, float) {
    if (!enabled_) {
        return;
    }
    pressed_ = true;
    if (onPressed) {
        onPressed();
    }
}

void Button::releasePressed() {
    pressed_ = false;
    Widget::releasePressed();
}

PopUpButton::PopUpButton() = default;

void PopUpButton::draw(Engine& engine, int screenW, int screenH) {
    Button::draw(engine, screenW, screenH);
    if (open_ && popup_ && popup_->visible()) {
        popup_->draw(engine, screenW, screenH);
    }
}

void PopUpButton::onClick(float x, float y) {
    Button::onClick(x, y);
    open_ = !open_;
    if (popup_) {
        popup_->setVisible(open_);
    }
}

Edit::Edit() = default;

bool Edit::validateInput() const {
    switch (filter_) {
    case FilterKind::None:
        return true;
    case FilterKind::Float: {
        float v = 0.f;
        return FloatFilter::accept(text, v);
    }
    case FilterKind::Integer: {
        int v = 0;
        return IntegerFilter::accept(text, v);
    }
    case FilterKind::UnsignedFloat: {
        float v = 0.f;
        return UnsignedFloatFilter::accept(text, v);
    }
    case FilterKind::UnsignedInteger: {
        unsigned v = 0;
        return UnsignedIntegerFilter::accept(text, v);
    }
    }
    return true;
}

void Edit::append(wchar_t c) {
    text.push_back(c);
}

void Edit::backspace() {
    if (!text.empty()) {
        text.pop_back();
    }
}

void Edit::draw(Engine& engine, int screenW, int screenH) {
    if (!visible_) {
        return;
    }
    const Vec4 bg = focused_ ? Vec4{0.18f, 0.2f, 0.28f, 1.f} : Vec4{0.15f, 0.15f, 0.18f, 1.f};
    drawSolidQuad(engine, bounds_, screenW, screenH, bg);
    if (!validateInput()) {
        drawSolidQuad(engine, bounds_, screenW, screenH, {0.5f, 0.15f, 0.15f, 0.35f});
    }
    const std::wstring& shown = !text.empty() ? text : hint;
    if (!shown.empty()) {
        const Vec4 color = text.empty() ? Vec4{0.55f, 0.58f, 0.65f, 1.f}
                                        : Vec4{0.92f, 0.94f, 0.98f, 1.f};
        uiFont().drawText(engine, shown, bounds_.x + 6.f, bounds_.y + 4.f, color, screenW, screenH);
    }
}

void Edit::onClick(float x, float y) {
    setFocused(hitTest(x, y));
}

ListWidget::ListWidget() = default;

int ListWidget::visibleRowCount() const {
    return static_cast<int>((bounds_.h - 8.f) / rowHeight);
}

int ListWidget::maxScrollOffset() const {
    return std::max(0, static_cast<int>(items.size()) - visibleRowCount());
}

void ListWidget::setScrollOffset(int offset) {
    scrollOffset = std::max(0, std::min(maxScrollOffset(), offset));
}

void ListWidget::scrollBy(int delta) {
    setScrollOffset(scrollOffset + delta);
}

bool ListWidget::onMouseWheel(float delta) {
    if (delta == 0.f) {
        return false;
    }
    scrollBy(delta > 0.f ? -3 : 3);
    return true;
}

void ListWidget::draw(Engine& engine, int screenW, int screenH) {
    if (!visible_) {
        return;
    }
    drawSolidQuad(engine, bounds_, screenW, screenH, {0.18f, 0.2f, 0.24f, 0.95f});
    const int visibleRows = visibleRowCount();
    float y = bounds_.y + 4.f;
    for (int i = 0; i < visibleRows; ++i) {
        const int idx = scrollOffset + i;
        if (idx < 0 || idx >= static_cast<int>(items.size())) {
            break;
        }
        Widget::Rect row{bounds_.x + 4.f, y, bounds_.w - 8.f, rowHeight - 2.f};
        const Vec4 col = idx == selected ? Vec4{0.35f, 0.45f, 0.65f, 1.f}
                                         : Vec4{0.25f, 0.28f, 0.32f, 0.9f};
        drawSolidQuad(engine, row, screenW, screenH, col);
        const std::string& item = items[static_cast<size_t>(idx)];
        const size_t maxChars = static_cast<size_t>((row.w - 8.f) / 7.f);
        const std::string clipped = item.substr(0, maxChars);
        const std::wstring wtext(clipped.begin(), clipped.end());
        uiFont().drawText(engine, wtext, row.x + 4.f, row.y + 3.f, {0.92f, 0.94f, 0.98f, 1.f}, screenW,
                          screenH);
        y += rowHeight;
    }
}

void ListWidget::onClick(float x, float y) {
    if (!hitTest(x, y) || items.empty()) {
        return;
    }
    const int row = static_cast<int>((y - bounds_.y - 4.f) / rowHeight) + scrollOffset;
    if (row >= 0 && row < static_cast<int>(items.size())) {
        selected = row;
        if (onSelectionChanged) {
            onSelectionChanged(row);
        }
    }
}

ScrollButton::ScrollButton() = default;

void ScrollButton::draw(Engine& engine, int screenW, int screenH) {
    if (!visible_) {
        return;
    }
    const Vec4 col = holding_ ? Vec4{0.38f, 0.38f, 0.44f, 1.f} : Vec4{0.3f, 0.3f, 0.35f, 1.f};
    drawSolidQuad(engine, bounds_, screenW, screenH, col);
    const std::string caption =
        !label.empty()
            ? label
            : (direction == Direction::Up     ? "^"
               : direction == Direction::Down  ? "v"
               : direction == Direction::Left  ? "<"
                                               : ">");
    const std::wstring wtext(caption.begin(), caption.end());
    uiFont().drawText(engine, wtext, bounds_.x + bounds_.w * 0.35f, bounds_.y + 4.f, {0.95f, 0.97f, 1.f, 1.f},
                      screenW, screenH);
}

void ScrollButton::onClick(float x, float y) {
    if (!hitTest(x, y)) {
        return;
    }
    holding_ = true;
    holdTime_ = 0.f;
    if (onScroll) {
        onScroll();
    }
}

void ScrollButton::update(float dt) {
    if (!holding_) {
        return;
    }
    holdTime_ += dt;
    if (holdTime_ >= repeatDelay && onScroll) {
        const float repeats = (holdTime_ - repeatDelay) / repeatRate;
        static float accumulated = 0.f;
        accumulated += dt;
        if (accumulated >= repeatRate) {
            onScroll();
            accumulated = 0.f;
        }
        (void)repeats;
    }
}

void ScrollButton::releasePressed() {
    holding_ = false;
    holdTime_ = 0.f;
    Widget::releasePressed();
}

ScrollSlider::ScrollSlider() = default;

void ScrollSlider::syncFromList() {
    if (!listTarget) {
        return;
    }
    minValue = 0.f;
    maxValue = static_cast<float>(listTarget->maxScrollOffset());
    value = static_cast<float>(listTarget->scrollOffset);
}

void ScrollSlider::draw(Engine& engine, int screenW, int screenH) {
    if (!visible_) {
        return;
    }
    syncFromList();
    drawSolidQuad(engine, bounds_, screenW, screenH, {0.2f, 0.2f, 0.24f, 1.f});

    const float span = maxValue - minValue;
    const float t = span > 0.f ? (value - minValue) / span : 0.f;

    if (direction == Direction::Vertical) {
        const int total = listTarget ? static_cast<int>(listTarget->items.size()) : 0;
        const int visible = listTarget ? listTarget->visibleRowCount() : 1;
        const float trackH = bounds_.h > 0.f ? bounds_.h : 1.f;
        float knobH = trackH;
        if (total > visible) {
            knobH = std::max(20.f, trackH * (static_cast<float>(visible) / static_cast<float>(total)));
        }
        const float travel = std::max(1.f, trackH - knobH);
        Widget::Rect knob{bounds_.x + 2.f, bounds_.y + t * travel, bounds_.w - 4.f, knobH};
        drawSolidQuad(engine, knob, screenW, screenH, {0.45f, 0.47f, 0.52f, 1.f});
    } else {
        const float knobW = std::max(12.f, bounds_.w * 0.2f);
        Widget::Rect knob{bounds_.x + t * (bounds_.w - knobW), bounds_.y + 2.f, knobW, bounds_.h - 4.f};
        drawSolidQuad(engine, knob, screenW, screenH, {0.45f, 0.47f, 0.52f, 1.f});
    }
}

void ScrollSlider::applyAt(float x, float y) {
    syncFromList();
    const float span = maxValue - minValue;

    if (direction == Direction::Vertical) {
        const int total = listTarget ? static_cast<int>(listTarget->items.size()) : 0;
        const int visible = listTarget ? listTarget->visibleRowCount() : 1;
        const float trackH = bounds_.h > 0.f ? bounds_.h : 1.f;
        float knobH = trackH;
        if (total > visible) {
            knobH = std::max(20.f, trackH * (static_cast<float>(visible) / static_cast<float>(total)));
        }
        const float travel = std::max(1.f, trackH - knobH);
        float t = (y - bounds_.y - knobH * 0.5f) / travel;
        t = std::max(0.f, std::min(1.f, t));
        value = minValue + t * span;
    } else {
        float t = (x - bounds_.x) / (bounds_.w > 0.f ? bounds_.w : 1.f);
        t = std::max(0.f, std::min(1.f, t));
        value = minValue + t * span;
    }

    if (listTarget) {
        listTarget->setScrollOffset(static_cast<int>(value + 0.5f));
        value = static_cast<float>(listTarget->scrollOffset);
    }
    if (onValueChanged) {
        onValueChanged(value);
    }
}

void ScrollSlider::onClick(float x, float y) {
    if (!hitTest(x, y)) {
        return;
    }
    dragging_ = true;
    applyAt(x, y);
}

void ScrollSlider::onDrag(float x, float y) {
    if (!dragging_) {
        return;
    }
    applyAt(x, y);
}

void ScrollSlider::releasePressed() {
    dragging_ = false;
    Widget::releasePressed();
}

ScrollBar::ScrollBar() {
    list_ = new ListWidget();
    list_->setName("model_list");
    addChild(list_);

    slider_ = new ScrollSlider();
    slider_->setName("scroll_slider");
    slider_->direction = ScrollSlider::Direction::Vertical;
    slider_->listTarget = list_;
    addChild(slider_);

    slider_->onValueChanged = [this](float) { syncSlider(); };
}

void ScrollBar::syncSlider() {
    if (slider_) {
        slider_->syncFromList();
    }
}

void ScrollBar::layout() {
    if (!list_ || !slider_) {
        return;
    }
    const float x = bounds_.x;
    const float y = bounds_.y;
    const float listW = std::max(40.f, bounds_.w - barWidth_);
    list_->bounds() = {x, y, listW, bounds_.h};

    const float barX = x + listW;
    slider_->bounds() = {barX, y, barWidth_, bounds_.h};
    syncSlider();
}

void ScrollBar::draw(Engine& engine, int screenW, int screenH) {
    if (!visible_) {
        return;
    }
    layout();
    for (Widget* c : children_) {
        if (c) {
            c->draw(engine, screenW, screenH);
        }
    }
}

void ScrollBar::update(float dt) {
    layout();
    for (Widget* c : children_) {
        if (c) {
            c->update(dt);
        }
    }
}

bool ScrollBar::onMouseWheel(float delta) {
    if (list_ && list_->onMouseWheel(delta)) {
        syncSlider();
        return true;
    }
    return false;
}

namespace {

bool isTextlessButton(const Widget* widget) {
    if (!widget) {
        return false;
    }
    if (const auto* btn = dynamic_cast<const Button*>(widget)) {
        return btn->label.empty() && btn->wlabel.empty();
    }
    if (dynamic_cast<const ScrollButton*>(widget)) {
        return true;
    }
    return false;
}

} // namespace

void pruneTextlessButtons(Widget* root) {
    if (!root) {
        return;
    }
    std::vector<Widget*>& kids = const_cast<std::vector<Widget*>&>(root->children());
    for (auto it = kids.begin(); it != kids.end();) {
        Widget* child = *it;
        if (!child) {
            it = kids.erase(it);
            continue;
        }
        if (isTextlessButton(child)) {
            delete child;
            it = kids.erase(it);
            continue;
        }
        pruneTextlessButtons(child);
        ++it;
    }
}

} // namespace plasma
