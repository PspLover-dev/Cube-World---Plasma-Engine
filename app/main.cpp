#include "plasma/Engine.hpp"

#include "plasma/OpenGLEngine.hpp"

#include "plasma/Math.hpp"

#include "plasma/UiInput.hpp"

#include "plasma/UiDraw.hpp"

#include "plasma/Widget.hpp"

#include "plasma/Font.hpp"

#include "cube/Assets.hpp"

#include "cube/AudioFileLoader.hpp"

#include "cube/RegistryModelResolver.hpp"

#include "cube/RetailFonts.hpp"

#include "cube/TextureCatalog.hpp"

#include "cube/Types.hpp"

#include "cube/XAudio2Engine.hpp"



#define GLFW_INCLUDE_NONE

#include <GL/glew.h>

#include <GLFW/glfw3.h>



#include <algorithm>

#include <array>

#include <chrono>

#include <cmath>

#include <iostream>

#include <memory>

#include <optional>

#include <string>

#include <vector>



#include <sstream>



namespace {



enum class ModelSource { Model, Db, Race, Items, Equip };



struct ModelListEntry {

    std::string label;

    ModelSource source{ModelSource::Db};

    std::string creaturePrefix;

    std::string modelKey;

    std::string registryName;

    uint32_t registryId{0};

    uint32_t equipSlot{0};

    uint32_t equipSub{0};

};



constexpr float kBottomPanelH = 220.f;

constexpr float kSidePanelW = 280.f;



class IconGallery : public plasma::Widget {
public:
    cube::TextureCatalog* catalog{nullptr};
    plasma::Engine* engine{nullptr};
    std::vector<std::string> keys;
    int scrollCol{0};
    float iconSize{52.f};
    float gap{4.f};

    int visibleColumns() const {
        const float cell = iconSize + gap;
        return std::max(1, static_cast<int>((bounds().w - 8.f) / cell));
    }

    int rowCount() const {
        const float cell = iconSize + gap;
        return std::max(1, static_cast<int>((bounds().h - 8.f) / cell));
    }

    int maxScrollCol() const {
        const int rows = rowCount();
        if (rows <= 0) {
            return 0;
        }
        const int totalCols =
            static_cast<int>((keys.size() + static_cast<size_t>(rows) - 1) / static_cast<size_t>(rows));
        return std::max(0, totalCols - visibleColumns());
    }

    void draw(plasma::Engine& eng, int screenW, int screenH) override {
        if (!visible() || catalog == nullptr || engine == nullptr) {
            return;
        }
        plasma::drawSolidQuad(eng, bounds(), screenW, screenH, {0.12f, 0.13f, 0.16f, 0.98f});

        const float cell = iconSize + gap;
        const int cols = visibleColumns();
        const int rows = rowCount();
        const int startIdx = scrollCol * rows;

        for (int c = 0; c < cols; ++c) {
            for (int r = 0; r < rows; ++r) {
                const size_t idx = static_cast<size_t>(startIdx + c * rows + r);
                if (idx >= keys.size()) {
                    continue;
                }
                plasma::Texture* tex = catalog->gpuTexture(*engine, keys[idx]);
                if (tex == nullptr) {
                    continue;
                }
                const plasma::Widget::Rect iconRect{bounds().x + 4.f + static_cast<float>(c) * cell,
                                                    bounds().y + 4.f + static_cast<float>(r) * cell,
                                                    iconSize, iconSize};
                plasma::drawSolidQuad(eng, iconRect, screenW, screenH,
                                      {0.22f, 0.24f, 0.28f, 1.f});
                plasma::drawTexturedQuad(eng, iconRect, screenW, screenH, tex);
            }
        }
    }

    bool onMouseWheel(float delta) override {
        if (delta == 0.f) {
            return false;
        }
        scrollCol += delta > 0.f ? -2 : 2;
        scrollCol = std::clamp(scrollCol, 0, maxScrollCol());
        return true;
    }
};



struct GpuMesh {

    unsigned vao{0};

    unsigned vbo{0};

    unsigned ebo{0};

    int indexCount{0};

    float footOffsetZ{0.f};



