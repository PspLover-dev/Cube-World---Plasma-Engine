#include "plasma/PlxLoader.hpp"

#include "plasma/Exception.hpp"

#include "plasma/Widget.hpp"



#include "cube/TextureCatalog.hpp"



#include <algorithm>

#include <cstring>

#include <fstream>
#include <unordered_set>



namespace plasma {



namespace {



constexpr const char* kKnownKeys[] = {

    "PlasmaGraphics",       "Attribute.frame",         "Attribute.sequence",

    "Attribute.sequence.name", "Attribute.sequence.wname", "Attribute.sequence.key",

    "Attribute.sequence.key.frame", "Attribute.sequence.key.time",

    "Attribute.sequence.key.smoothness", "ArrayAttribute.size", "Button.type",

    "Widget.name",          "Widget.wname",          "Widget.caption",

    "Widget.innerBindPos",  "Widget.innerBindSize",  "Widget.bindPos",

    "Widget.bindSize",      "Widget.framePos",       "Widget.frameSize",

    "Widget.bindMatrix",    "Widget.horizontalAlignment", "Widget.verticalAlignment",

    "Widget.flags",         "Texture.name",          "Texture.file",

    "Deformer.texture",

};



bool readLeU32(const uint8_t*& p, const uint8_t* end, uint32_t& out) {

    if (p + 4 > end) {

        return false;

    }

    std::memcpy(&out, p, 4);

    p += 4;

    return true;

}



uint32_t readBeU32At(const uint8_t* p) {

    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |

           (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);

}



bool readBeU32(const uint8_t*& p, const uint8_t* end, uint32_t& out) {

    if (p + 4 > end) {

        return false;

    }

    out = readBeU32At(p);

    p += 4;

    return true;

}



std::string widgetTypeName(uint32_t id) {

    switch (id) {

    case 1: return "Widget";

    case 2: return "Button";

    case 3: return "ListWidget";

    case 4: return "Edit";

    default: return "Widget";

    }

}



} // namespace



PlxStreamReader::PlxStreamReader(const uint8_t* data, size_t size, size_t baseOffset)

    : data_(data), size_(size), base_(baseOffset) {}



size_t PlxStreamReader::tell() const { return base_ + pos_; }



bool PlxStreamReader::eof() const { return pos_ >= size_; }



bool PlxStreamReader::readU32(uint32_t& out) {

    if (pos_ + 4 > size_) {

        return false;

    }

    std::memcpy(&out, data_ + pos_, 4);

    pos_ += 4;

    return true;

}



bool PlxStreamReader::readF32(float& out) {

    uint32_t bits = 0;

    if (!readU32(bits)) {

        return false;

    }

    std::memcpy(&out, &bits, 4);

    return true;

}



bool PlxStreamReader::readBytes(size_t n, std::vector<uint8_t>& out) {

    if (pos_ + n > size_) {

        return false;

    }

    out.assign(data_ + pos_, data_ + pos_ + n);

    pos_ += n;

    return true;

}



bool PlxStreamReader::readString(std::string& out) {

    uint32_t n = 0;

    if (!readU32(n)) {

        return false;

    }

    if (n == 0) {

        out.clear();

        return true;

    }

    std::vector<uint8_t> raw;

    if (!readBytes(n, raw)) {

        return false;

    }

    out.assign(reinterpret_cast<const char*>(raw.data()), raw.size());

    return true;

}



bool PlxStreamReader::readWString(std::wstring& out) {

    uint32_t n = 0;

    if (!readU32(n)) {

        return false;

    }

    if (n == 0) {

        out.clear();

        return true;

    }

    std::vector<uint8_t> raw;

    if (!readBytes(static_cast<size_t>(n) * 2, raw)) {

        return false;

    }

    out.resize(n);

    std::memcpy(out.data(), raw.data(), raw.size());

    return true;

}



bool PlxStreamReader::pushCheckpoint() {

    uint32_t delta = 0;

    if (!readU32(delta)) {

        return false;

    }

    checkpoints_.push_back(pos_ + delta);

    return true;

}



bool PlxStreamReader::atCheckpoint() const {

    return !checkpoints_.empty() && pos_ >= checkpoints_.back();

}



void PlxStreamReader::popCheckpoint() {

    if (!checkpoints_.empty()) {

        checkpoints_.pop_back();

    }

}



bool PlxStreamReader::decodeKey(const std::unordered_map<uint32_t, std::string>& map,

                                uint32_t& hashId, std::string& key) {

    uint32_t first = 0;

    if (!readU32(first)) {

        return false;

    }

    if (first == 0) {

        if (!pushCheckpoint()) {

            return false;

        }

        if (!readU32(hashId)) {

            return false;

        }

        std::string inlineKey;

        if (!readString(inlineKey)) {

            return false;

        }

        const auto it = map.find(hashId);

        key = it != map.end() ? it->second

                              : (inlineKey.empty() ? ("hash:" + std::to_string(hashId)) : inlineKey);

        return true;

    }

    hashId = first;

    const auto it = map.find(hashId);

    key = it != map.end() ? it->second : ("hash:" + std::to_string(hashId));

    return true;

}



bool PlxStreamReader::skipBlock() {

    uint32_t n = 0;

    if (!readU32(n)) {

        return false;

    }

    if (pos_ + n > size_) {

        return false;

    }

    pos_ += n;

    return true;

}



uint32_t PlxDocument::plasmaHash(const std::string& key) {

    uint32_t u4 = 0xFFFFFFE5u;

    for (unsigned char b : key) {

        const uint32_t u2 = (u4 * 31u) & 0xFFFFFFFFu;

        u4 = (static_cast<uint32_t>(b) + u2) & 0xFFFFFFFFu;

    }

    return u4;

}



std::unordered_map<uint32_t, std::string> PlxDocument::buildHashMap() {

    std::unordered_map<uint32_t, std::string> map;

    for (const char* k : kKnownKeys) {

        map[plasmaHash(k)] = k;

    }

    return map;

}



bool PlxDocument::loadFromFile(const std::string& path) {

    std::ifstream in(path, std::ios::binary);

    if (!in) {

        return false;

    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), {});

