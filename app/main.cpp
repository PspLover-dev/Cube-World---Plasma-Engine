#include "plasma/Engine.hpp"
#include "plasma/OpenGLEngine.hpp"
#include "plasma/PlxLoader.hpp"
#include "plasma/Widget.hpp"
#include "plasma/Keyable.hpp"
#include "plasma/UiInput.hpp"
#include "plasma/Math.hpp"
#include "cube/Assets.hpp"
#include "cube/TextureCatalog.hpp"
#include "cube/GuiHud.hpp"
#include "plasma/Filters.hpp"
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace {

struct GpuMesh {
    unsigned vao{0};
    unsigned vbo{0};
    unsigned ebo{0};
    int indexCount{0};

    void destroy() {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
        vao = vbo = ebo = 0;
        indexCount = 0;
    }

    void upload(const cube::CubMesh& mesh) {
        destroy();
        std::vector<float> verts;
        verts.reserve(mesh.vertices.size() * 9);
        for (const auto& v : mesh.vertices) {
            verts.push_back(v.px);
            verts.push_back(v.py);
            verts.push_back(v.pz);
            verts.push_back(v.nx);
            verts.push_back(v.ny);
            verts.push_back(v.nz);
            verts.push_back(v.r / 255.f);
            verts.push_back(v.g / 255.f);
            verts.push_back(v.b / 255.f);
        }
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                     verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)),
                     mesh.indices.data(), GL_STATIC_DRAW);
        const GLsizei stride = 9 * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
        glBindVertexArray(0);
        indexCount = static_cast<int>(mesh.indices.size());
    }
};

plasma::Mat4 cubModelMatrix(const cube::CubModel& model, float yaw) {
    plasma::Mat4 center = plasma::Mat4::identity();
    center.m[12] = -static_cast<float>(model.width) * 0.5f;
    center.m[13] = -static_cast<float>(model.depth) * 0.5f;
    center.m[14] = -static_cast<float>(model.height) * 0.5f;

    const float maxDim = static_cast<float>(
        std::max(std::max(model.width, model.depth), std::max(model.height, 1)));
    plasma::Mat4 scale = plasma::Mat4::identity();
    const float fit = 2.5f;
    const float s = fit / maxDim;
    scale.m[0] = scale.m[5] = scale.m[10] = s;

    return plasma::multiply(plasma::multiply(plasma::rotationY(yaw), scale), center);
}

plasma::Button* addButton(plasma::Widget* parent, const char* name, const char* label, float x, float y,
                          float w, float h, std::function<void()> onPressed) {
    auto* btn = new plasma::Button();
    btn->setName(name);
    btn->label = label;
    btn->bounds() = {x, y, w, h};
    btn->onPressed = std::move(onPressed);
    parent->addChild(btn);
    return btn;
}

} // namespace