    void destroy() {

        if (vao) {

            glDeleteVertexArrays(1, &vao);

        }

        if (vbo) {

            glDeleteBuffers(1, &vbo);

        }

        if (ebo) {

            glDeleteBuffers(1, &ebo);

        }

        vao = vbo = ebo = 0;

        indexCount = 0;

        footOffsetZ = 0.f;

    }



    void upload(const cube::CubMesh& mesh) {

        destroy();

        std::vector<float> verts;

        verts.reserve(mesh.vertices.size() * 9);

        float minZ = mesh.vertices.empty() ? 0.f : mesh.vertices[0].pz;

        for (const auto& v : mesh.vertices) {

            minZ = std::min(minZ, v.pz);

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

        footOffsetZ = -minZ;



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

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,

                              reinterpret_cast<void*>(3 * sizeof(float)));

        glEnableVertexAttribArray(2);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,

                              reinterpret_cast<void*>(6 * sizeof(float)));

        glBindVertexArray(0);

        indexCount = static_cast<int>(mesh.indices.size());

    }

};



plasma::Mat4 translation(float x, float y, float z) {

    plasma::Mat4 m = plasma::Mat4::identity();

    m.m[12] = x;

    m.m[13] = y;

    m.m[14] = z;

    return m;

}



plasma::Mat4 scaleXYZ(float sx, float sy, float sz) {

    plasma::Mat4 m = plasma::Mat4::identity();

    m.m[0] = sx;

    m.m[5] = sy;

    m.m[10] = sz;

    return m;

}



plasma::Mat4 cubPartMatrix(const cube::CubModel& model, float footOffsetZ, float yaw,

                           const plasma::Vec3& offset, float scaleXY, float scaleZ) {

    plasma::Mat4 center = plasma::Mat4::identity();

    center.m[12] = -static_cast<float>(model.width) * 0.5f;

    center.m[13] = -static_cast<float>(model.depth) * 0.5f;



    plasma::Mat4 feet = plasma::Mat4::identity();

    feet.m[14] = footOffsetZ;



    const plasma::Mat4 world =

        plasma::multiply(plasma::multiply(translation(0.f, 0.f, 0.f), plasma::rotationZ(yaw)),

                         translation(offset[0], offset[1], offset[2]));

    const plasma::Mat4 scaled = plasma::multiply(world, scaleXYZ(scaleXY, scaleXY, scaleZ));

    return plasma::multiply(plasma::multiply(scaled, feet), center);

}



void drawMesh(plasma::OpenGLEngine* engine, const GpuMesh& mesh, const plasma::Mat4& worldMat) {

    engine->setWorldMatrix(worldMat);

    engine->drawVoxelMesh(mesh.vao, mesh.indexCount, worldMat);

}



void buildPreviewCamera(int vpW, int vpH, plasma::Mat4& proj, plasma::Mat4& view) {

    const float aspect = vpW > 0 ? static_cast<float>(vpW) / static_cast<float>(vpH) : 1.f;

    plasma::setPerspective(proj, 0.85f, aspect, 0.1f, 600.f);

    const plasma::Vec3 eye{90.f, -130.f, 70.f};

    const plasma::Vec3 center{0.f, 0.f, 18.f};

    const plasma::Vec3 up{0.f, 0.f, 1.f};

    plasma::setLookAt(view, eye, center, up);

}



std::string prefixToLabel(const std::string& prefix) {

    std::string out;

    for (size_t i = 0; i < prefix.size(); ++i) {

        const char c = prefix[i];

        if (c == '-') {

            if (!out.empty() && out.back() != ' ') {

                out += ' ';

            }

            continue;

        }

        if (out.empty() || out.back() == ' ') {

            out += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        } else {

            out += c;

        }

    }

    return out;

}



std::vector<std::string> listBodyPrefixes(cube::ModelCatalog& catalog) {

    std::vector<std::string> prefixes;

    for (const std::string& key : catalog.modelNames()) {

        const std::string suffix = "-body.cub";

        if (key.size() > suffix.size() &&

            key.compare(key.size() - suffix.size(), suffix.size(), suffix) == 0) {

            prefixes.push_back(key.substr(0, key.size() - suffix.size()));

        }

    }

    std::sort(prefixes.begin(), prefixes.end());

    prefixes.erase(std::unique(prefixes.begin(), prefixes.end()), prefixes.end());

    return prefixes;

}