    return loadFromMemory(data.data(), data.size());

}



bool PlxDocument::loadFromMemory(const uint8_t* data, size_t size) {

    widgets_.clear();

    indexTable_.clear();

    blob_.clear();

    root_ = PlxNode{};

    root_.name = "gui_root";

    root_.widgetType = "Widget";

    root_.frameW = 1280.f;

    root_.frameH = 720.f;

    compiledFormat_ = true;

    if (!data || size < 32) {

        return false;

    }

    const uint8_t* p = data;

    const uint8_t* end = data + size;

    if (!parseHeader(data, p, end)) {

        return false;

    }

    return parseTree();

}



bool PlxDocument::parseHeader(const uint8_t* start, const uint8_t*& p, const uint8_t* end) {

    if (!readLeU32(p, end, version_)) {

        return false;

    }

    for (int i = 0; i < 3; ++i) {

        uint32_t skip = 0;

        if (!readLeU32(p, end, skip)) {

            return false;

        }

    }

    if (p + 15 > end) {

        return false;

    }

    if (std::strncmp(reinterpret_cast<const char*>(p), "PlasmaGraphics", 14) != 0) {

        throw InvalidFileFormatException("Expected PlasmaGraphics magic");

    }

    p += 15;



    const uint8_t* wp = start + 47;

    uint32_t count = 0;

    if (!readBeU32(wp, end, count) || count == 0 || count > 4096) {

        return false;

    }

    for (uint32_t i = 0; i < count; ++i) {

        uint32_t nlen = 0;

        if (i == 0) {

            if (!readBeU32(wp, end, nlen)) {

                return false;

            }

            if (nlen <= 8 && wp + 3 <= end && wp[0] == 0 && wp[1] == 0 && wp[2] == 0) {

                wp += 3;

            }

        } else if (!readLeU32(wp, end, nlen)) {

            return false;

        }

        if (nlen > 512 || wp + nlen + 4 > end) {

            return false;

        }

        PlxWidgetDef def;

        def.name.assign(reinterpret_cast<const char*>(wp), nlen);

        wp += nlen;

        if (!readLeU32(wp, end, def.typeId)) {

            return false;

        }

        widgets_.push_back(std::move(def));

    }



    if (!readLeU32(wp, end, count) || count > 100000) {

        return false;

    }

    for (uint32_t i = 0; i < count && wp + 4 <= end; ++i) {

        uint32_t v = 0;

        if (!readLeU32(wp, end, v)) {

            break;

        }

        indexTable_.push_back(v);

    }



    streamOffset_ = static_cast<size_t>(wp - start);

    if (wp < end) {

        blob_.assign(wp, end);

    }

    p = end;

    return true;

}



