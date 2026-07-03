#pragma once

struct GLFWwindow;

namespace plasma {

class Keyable;
class Widget;

/// GLFW-backed UI input: edge-triggered keys and mouse clicks for widget trees.
class UiInput {
public:
    void attach(GLFWwindow* window);

    void beginFrame();

    /// Dispatch mouse, scroll, text field input, and key bindings.
    void process(Widget* root, Keyable* keyable = nullptr);

    void onChar(unsigned int codepoint);

    bool keyPressed(int glfwKey);
    bool keyDown(int glfwKey) const;

    float mouseX() const { return mouseX_; }
    float mouseY() const { return mouseY_; }

    void addScroll(float delta);

private:
    GLFWwindow* window_{nullptr};
    bool keyWasDown_[512]{};
    bool mouseWasDown_{false};
    float mouseX_{0.f};
    float mouseY_{0.f};
    float scrollDelta_{0.f};
    Widget* dragWidget_{nullptr};
    wchar_t pendingChar_{0};
};

} // namespace plasma