struct PartMesh {

    GpuMesh gpu;

    cube::CubModel model;

    cube::PartKind kind{cube::PartKind::Body};

};



struct AppState {

    cube::ModelCatalog catalog;

    cube::TextureCatalog icons;

    cube::XAudio2Engine audio;

    cube::RegistryModelResolver modelResolver;



    std::vector<std::string> creaturePrefixes;

    std::vector<std::string> allModelKeys;

    std::vector<std::string> soundKeys;

    std::vector<std::string> musicKeys;

    std::string gameRoot;

    std::string selectedMusicPath;



    std::vector<ModelListEntry> modelEntries;

    ModelSource modelSource{ModelSource::Db};



    std::vector<PartMesh> creatureParts;

    std::string selectedPrefix{"desert-nomad"};

    std::string selectedAudioKey;

    bool musicMode{false};

    std::string statusLine{"Ready"};



    plasma::Widget root;

    plasma::Widget sidePanel;

    plasma::Widget bottomPanel;

    plasma::ListWidget modelList;

    plasma::Button modelModeButton;

    plasma::Button dbModeButton;

    plasma::Button raceModeButton;

    plasma::Button itemsModeButton;

    plasma::Button equipModeButton;

    plasma::ListWidget audioList;

    plasma::Button playButton;

    plasma::Button soundModeButton;

    plasma::Button musicModeButton;

    IconGallery iconGallery;



    plasma::UiInput uiInput;

};



const std::array<const char*, 5> kPartFiles = {"-body.cub", "-foot.cub", "-head1.cub", "-hand.cub",

                                               "-hand2.cub"};

const std::array<cube::PartKind, 5> kPartKinds = {

    cube::PartKind::Body, cube::PartKind::Foot, cube::PartKind::Head, cube::PartKind::Hand,

    cube::PartKind::Hand};



bool loadSingleModel(AppState& app, const std::string& modelKey) {

    for (PartMesh& part : app.creatureParts) {

        part.gpu.destroy();

    }

    app.creatureParts.clear();



    PartMesh part;

    part.kind = cube::PartKind::Body;

    if (!app.catalog.loadModel(modelKey, part.model)) {

        app.statusLine = "No model: " + modelKey;

        return false;

    }

    const cube::CubMesh mesh = app.catalog.buildMesh(part.model, 1.f);

    part.gpu.upload(mesh);

    app.creatureParts.push_back(std::move(part));

    app.statusLine = "Model: " + modelKey;

    return true;

}



bool loadCreaturePrefix(AppState& app, const std::string& prefix) {

    for (PartMesh& part : app.creatureParts) {

        part.gpu.destroy();

    }

    app.creatureParts.clear();



    int loaded = 0;

    for (size_t i = 0; i < kPartFiles.size(); ++i) {

        const std::string key = prefix + kPartFiles[i];

        PartMesh part;

        part.kind = kPartKinds[i];

        cube::CubModel model;

        if (!app.catalog.loadModel(key, model)) {

            continue;

        }

        const cube::CubMesh mesh = app.catalog.buildMesh(model, 1.f);

        part.gpu.upload(mesh);

        part.model = std::move(model);

        app.creatureParts.push_back(std::move(part));

        ++loaded;

    }



    app.selectedPrefix = prefix;

    if (loaded == 0) {

        app.statusLine = "No parts for " + prefix;

        return false;

    }

    app.statusLine = "Creature: " + prefixToLabel(prefix) + " (" + std::to_string(loaded) + " parts)";

    return true;

}



bool loadRegistryCreature(AppState& app, const std::string& name) {

    const std::optional<cube::RegistryCreatureModel> resolved = app.modelResolver.resolveCreature(name);

    if (!resolved) {

        app.statusLine = "No model for registry entry: " + name;

        for (PartMesh& part : app.creatureParts) {

            part.gpu.destroy();

        }

        app.creatureParts.clear();

        return false;

    }

    if (resolved->multiPart) {

        return loadCreaturePrefix(app, resolved->prefix);

    }

    return loadSingleModel(app, resolved->prefix + ".cub");

}



