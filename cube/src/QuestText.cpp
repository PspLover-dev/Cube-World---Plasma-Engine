#include "cube/QuestText.hpp"

namespace cube {

QuestTextNode::QuestTextNode(std::string text) : text_(std::move(text)) {}

QuestTextNode* QuestTextNode::addChild(std::string text) {
    children_.push_back(std::make_unique<QuestTextNode>(std::move(text)));
    return children_.back().get();
}

std::string QuestText::format(const QuestTextNode& node) const {
    return node.text();
}

} // namespace cube
