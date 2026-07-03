#include "plasma/Engine.hpp"
#include "plasma/OpenGLEngine.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace plasma {

Display::Display() = default;

Display::~Display() = default;

void Display::attachWindow(void* glfwWindow) {
    window_ = glfwWindow;
    if (window_) {
        glfwGetFramebufferSize(static_cast<GLFWwindow*>(window_), &width_, &height_);
    }
}

void Display::setSize(int w, int h) {
    width_ = w;
    height_ = h;
}

void Display::setEngine(std::unique_ptr<Engine> engine) {
    engine_ = std::move(engine);
}

void Display::pollEvents() {
    if (window_) {
        glfwPollEvents();
    }
}

bool Display::shouldClose() const {
    return window_ && glfwWindowShouldClose(static_cast<GLFWwindow*>(window_));
}

void Display::swapBuffers() {
    if (window_) {
        glfwSwapBuffers(static_cast<GLFWwindow*>(window_));
    }
}

} // namespace plasma