bool loadRegistryEquip(AppState& app, const std::string& name) {

    const std::optional<cube::RegistryEquipModel> resolved = app.modelResolver.resolveEquip(name);

    if (!resolved) {

        app.statusLine = "No model for equip entry: " + name;

        for (PartMesh& part : app.creatureParts) {

            part.gpu.destroy();

        }

        app.creatureParts.clear();

        return false;

    }

    return loadSingleModel(app, resolved->modelKey);

}



bool loadModelEntry(AppState& app, const ModelListEntry& entry) {

    switch (entry.source) {

    case ModelSource::Model:

        return loadSingleModel(app, entry.modelKey);

    case ModelSource::Db:

        return loadCreaturePrefix(app, entry.creaturePrefix);

    case ModelSource::Race:

    case ModelSource::Items:

        return loadRegistryCreature(app, entry.registryName);

    case ModelSource::Equip:

        return loadRegistryEquip(app, entry.registryName);

    }

    return false;

}



void updateModelSourceButtons(AppState& app) {

    app.modelModeButton.setPressed(app.modelSource == ModelSource::Model);

    app.dbModeButton.setPressed(app.modelSource == ModelSource::Db);

    app.raceModeButton.setPressed(app.modelSource == ModelSource::Race);

    app.itemsModeButton.setPressed(app.modelSource == ModelSource::Items);

    app.equipModeButton.setPressed(app.modelSource == ModelSource::Equip);

}



void refreshModelList(AppState& app) {

    app.modelEntries.clear();

    app.modelList.items.clear();



    switch (app.modelSource) {

    case ModelSource::Model:

        for (const std::string& key : app.allModelKeys) {

            ModelListEntry entry;

            entry.source = ModelSource::Model;

            entry.label = key;

            entry.modelKey = key;

            app.modelEntries.push_back(std::move(entry));

        }

        break;

    case ModelSource::Db:

        for (const std::string& prefix : app.creaturePrefixes) {

            ModelListEntry entry;

            entry.source = ModelSource::Db;

            entry.label = prefixToLabel(prefix);

            entry.creaturePrefix = prefix;

            app.modelEntries.push_back(std::move(entry));

        }

        break;

    case ModelSource::Race:

        for (const cube::RegistryNamedId& race : cube::RegistryModelResolver::worldRaces()) {

            ModelListEntry entry;

            entry.source = ModelSource::Race;

            entry.registryId = race.id;

            entry.registryName = race.name;

            std::ostringstream label;

            label << "0x" << std::hex << race.id << std::dec << ' ' << race.name;

            entry.label = label.str();

            app.modelEntries.push_back(std::move(entry));

        }

        break;

    case ModelSource::Items:

        for (const cube::RegistryNamedId& item : cube::RegistryModelResolver::worldItems()) {

            ModelListEntry entry;

            entry.source = ModelSource::Items;

            entry.registryId = item.id;

            entry.registryName = item.name;

            std::ostringstream label;

            label << "0x" << std::hex << item.id << std::dec << ' ' << item.name;

            entry.label = label.str();

            app.modelEntries.push_back(std::move(entry));

        }

        break;

    case ModelSource::Equip:

        for (const cube::RegistryNamedId& equip : cube::RegistryModelResolver::worldEquip()) {

            ModelListEntry entry;

            entry.source = ModelSource::Equip;

            entry.equipSlot = equip.id;

            entry.equipSub = equip.subId;

            entry.registryName = equip.name;

            std::ostringstream label;

            label << equip.name << " [" << equip.id << ':' << equip.subId << ']';

            entry.label = label.str();

            app.modelEntries.push_back(std::move(entry));

        }

        break;

    }



    for (const ModelListEntry& entry : app.modelEntries) {

        app.modelList.items.push_back(entry.label);

    }

    app.modelList.selected = app.modelEntries.empty()

                                 ? 0

                                 : std::clamp(app.modelList.selected, 0,

                                              static_cast<int>(app.modelEntries.size()) - 1);

}



