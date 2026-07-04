#pragma once

#include <memory>
#include <string>
#include <vector>

namespace cube {

class QuestTextNode {
public:
    QuestTextNode() = default;
    explicit QuestTextNode(std::string text);

    const std::string& text() const { return text_; }
    void setText(std::string text) { text_ = std::move(text); }

    QuestTextNode* addChild(std::string text);
    const std::vector<std::unique_ptr<QuestTextNode>>& children() const { return children_; }

private:
    std::string text_;
    std::vector<std::unique_ptr<QuestTextNode>> children_;
};

class QuestText {
public:
    QuestText() = default;

    QuestTextNode& root() { return root_; }
    const QuestTextNode& root() const { return root_; }

    std::string format(const QuestTextNode& node) const;

private:
    QuestTextNode root_;
};

class Speech {
public:
    Speech() = default;

    void setSpeaker(std::string name) { speaker_ = std::move(name); }
    void setLine(std::string line) { line_ = std::move(line); }

    const std::string& speaker() const { return speaker_; }
    const std::string& line() const { return line_; }

private:
    std::string speaker_;
    std::string line_;
};

} // namespace cube
