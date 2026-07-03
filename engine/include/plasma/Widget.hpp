#pragma once

#include "plasma/Node.hpp"

#include <functional>
#include <string>
#include <vector>

namespace plasma {

class Engine;
class Font;
class Texture;

class Widget : public NamedObject {
public:
    Widget();
    ~Widget() override;

    struct Rect {
        float x{0}, y{0}, w{0}, h{0};
    };

    Rect& bounds() { return bounds_; }
    const Rect& bounds() const { return bounds_; }

    bool visible() const { return visible_; }
    void setVisible(bool v) { visible_ = v; }

    Widget* parent() const { return parent_; }
    void setParent(Widget* p) { parent_ = p; }

    void addChild(Widget* child);
    const std::vector<Widget*>& children() const { return children_; }

    virtual void update(float dt);
    virtual void draw(Engine& engine, int screenW, int screenH);
    virtual void onClick(float x, float y);
    virtual void onDrag(float x, float y);
    virtual bool hitTest(float x, float y) const;
    virtual bool wantsDrag() const { return false; }

    bool dispatchClick(float x, float y);
    virtual void releasePressed();

    Texture* background() const { return background_; }
    void setBackground(Texture* t) { background_ = t; }

protected:
    Rect bounds_{};
    bool visible_{true};
    Widget* parent_{nullptr};
    std::vector<Widget*> children_;
    Texture* background_{nullptr};
};

class Button : public Widget {
public:
    Button();
    void draw(Engine& engine, int screenW, int screenH) override;
    void onClick(float x, float y) override;
    void releasePressed() override;

    std::function<void()> onPressed;
    std::string label;
    std::wstring wlabel;

    bool enabled() const { return enabled_; }
    void setEnabled(bool e) { enabled_ = e; }

    bool pressed() const { return pressed_; }
    void setPressed(bool p) { pressed_ = p; }

    Texture* icon() const { return icon_; }
    void setIcon(Texture* t) { icon_ = t; }

private:
    bool enabled_{true};
    bool pressed_{false};
    Texture* icon_{nullptr};
};

class PopUpButton : public Button {
public:
    PopUpButton();
    void draw(Engine& engine, int screenW, int screenH) override;
    void onClick(float x, float y) override;

    bool open() const { return open_; }
    void setOpen(bool o) { open_ = o; }

    Widget* popup() const { return popup_; }
    void setPopup(Widget* w) { popup_ = w; }

private:
    bool open_{false};
    Widget* popup_{nullptr};
};

class Edit : public Widget {
public:
    Edit();
    void draw(Engine& engine, int screenW, int screenH) override;
    void onClick(float x, float y) override;

    std::wstring text;
    bool focused() const { return focused_; }
    void setFocused(bool f) { focused_ = f; }

    void append(wchar_t c);
    void backspace();

    enum class FilterKind { None, Float, Integer, UnsignedFloat, UnsignedInteger };
    void setFilter(FilterKind kind) { filter_ = kind; }
    FilterKind filter() const { return filter_; }
    bool validateInput() const;

    std::wstring hint;
    std::function<void()> onSubmit;

private:
    bool focused_{false};
    FilterKind filter_{FilterKind::None};
};

class ListWidget : public Widget {
public:
    ListWidget();
    void draw(Engine& engine, int screenW, int screenH) override;
    void onClick(float x, float y) override;
    bool onMouseWheel(float delta);

    std::vector<std::string> items;
    int selected{0};
    int scrollOffset{0};
    float rowHeight{20.f};

    int visibleRowCount() const;
    int maxScrollOffset() const;
    void setScrollOffset(int offset);
    void scrollBy(int delta);

    std::function<void(int)> onSelectionChanged;
};

class ScrollButton : public Widget {
public:
    enum class Direction { Up, Down, Left, Right };

    ScrollButton();
    void draw(Engine& engine, int screenW, int screenH) override;
    void onClick(float x, float y) override;
    void update(float dt) override;
    void releasePressed() override;

    Direction direction{Direction::Down};
    std::string label;
    std::function<void()> onScroll;
    float repeatDelay{0.4f};
    float repeatRate{0.08f};

private:
    float holdTime_{0.f};
    bool holding_{false};
};

class ScrollSlider : public Widget {
public:
    enum class Direction { Vertical, Horizontal };

    ScrollSlider();
    void draw(Engine& engine, int screenW, int screenH) override;
    void onClick(float x, float y) override;
    void onDrag(float x, float y) override;
    void releasePressed() override;
    bool wantsDrag() const override { return true; }

    Direction direction{Direction::Vertical};
    ListWidget* listTarget{nullptr};

    float value{0.f};
    float minValue{0.f};
    float maxValue{1.f};

    void syncFromList();
    std::function<void(float)> onValueChanged;

private:
    void applyAt(float x, float y);
    bool dragging_{false};
};

/// List + vertical ScrollSlider strip on the right (retail plasma layout).
class ScrollBar : public Widget {
public:
    ScrollBar();

    ListWidget* list() { return list_; }
    const ListWidget* list() const { return list_; }

    void setBarWidth(float w) { barWidth_ = w; }
    void layout();
    void syncSlider();

    void draw(Engine& engine, int screenW, int screenH) override;
    void update(float dt) override;
    bool onMouseWheel(float delta);

private:
    ListWidget* list_{nullptr};
    ScrollSlider* slider_{nullptr};
    float barWidth_{18.f};
};

/// Remove Button/ScrollButton widgets that have no caption text.
void pruneTextlessButtons(Widget* root);

template <typename T>
class MemberFunctionConnection {
public:
    void bind(T* target, void (T::*method)()) {
        target_ = target;
        method_ = method;
    }

    void invoke() {
        if (target_ && method_) {
            (target_->*method_)();
        }
    }

    bool bound() const { return target_ && method_; }

private:
    T* target_{nullptr};
    void (T::*method_)(){nullptr};
};

} // namespace plasma