void setModelSource(AppState& app, ModelSource source) {

    app.modelSource = source;

    updateModelSourceButtons(app);

    refreshModelList(app);

    if (!app.modelEntries.empty()) {

        loadModelEntry(app, app.modelEntries[static_cast<size_t>(app.modelList.selected)]);

    }

}



void refreshAudioList(AppState& app) {

    app.audioList.items.clear();

    const std::vector<std::string>& src = app.musicMode ? app.musicKeys : app.soundKeys;

    for (const std::string& key : src) {

        app.audioList.items.push_back(key);

    }

    if (app.audioList.items.empty()) {

        app.audioList.selected = 0;

        app.selectedAudioKey.clear();

        return;

    }

    app.audioList.selected =

        std::clamp(app.audioList.selected, 0, static_cast<int>(app.audioList.items.size()) - 1);

    app.selectedAudioKey = app.audioList.items[static_cast<size_t>(app.audioList.selected)];

    if (app.musicMode) {
        app.selectedMusicPath = app.gameRoot + "/Music/" + app.selectedAudioKey;
    }

}



void setAudioMode(AppState& app, bool music) {

    app.musicMode = music;

    app.soundModeButton.setPressed(!music);

    app.musicModeButton.setPressed(music);

    refreshAudioList(app);

}



void playSelectedAudio(AppState& app) {

    if (app.selectedAudioKey.empty()) {

        app.statusLine = "No audio selected";

        return;

    }

    cube::Database& db = app.audio.database();

    if (app.musicMode) {

        if (app.selectedMusicPath.empty()) {

            app.statusLine = "No music track selected";

            return;

        }

        if (!app.audio.music().loadFromFile(app.selectedMusicPath)) {

            app.statusLine = "Failed to load music: " + app.selectedMusicPath;

            return;

        }

        app.audio.music().play(true);

        app.statusLine = "Music: " + app.selectedAudioKey;

    } else {

        if (!app.audio.sound().loadFromDatabase(db, app.selectedAudioKey)) {

            app.statusLine = "Failed to load sound: " + app.selectedAudioKey;

            return;

        }

        app.audio.sound().play();

        app.statusLine = "Sound: " + app.selectedAudioKey;

    }

}



void layoutUi(AppState& app, int screenW, int screenH) {

    const float mainH = std::max(0.f, static_cast<float>(screenH) - kBottomPanelH);



    app.sidePanel.bounds() = {0.f, 0.f, kSidePanelW, mainH};

    app.bottomPanel.bounds() = {0.f, mainH, static_cast<float>(screenW), kBottomPanelH};



    app.modelModeButton.bounds() = {8.f, 4.f, 50.f, 24.f};

    app.dbModeButton.bounds() = {62.f, 4.f, 50.f, 24.f};

    app.raceModeButton.bounds() = {116.f, 4.f, 50.f, 24.f};

    app.itemsModeButton.bounds() = {170.f, 4.f, 50.f, 24.f};

    app.equipModeButton.bounds() = {224.f, 4.f, 50.f, 24.f};



    app.modelList.bounds() = {8.f, 36.f, kSidePanelW - 16.f, mainH * 0.42f - 40.f};

    app.modelList.rowHeight = 18.f;



    app.soundModeButton.bounds() = {8.f, mainH * 0.42f + 4.f, 80.f, 24.f};

    app.musicModeButton.bounds() = {96.f, mainH * 0.42f + 4.f, 80.f, 24.f};

    app.audioList.bounds() = {8.f, mainH * 0.42f + 34.f, kSidePanelW - 16.f, mainH * 0.38f - 40.f};

    app.audioList.rowHeight = 18.f;

    app.playButton.bounds() = {8.f, mainH - 36.f, kSidePanelW - 16.f, 28.f};



    app.iconGallery.bounds() = {8.f, mainH + 8.f, static_cast<float>(screenW) - 16.f, kBottomPanelH - 16.f};



    app.playButton.label = "Play selected";

}



