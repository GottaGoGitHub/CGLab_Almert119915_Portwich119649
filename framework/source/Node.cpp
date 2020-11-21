#include "Node.hpp"

#include <utility>
#include <iostream>

Node::Node() {}

Node::Node(std::string name) :
        name_(std::move(name)), // move semantic
        path_("/" + name_),
        parent_(nullptr) {
    depth_ = 0;
    localTransform_ = glm::mat4(1.0f);
    worldTransform_ = glm::mat4(1.0f);
}

Node::Node(std::string name, const std::shared_ptr<Node> &parent) :
        name_(std::move(name)), // move semantic
        parent_(parent) {
    path_ = getPath();
    depth_ = parent->getDepth() + 1;
    localTransform_ = glm::mat4(1.0f);
    worldTransform_ = glm::mat4(1.0f);
}

Node::~Node() = default;

std::string Node::getName() {
    return name_;
}

int Node::getDepth() {
    // depth = 0 if no parent
    if (parent_ == nullptr)
        return 0;
    else {
        // get path recursively from parents
        depth_ = parent_->getDepth() + 1;
        return depth_;
    }
}

std::string Node::getPath() {
    // if there is no parent
    if (parent_ == nullptr)
        return "/" + name_;
    else {
        // get path recursively from all parents
        path_ = parent_->getPath() + "/" + name_;
        return path_;
    }
}

void Node::setParent(const std::shared_ptr<Node> &parent) {
    parent_ = parent;
    depth_ = parent_->getDepth() + 1;
    path_ = parent->getPath() + "/" + name_;
}

std::shared_ptr<Node> Node::getParent() {
    return parent_;
}

void Node::addChildren(const std::shared_ptr<Node> &node) {
    children_.push_back(node);
}

std::shared_ptr<Node> Node::removeChildren(const std::string &name) {
    // search for child recursively
    auto found_child = this->getChildren(name);
    // if child was found
    if (found_child != nullptr) {
        // get parent to access childrenList
        auto parent_found_child = found_child->getParent();
        // if parent was found
        if (parent_found_child != nullptr)
            // remove child from the parents childrenList
            parent_found_child->children_.remove(found_child);
    }
    return found_child;
}

std::shared_ptr<Node> Node::getChildren(std::string const &name) {
    // iterate over childrenList
    for (auto child : children_) {
        // if direct child
        if (child->name_ == name)
            return child;
        else {
            // search recursively
            std::shared_ptr<Node> next_child = child->getChildren(name);
            if (next_child != nullptr)
                return child->getChildren(name);
        }
    }
    // nothing found :(
    return nullptr;
}

void Node::printChildren(int level) {
    if (level == 0)
        std::cout << "." << "- " << this->getName() << std::endl;
    else
        std::cout << std::string(level, '|') << "- " << this->getName() << std::endl;
    for (auto child: children_) {
        child->printChildren(level + 1);
    }
}

std::list<std::shared_ptr<Node>> Node::getChildrenList() {
    return children_;
}

void Node::setLocalTransform(const glm::mat4 &mat) {
    localTransform_ = mat;
}

glm::mat4 Node::getLocalTransform() {
    return localTransform_;
}

void Node::setWorldTransform(const glm::mat4 &mat) {
    worldTransform_ = mat;
}

glm::mat4 Node::getWorldTransform() {
    return worldTransform_;
}