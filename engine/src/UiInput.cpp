#include "plasma/UiInput.hpp"
#include "plasma/Keyable.hpp"
#include "plasma/Widget.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>

namespace plasma {

namespace {

UiInput* g_uiInputForScroll = nullptr;

void glfwScrollCallback(GLFWwindow*, double, double yoff) {
    if (g_uiInputForScroll) {
        g_uiInputForScroll->addScroll(static_cast<float>(yoff));
    }
}

Widget* widgetAt(Widget* root, float x, float y) {
    if (!root || !root->visible()) {
        return nullptr;
    }
    for (auto it = root->children().rbegin(); it != root->children().rend(); ++it) {
        if (Widget* hit = widgetAt(*it, x, y)) {
            return hit;
        }
    }
    if (root->hitTest(x, y)) {
        return root;
    }
    return nullptr;
}

Widget* dragTargetAt(Widget* root, float x, float y) {
    Widget* hit = widgetAt(root, x, y);
    if (hit && hit->wantsDrag()) {
        return hit;
    }
    return nullptr;
}

bool dispatchWheel(Widget* root, float x, float y, float delta) {
    Widget* hit = widgetAt(root, x, y);
    for (Widget* node = hit; node; node = node->parent()) {
        if (auto* bar = dynamic_cast<ScrollBar*>(node)) {
            return bar->onMouseWheel(delta);
        }
        if (auto* list = dynamic_cast<ListWidget*>(node)) {
            return list->onMouseWheel(delta);
        }
    }
    return false;
}

Edit* findFocusedEdit(Widget* root) {
    if (!root) {
        return nullptr;
    }
    if (auto* edit = dynamic_cast<Edit*>(root)) {
        if (edit->focused()) {
            return edit;
        }
    }
    for (Widget* child : root->children()) {
        if (Edit* edit = findFocusedEdit(child)) {
            return edit;
        }
    }
    return nullptr;
}

void clearEditFocus(Widget* root) {
    if (!root) {
        return;
    }
    if (auto* edit = dynamic_cast<Edit*>(root)) {
        edit->setFocused(false);
    }
    for (Widget* child : root->children()) {
        clearEditFocus(child);
    }
}

} // namespace

void glfwCharCallback(GLFWwindow*, unsigned int codepoint) {
    if (g_uiInputForScroll) {
        g_uiInputForScroll->onChar(codepoint);
    }
}

void UiInput::attach(GLFWwindow* window) {
    window_ = window;
    g_uiInputForScroll = this;
    glfwSetScrollCallback(window_, glfwScrollCallback);
    glfwSetCharCallback(window_, glfwCharCallback);
}

void UiInput::onChar(unsigned int codepoint) {
    if (codepoint < 32 || codepoint > 0xFFFF) {
        return;
    }
    pendingChar_ = static_cast<wchar_t>(codepoint);
}

void UiInput::addScroll(float delta) {
    scrollDelta_ += delta;
}

void UiInput::beginFrame() {
    if (!window_) {
        return;
    }
    double mx = 0.0;
    double my = 0.0;
    glfwGetCursorPos(window_, &mx, &my);
    mouseX_ = static_cast<float>(mx);
    mouseY_ = static_cast<float>(my);
}

bool UiInput::keyDown(int glfwKey) const {
    if (!window_ || glfwKey < 0 || glfwKey >= static_cast<int>(sizeof(keyWasDown_) / sizeof(keyWasDown_[0]))) {
        return false;
    }
    return glfwGetKey(window_, glfwKey) == GLFW_PRESS;
}

bool UiInput::keyPressed(int glfwKey) {
    if (!window_ || glfwKey < 0 || glfwKey >= static_cast<int>(sizeof(keyWasDown_) / sizeof(keyWasDown_[0]))) {
        return false;
    }
    const bool down = glfwGetKey(window_, glfwKey) == GLFW_PRESS;
    const bool pressed = down && !keyWasDown_[glfwKey];
    keyWasDown_[glfwKey] = down;
    return pressed;
}

void UiInput::process(Widget* root, Keyable* keyable) {
    if (!window_) {
        return;
    }

    const bool mouseDown = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (mouseDown && !mouseWasDown_ && root) {
        const Widget* hit = widgetAt(root, mouseX_, mouseY_);
        if (!dynamic_cast<const Edit*>(hit)) {
            clearEditFocus(root);
        }
        root->dispatchClick(mouseX_, mouseY_);
        dragWidget_ = dragTargetAt(root, mouseX_, mouseY_);
    }
    if (mouseDown && dragWidget_) {
        dragWidget_->onDrag(mouseX_, mouseY_);
    }
    if (!mouseDown && mouseWasDown_) {
        if (dragWidget_) {
            dragWidget_->releasePressed();
            dragWidget_ = nullptr;
        }
        if (root) {
            root->releasePressed();
        }
    }
    mouseWasDown_ = mouseDown;

    if (scrollDelta_ != 0.f && root) {
        dispatchWheel(root, mouseX_, mouseY_, scrollDelta_);
        scrollDelta_ = 0.f;
    }

    Edit* focusedEdit = findFocusedEdit(root);
    if (focusedEdit) {
        if (pendingChar_ != 0) {
            const wchar_t ch = pendingChar_;
            pendingChar_ = 0;
            if (focusedEdit->filter() == Edit::FilterKind::Integer) {
                if (ch >= L'0' && ch <= L'9') {
                    focusedEdit->append(ch);
                }
            } else {
                focusedEdit->append(ch);
            }
        }
        if (keyPressed(GLFW_KEY_BACKSPACE)) {
            focusedEdit->backspace();
        }
        if (keyPressed(GLFW_KEY_ENTER) && focusedEdit->onSubmit) {
            focusedEdit->onSubmit();
        }
        return;
    }
    pendingChar_ = 0;

    if (keyable) {
        for (const auto& entry : keyable->bindings()) {
            if (keyPressed(entry.first)) {
                keyable->onKey(entry.first);
            }
        }
    }
}

} // namespace plasma