void buildUi(AppState& app, plasma::Engine& engine) {

    app.root.setName("Root");

    app.sidePanel.setName("SidePanel");

    app.bottomPanel.setName("BottomPanel");



    app.modelModeButton.label = "Model";

    app.dbModeButton.label = "DB";

    app.raceModeButton.label = "Race";

    app.itemsModeButton.label = "Items";

    app.equipModeButton.label = "Equip";

    app.modelModeButton.onPressed = [&app]() { setModelSource(app, ModelSource::Model); };

    app.dbModeButton.onPressed = [&app]() { setModelSource(app, ModelSource::Db); };

    app.raceModeButton.onPressed = [&app]() { setModelSource(app, ModelSource::Race); };

    app.itemsModeButton.onPressed = [&app]() { setModelSource(app, ModelSource::Items); };

    app.equipModeButton.onPressed = [&app]() { setModelSource(app, ModelSource::Equip); };



    app.modelList.setName("ModelList");

    refreshModelList(app);

    app.modelList.onSelectionChanged = [&app](int index) {

        if (index >= 0 && index < static_cast<int>(app.modelEntries.size())) {

            loadModelEntry(app, app.modelEntries[static_cast<size_t>(index)]);

        }

    };



    app.soundModeButton.label = "Sound";

    app.musicModeButton.label = "Music";

    app.soundModeButton.onPressed = [&app]() { setAudioMode(app, false); };

    app.musicModeButton.onPressed = [&app]() { setAudioMode(app, true); };

    setAudioMode(app, false);



    app.audioList.onSelectionChanged = [&app](int index) {

        if (index >= 0 && index < static_cast<int>(app.audioList.items.size())) {

            app.selectedAudioKey = app.audioList.items[static_cast<size_t>(index)];

            if (app.musicMode) {

                app.selectedMusicPath = app.gameRoot + "/Music/" + app.selectedAudioKey;

            }

        }

    };



    app.playButton.onPressed = [&app]() { playSelectedAudio(app); };



    app.iconGallery.setName("IconGallery");

    app.iconGallery.catalog = &app.icons;

    app.iconGallery.engine = &engine;

    app.iconGallery.keys = app.icons.keys();

    std::sort(app.iconGallery.keys.begin(), app.iconGallery.keys.end());

    for (const std::string& key : app.iconGallery.keys) {

        app.icons.gpuTexture(engine, key);

    }



    app.sidePanel.addChild(&app.modelModeButton);

    app.sidePanel.addChild(&app.dbModeButton);

    app.sidePanel.addChild(&app.raceModeButton);

    app.sidePanel.addChild(&app.itemsModeButton);

    app.sidePanel.addChild(&app.equipModeButton);

    app.sidePanel.addChild(&app.modelList);

    app.sidePanel.addChild(&app.soundModeButton);

    app.sidePanel.addChild(&app.musicModeButton);

    app.sidePanel.addChild(&app.audioList);

    app.sidePanel.addChild(&app.playButton);



    app.bottomPanel.addChild(&app.iconGallery);



    app.root.addChild(&app.sidePanel);

    app.root.addChild(&app.bottomPanel);

}



std::vector<std::string> loadSoundKeys(cube::AssetDatabase& db) {

    return db.listKeys("%.wav");

}



} // namespace