bool PlxDocument::readWStringAt(size_t pos, std::wstring& out, size_t& nextPos) const {

    if (pos + 4 > blob_.size()) {

        return false;

    }

    uint32_t n = 0;

    std::memcpy(&n, blob_.data() + pos, 4);

    if (n == 0 || n > 256 || pos + 4 + static_cast<size_t>(n) * 2 > blob_.size()) {

        return false;

    }

    out.resize(n);

    std::memcpy(out.data(), blob_.data() + pos + 4, static_cast<size_t>(n) * 2);

    for (wchar_t ch : out) {

        if (ch > 127) {

            continue;

        }

    }

    nextPos = pos + 4 + static_cast<size_t>(n) * 2;

    return true;

}



bool PlxDocument::tryParseAttributeStream() {

    const auto map = buildHashMap();

    PlxStreamReader reader(blob_.data(), blob_.size(), streamOffset_);

    PlxNode* current = &root_;

    int attrs = 0;

    compiledFormat_ = true;

    for (int guard = 0; guard < 10000; ++guard) {

        if (reader.atCheckpoint()) {

            reader.popCheckpoint();

            continue;

        }

        if (reader.eof()) {

            break;

        }

        uint32_t hashId = 0;

        std::string key;

        if (!reader.decodeKey(map, hashId, key)) {

            break;

        }

        if (key == "PlasmaGraphics") {

            compiledFormat_ = false;

            uint32_t ver = 0;

            reader.readU32(ver);

            return true;

        }

        if (key.rfind("hash:", 0) == 0) {

            reader.skipBlock();

            continue;

        }

        ++attrs;

        if (key == "Widget.name" || key == "Attribute.sequence.name" || key == "Texture.name") {

            std::string s;

            reader.readString(s);

            if (key == "Attribute.sequence.name") {

                root_.children.push_back(PlxNode{});

                current = &root_.children.back();

                current->name = s;

            } else if (key == "Texture.name") {

                current->texture = s;

            } else {

                current->name = s;

            }

        } else if (key == "Widget.wname" || key == "Widget.caption" ||

                   key == "Attribute.sequence.wname") {

            std::wstring ws;

            reader.readWString(ws);

            current->wname.assign(ws.begin(), ws.end());

        } else if (key == "Widget.framePos") {

            reader.readF32(current->frameX);

            reader.readF32(current->frameY);

        } else if (key == "Widget.frameSize") {

            reader.readF32(current->frameW);

            reader.readF32(current->frameH);

        } else if (key == "Widget.bindPos") {

            reader.readF32(current->bindX);

            reader.readF32(current->bindY);

        } else if (key == "Widget.bindSize") {

            reader.readF32(current->bindW);

            reader.readF32(current->bindH);

        } else if (key == "Widget.flags") {

            reader.readU32(current->flags);

        } else {

            reader.skipBlock();

        }

        if (reader.atCheckpoint()) {

            reader.popCheckpoint();

        }

    }

    return attrs > 0 && !compiledFormat_;

}



