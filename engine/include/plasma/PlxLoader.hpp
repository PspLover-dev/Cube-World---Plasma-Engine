#pragma once



#include "plasma/Widget.hpp"



#include <cstdint>

#include <memory>

#include <string>

#include <unordered_map>

#include <vector>



namespace cube {

class TextureCatalog;

}



namespace plasma {



struct PlxWidgetDef {

    std::string name;

    uint32_t typeId{0};

};



struct PlxNode {

    std::string name;

    std::string wname;

    std::string caption;

    std::string widgetType{"Widget"};

    uint32_t typeId{0};

    std::string texture;

    float frameX{0.f};

    float frameY{0.f};

    float frameW{64.f};

    float frameH{64.f};

    float bindX{0.f};

    float bindY{0.f};

    float bindW{0.f};

    float bindH{0.f};

    uint32_t flags{0};

    std::vector<PlxNode> children;

};



class PlxStreamReader {

public:

    PlxStreamReader(const uint8_t* data, size_t size, size_t baseOffset = 0);



    size_t tell() const;

    bool eof() const;



    bool readU32(uint32_t& out);

    bool readF32(float& out);

    bool readBytes(size_t n, std::vector<uint8_t>& out);

    bool readString(std::string& out);

    bool readWString(std::wstring& out);



    bool pushCheckpoint();

    bool atCheckpoint() const;

    void popCheckpoint();



    bool decodeKey(const std::unordered_map<uint32_t, std::string>& map, uint32_t& hashId,

                   std::string& key);

    bool skipBlock();



private:

    const uint8_t* data_;

    size_t size_;

    size_t base_;

    size_t pos_{0};

    std::vector<size_t> checkpoints_;

};



class PlxDocument {

public:

    bool loadFromFile(const std::string& path);

    bool loadFromMemory(const uint8_t* data, size_t size);



    uint32_t version() const { return version_; }

    const std::vector<PlxWidgetDef>& widgets() const { return widgets_; }

    const std::vector<uint8_t>& blob() const { return blob_; }

    const std::vector<uint32_t>& indexTable() const { return indexTable_; }

    const PlxNode& root() const { return root_; }

    bool isCompiledFormat() const { return compiledFormat_; }



    std::unique_ptr<Widget> buildRootWidget(Engine& engine,

                                            cube::TextureCatalog* textures = nullptr) const;



private:

    static uint32_t plasmaHash(const std::string& key);

    static std::unordered_map<uint32_t, std::string> buildHashMap();



    bool parseHeader(const uint8_t* start, const uint8_t*& p, const uint8_t* end);

    bool parseTree();

    bool tryParseAttributeStream();

    bool parseCompiledStream();

    bool readWStringAt(size_t pos, std::wstring& out, size_t& nextPos) const;



    void applyNode(const PlxNode& src, Widget& dst, Engine& engine,

                   cube::TextureCatalog* textures) const;

    std::unique_ptr<Widget> createFromNode(const PlxNode& node, Engine& engine,

                                           cube::TextureCatalog* textures) const;



    uint32_t version_{0};

    std::vector<PlxWidgetDef> widgets_;

    std::vector<uint32_t> indexTable_;

    std::vector<uint8_t> blob_;

    size_t streamOffset_{0};

    PlxNode root_;

    bool compiledFormat_{true};

};



} // namespace plasma