int main(int argc, char** argv) {

    const std::string root = (argc > 1) ? argv[1] : ".";

    const std::string data1 = root + "/data1.db";

    const std::string data2 = root + "/data2.db";

    const std::string data3 = root + "/data3.db";



    if (!glfwInit()) {

        std::cerr << "GLFW init failed\n";

        return 1;

    }



    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



    GLFWwindow* window =

        glfwCreateWindow(1280, 720, "Cube World — Creature & Audio Viewer", nullptr, nullptr);

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



    AppState app;

    app.gameRoot = root;

    if (!cube::fonts::init(root)) {
        std::cerr << "Warning: failed to load resource1.dat / resource2.dat from " << root << '\n';
    }

    if (!app.catalog.open(data1)) {

        std::cerr << "Failed to open " << data1 << '\n';

        return 1;

    }

    if (!app.icons.open(data3)) {

        std::cerr << "Warning: failed to open " << data3 << " (icons unavailable)\n";

    }

    if (!app.audio.init() || !app.audio.loadDatabase(data2)) {

        std::cerr << "Warning: failed to init audio or open " << data2 << '\n';

    }



    app.creaturePrefixes = listBodyPrefixes(app.catalog);

    app.allModelKeys = app.catalog.modelNames();

    app.modelResolver.rebuild(app.catalog);

    app.soundKeys = loadSoundKeys(app.audio.database().db());

    app.musicKeys = cube::audio::listMusicTracks(root);

    if (app.musicKeys.empty()) {

        std::cerr << "Note: no music in " << root << "/Music/ — copy the Music folder from your Cube World install\n";

    }



    buildUi(app, *engine);

    app.uiInput.attach(window);



    updateModelSourceButtons(app);

    auto nomadIt = std::find(app.creaturePrefixes.begin(), app.creaturePrefixes.end(), "desert-nomad");

    if (nomadIt != app.creaturePrefixes.end()) {

        app.modelList.selected = static_cast<int>(nomadIt - app.creaturePrefixes.begin());

    }

    loadCreaturePrefix(app, app.selectedPrefix);



    if (!app.soundKeys.empty()) {

        app.selectedAudioKey = app.soundKeys.front();

    } else if (!app.musicKeys.empty()) {

        setAudioMode(app, true);

        app.selectedAudioKey = app.musicKeys.front();

        app.selectedMusicPath = app.gameRoot + "/Music/" + app.selectedAudioKey;

    }



    auto last = std::chrono::steady_clock::now();

    float previewYaw = 0.f;



    while (!display.shouldClose()) {

        display.pollEvents();

        app.uiInput.beginFrame();

        app.uiInput.process(&app.root);

        const auto now = std::chrono::steady_clock::now();

        const float dt = std::min(std::chrono::duration<float>(now - last).count(), 0.05f);

        last = now;

        previewYaw += dt * 0.35f;

        app.root.update(dt);



        int w = 0, h = 0;

        glfwGetFramebufferSize(window, &w, &h);

        display.setSize(w, h);

        layoutUi(app, w, h);



        const int vpX = static_cast<int>(kSidePanelW);

        const int vpW = std::max(1, w - vpX);

        const int vpH = std::max(1, h - static_cast<int>(kBottomPanelH));



        engine->beginFrame(display);



        glEnable(GL_DEPTH_TEST);

        glEnable(GL_CULL_FACE);

        glEnable(GL_SCISSOR_TEST);

        glViewport(vpX, static_cast<int>(kBottomPanelH), vpW, vpH);

        glScissor(vpX, static_cast<int>(kBottomPanelH), vpW, vpH);



        plasma::Mat4 proj;

        plasma::Mat4 view;

        buildPreviewCamera(vpW, vpH, proj, view);

        engine->setProjectionMatrix(proj);

        engine->setViewMatrix(view);



        const cube::AnimPose pose = cube::poseForAction(cube::ActionId::Idle, false);

        for (const PartMesh& part : app.creatureParts) {

            const plasma::Vec3 offset = cube::partOffset(part.kind, pose);

            const plasma::Mat4 worldMat =

                cubPartMatrix(part.model, part.gpu.footOffsetZ, previewYaw, offset, pose.scaleXY,

                              pose.scaleZ);

            drawMesh(engine, part.gpu, worldMat);

        }



        glDisable(GL_SCISSOR_TEST);

        glViewport(0, 0, w, h);

        glDisable(GL_DEPTH_TEST);



        app.root.draw(*engine, w, h);

        if (plasma::retailFontsReady()) {
            const std::wstring title = L"Cube World";
            plasma::scriptFont().drawText(*engine, title, static_cast<float>(vpX) + 16.f,
                                            static_cast<float>(h) - 36.f, {0.95f, 0.88f, 0.55f, 1.f}, w, h);

            const std::wstring status(app.statusLine.begin(), app.statusLine.end());
            plasma::uiFont().drawText(*engine, status, static_cast<float>(vpX) + 16.f,
                                       static_cast<float>(kBottomPanelH) + 8.f, {0.85f, 0.9f, 0.95f, 1.f}, w,
                                       h);
        }

        engine->endFrame(display);

    }



    for (PartMesh& part : app.creatureParts) {

        part.gpu.destroy();

    }

    glfwTerminate();

    return 0;

}