int main(int argc, char** argv) {
    const std::string root = (argc > 1) ? argv[1] : ".";
    const std::string data1 = root + "/data1.db";
    const std::string data3 = root + "/data3.db";
    const std::string guiPlx = root + "/gui.plx";

    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Plasma Engine Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed\n";
        return 1;
    }

    plasma::Display display;
    display.attachWindow(window);
    display.setEngine(std::make_unique<plasma::OpenGLEngine>());
    auto* engine = static_cast<plasma::OpenGLEngine*>(display.engine());

    cube::TextureCatalog textures;
    const bool texturesOk = textures.open(data3);
    if (!texturesOk) {
        std::cerr << "Warning: could not open " << data3 << '\n';
    } else {
        std::cout << "Textures in data3.db: " << textures.keys().size() << '\n';
    }

    plasma::PlxDocument plx;
    if (!plx.loadFromFile(guiPlx)) {
        std::cerr << "Warning: could not load " << guiPlx << '\n';
    } else {
        std::cout << "Loaded gui.plx v" << plx.version() << " widgets=" << plx.widgets().size();
        for (const plasma::PlxWidgetDef& w : plx.widgets()) {
            std::cout << " [" << w.name << ":" << w.typeId << "]";
        }
        std::cout << " stream=" << plx.blob().size() << " indices=" << plx.indexTable().size()
                  << " format=" << (plx.isCompiledFormat() ? "compiled" : "attribute") << '\n';
        const auto& root = plx.root();
        for (const plasma::PlxNode& child : root.children) {
            std::cout << "  node " << child.widgetType << " " << child.name;
            if (!child.texture.empty()) {
                std::cout << " tex=" << child.texture;
            }
            std::cout << " children=" << child.children.size() << '\n';
        }
    }
    auto uiRoot = plx.buildRootWidget(*engine, texturesOk ? &textures : nullptr);
    plasma::pruneTextlessButtons(uiRoot.get());

    cube::GuiHud hud;
    if (!hud.init(textures, *engine)) {
        std::cerr << "Warning: HUD icons not loaded from data3.db\n";
    } else {
        std::cout << "HUD icons loaded: " << hud.icons().size() << '\n';
    }

    cube::ModelCatalog catalog;
    if (!catalog.open(data1)) {
        std::cerr << "Failed to open " << data1 << '\n';
        return 1;
    }
    auto modelNames = catalog.modelNames();
    std::cout << "Models in data1.db: " << modelNames.size() << '\n';

    plasma::ScrollBar* modelPanel = nullptr;
    for (plasma::Widget* c : uiRoot->children()) {
        if (c && c->name() == "model_panel") {
            modelPanel = dynamic_cast<plasma::ScrollBar*>(c);
        }
    }
    if (!modelPanel) {
        modelPanel = new plasma::ScrollBar();
        modelPanel->setName("model_panel");
        modelPanel->bounds() = {8.f, 40.f, 260.f, 520.f};
        uiRoot->addChild(modelPanel);
    }
    plasma::ListWidget* modelList = modelPanel->list();
    modelList->items.clear();
    modelList->items = modelNames;
    modelPanel->layout();
    std::cout << "Model list: " << modelList->items.size() << " .cub files\n";
    std::cout << "Controls: Prev/Next, click list, slider, #+Enter, wheel, Space=rotate\n";

    int selectedModel = 0;
    bool autoRotate = true;
    if (!modelList->items.empty()) {
        for (size_t i = 0; i < modelList->items.size(); ++i) {
            if (modelList->items[i].find("barrel") != std::string::npos) {
                selectedModel = static_cast<int>(i);
                break;
            }
        }
    }
    modelList->selected = selectedModel;

    GpuMesh gpuMesh;
    cube::CubModel model;
    auto syncListScroll = [&]() {
        if (modelList->visibleRowCount() <= 0) {
            return;
        }
        if (selectedModel < modelList->scrollOffset) {
            modelList->setScrollOffset(selectedModel);
        } else if (selectedModel >= modelList->scrollOffset + modelList->visibleRowCount()) {
            modelList->setScrollOffset(selectedModel - modelList->visibleRowCount() + 1);
        }
        modelPanel->syncSlider();
    };

    auto reloadModel = [&]() {
        if (modelList->items.empty()) {
            return;
        }
        selectedModel = (selectedModel % static_cast<int>(modelList->items.size()) +
                         static_cast<int>(modelList->items.size())) %
                        static_cast<int>(modelList->items.size());
        modelList->selected = selectedModel;
        syncListScroll();
        if (catalog.loadModel(modelList->items[static_cast<size_t>(selectedModel)], model)) {
            const cube::CubMesh mesh = catalog.buildMesh(model, 1.f);
            gpuMesh.upload(mesh);
            const std::string title = "Plasma - " + modelList->items[static_cast<size_t>(selectedModel)] +
                                      " (" + std::to_string(selectedModel + 1) + "/" +
                                      std::to_string(modelList->items.size()) + ")";
            glfwSetWindowTitle(window, title.c_str());
            std::cout << "Model: " << modelList->items[static_cast<size_t>(selectedModel)] << " ("
                      << model.width << "x" << model.depth << "x" << model.height << ")\n";
        }
    };
    reloadModel();

    auto prevModel = [&]() {
        --selectedModel;
        reloadModel();
    };
    auto nextModel = [&]() {
        ++selectedModel;
        reloadModel();
    };
    auto randomModel = [&]() {
        if (modelList->items.empty()) {
            return;
        }
        selectedModel = static_cast<int>(std::rand() % modelList->items.size());
        reloadModel();
    };
    auto toggleRotate = [&]() { autoRotate = !autoRotate; };

    modelList->onSelectionChanged = [&](int idx) {
        selectedModel = idx;
        reloadModel();
    };

    auto* indexEdit = new plasma::Edit();
    indexEdit->setName("model_index");
    indexEdit->bounds() = {276.f, 40.f, 96.f, 26.f};
    indexEdit->setFilter(plasma::Edit::FilterKind::Integer);
    indexEdit->hint = L"#";
    indexEdit->onSubmit = [&]() {
        int index = 0;
        if (!plasma::IntegerFilter::accept(indexEdit->text, index) || modelList->items.empty()) {
            return;
        }
        selectedModel = std::max(0, std::min(index - 1, static_cast<int>(modelList->items.size()) - 1));
        reloadModel();
    };
    uiRoot->addChild(indexEdit);

    addButton(uiRoot.get(), "btn_prev", "Prev", 380.f, 8.f, 72.f, 26.f, prevModel);
    addButton(uiRoot.get(), "btn_next", "Next", 458.f, 8.f, 72.f, 26.f, nextModel);
    addButton(uiRoot.get(), "btn_random", "Random", 536.f, 8.f, 80.f, 26.f, randomModel);
    plasma::Button* btnPause = addButton(uiRoot.get(), "btn_pause", "Pause", 622.f, 8.f, 72.f, 26.f, {});
    btnPause->onPressed = [&]() {
        toggleRotate();
        btnPause->label = autoRotate ? "Pause" : "Spin";
    };

    plasma::Keyable modelKeys;
    modelKeys.setName("model_keys");
    modelKeys.bindKey(GLFW_KEY_UP, prevModel);
    modelKeys.bindKey(GLFW_KEY_DOWN, nextModel);
    modelKeys.bindKey(GLFW_KEY_PAGE_UP, prevModel);
    modelKeys.bindKey(GLFW_KEY_PAGE_DOWN, nextModel);
    modelKeys.bindKey(GLFW_KEY_R, randomModel);
    modelKeys.bindKey(GLFW_KEY_SPACE, toggleRotate);

    plasma::UiInput uiInput;
    uiInput.attach(window);

    float yaw = 0.f;
    auto last = std::chrono::steady_clock::now();

    while (!display.shouldClose()) {
        display.pollEvents();
        uiInput.beginFrame();
        uiInput.process(uiRoot.get(), &modelKeys);

        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (autoRotate) {
            yaw += dt * 0.8f;
        }

        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        display.setSize(w, h);
        glViewport(0, 0, w, h);

        engine->beginFrame(display);

        plasma::Mat4 proj;
        plasma::setPerspective(proj, 0.9f, w > 0 ? static_cast<float>(w) / h : 1.f, 0.1f, 500.f);
        plasma::Vec3 eye{4.f, 3.f, 9.f};
        plasma::Vec3 center{0.f, 0.f, 0.f};
        plasma::Vec3 up{0.f, 1.f, 0.f};
        plasma::Mat4 view;
        plasma::setLookAt(view, eye, center, up);
        engine->setProjectionMatrix(proj);
        engine->setViewMatrix(view);

        const plasma::Mat4 modelMat = cubModelMatrix(model, yaw);
        engine->setWorldMatrix(modelMat);

        if (gpuMesh.indexCount > 0) {
            engine->drawVoxelMesh(gpuMesh.vao, gpuMesh.indexCount, modelMat);
        }

        uiRoot->update(dt);
        uiRoot->draw(*engine, w, h);
        hud.draw(*engine, w, h);

        engine->endFrame(display);
    }

    gpuMesh.destroy();
    glfwTerminate();
    return 0;
}