bool PlxDocument::parseCompiledStream() {

    compiledFormat_ = true;

    std::vector<PlxNode> textures;
    const std::vector<uint8_t> needle = {'.', 0, 'p', 0, 'n', 0, 'g', 0};
    std::unordered_set<std::string> seen;
    size_t pos = 0;
    while (pos + needle.size() <= blob_.size()) {
        auto it = std::search(blob_.begin() + static_cast<ptrdiff_t>(pos), blob_.end(),
                              needle.begin(), needle.end());
        if (it == blob_.end()) {
            break;
        }
        const size_t j = static_cast<size_t>(it - blob_.begin());
        size_t start = j >= 4 ? j - 4 : 0;
        const size_t minStart = j > 200 ? j - 200 : 0;
        while (start >= minStart) {
            std::wstring ws;
            size_t next = 0;
            if (readWStringAt(start, ws, next)) {
                std::string ascii(ws.begin(), ws.end());
                if (ascii.size() >= 4 && ascii.compare(ascii.size() - 4, 4, ".png") == 0 &&
                    seen.insert(ascii).second) {
                    PlxNode tex;
                    tex.name = ascii.substr(0, ascii.size() - 4);
                    tex.wname = ascii;
                    tex.texture = ascii;
                    tex.widgetType = "Button";
                    textures.push_back(std::move(tex));
                    break;
                }
            }
            if (start < 2) {
                break;
            }
            start -= 2;
        }
        pos = j + 2;
    }



    if (!widgets_.empty() && widgets_[0].name == "Seal") {

        PlxNode seal;

        seal.name = "Seal";

        seal.widgetType = "Button";

        seal.typeId = widgets_[0].typeId;

        seal.frameX = 100.f;

        seal.frameY = 100.f;

        seal.frameW = static_cast<float>(std::max<size_t>(textures.size(), 1) * 72);

        seal.frameH = 72.f;

        for (size_t i = 0; i < textures.size(); ++i) {

            PlxNode child = textures[i];

            child.frameX = static_cast<float>(i * 72);

            child.frameY = 0.f;

            child.frameW = 64.f;

            child.frameH = 64.f;

            seal.children.push_back(std::move(child));

        }

        root_.children.push_back(std::move(seal));

    }



    if (widgets_.size() > 1 && widgets_[1].typeId == 0) {

        PlxNode custom;

        custom.name = widgets_[1].name.substr(0, 32);

        custom.widgetType = "Widget";

        custom.typeId = 0;

        root_.children.push_back(std::move(custom));

    }



    if (!indexTable_.empty()) {

        root_.flags = indexTable_[0];

    }

    return true;

}



bool PlxDocument::parseTree() {

    if (tryParseAttributeStream()) {

        return true;

    }

    return parseCompiledStream();

}



void PlxDocument::applyNode(const PlxNode& src, Widget& dst, Engine& engine,

                            cube::TextureCatalog* textures) const {

    dst.bounds() = {src.frameX, src.frameY, src.frameW, src.frameH};

    if (!src.texture.empty() && textures) {

        if (Texture* tex = textures->gpuTexture(engine, src.texture)) {

            dst.setBackground(tex);

        }

    }

    if (auto* btn = dynamic_cast<Button*>(&dst)) {

        btn->label = src.name.empty() ? src.wname : src.name;

        if (!src.texture.empty() && textures) {

            if (Texture* tex = textures->gpuTexture(engine, src.texture)) {

                btn->setIcon(tex);

            }

        }

    }

    for (const PlxNode& child : src.children) {

        auto w = createFromNode(child, engine, textures);

        if (w) {

            dst.addChild(w.release());

        }

    }

}



std::unique_ptr<Widget> PlxDocument::createFromNode(const PlxNode& node, Engine& engine,

                                                    cube::TextureCatalog* textures) const {

    const std::string type =

        node.widgetType.empty() ? widgetTypeName(node.typeId) : node.widgetType;

    std::unique_ptr<Widget> w(NamedObject::createWidget(type, node.name));

    applyNode(node, *w, engine, textures);

    return w;

}



std::unique_ptr<Widget> PlxDocument::buildRootWidget(Engine& engine,

                                                     cube::TextureCatalog* textures) const {

    auto root = createFromNode(root_, engine, textures);

    if (!root) {

        root = std::make_unique<Widget>();

        root->setName("gui_root");

        root->bounds() = {0.f, 0.f, 1280.f, 720.f};

    }



    bool hasList = false;

    for (Widget* c : root->children()) {

        if (c && c->name() == "model_list") {

            hasList = true;

            break;

        }

    }

    if (!hasList) {

        auto* list = new ListWidget();

        list->setName("model_list");

        list->bounds() = {8.f, 8.f, 220.f, 400.f};

        root->addChild(list);

    }

    return root;

}



} // namespace plasma


