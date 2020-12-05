#include "Node.hpp"

#include <utility>
#include <iostream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

Node::Node() {}

Node::Node(std::string name) :
        name_(std::move(name)), // move semantic
        path_("/" + name_),
        parent_(nullptr) {
    depth_ = 0;
    localTransform_ = glm::mat4(1.0f);
    worldTransform_ = glm::mat4(1.0f);
    speed_ = 1.0f;
    size_ = 1.0f;
    distance_ = 0.0f;
}

Node::Node(std::string name, const std::shared_ptr<Node> &parent) :
        name_(std::move(name)), // move semantic
        parent_(parent) {
    path_ = getPath();
    depth_ = parent->getDepth() + 1;
    localTransform_ = glm::mat4(1.0f);
    worldTransform_ = glm::mat4(1.0f);
    speed_ = 1.0f;
    size_ = 1.0f;
    distance_ = 0.0f;
}

Node::~Node() = default;

//setter

// set parent and refresh depth and path
void Node::setParent(const std::shared_ptr<Node> &parent) {
    parent_ = parent;
    depth_ = parent_->getDepth() + 1;
    path_ = parent->getPath() + "/" + name_;
}

void Node::setLocalTransform(const glm::mat4 &mat) {
    localTransform_ = mat;
}

void Node::setWorldTransform(const glm::mat4 &mat) {
    worldTransform_ = mat;
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

void Node::setDistance(float distance) {
    distance_ = distance;
    localTransform_ = glm::translate(localTransform_, glm::fvec3{0.0f, 0.0f, distance});
}

void Node::setSpeed(float speed) {
    speed_ = speed;
}

void Node::setSize(float size) {
    size_ = size;
}


// getter

std::list<std::shared_ptr<Node>> Node::getChildrenList() {
    return children_;
}

glm::mat4 Node::getLocalTransform() {
    return localTransform_;
}

glm::mat4 Node::getWorldTransform() {
    return worldTransform_;
}

float Node::getSpeed() const {
    return speed_;
}

float Node::getDistance() const {
    return distance_;
}

float Node::getSize() const {
    return size_;
}

// search recursively trough all child for a child with given name and return it
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

std::vector<std::shared_ptr<Node>> Node::getDrawable() {
    std::vector<std::shared_ptr<Node>> drawable;
    for(auto child : children_){
        if(child->name_.find("camera") == std::string::npos && child->name_.find("geo") == std::string::npos){
            drawable.push_back(child);
            if(not child->getChildrenList().empty()){
                auto child_drawable = child->getDrawable();
                drawable.insert(drawable.end(), child_drawable.begin(), child_drawable.end());
            }
        }
    }
    return drawable;
}

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

std::shared_ptr<Node> Node::getParent() {
    return parent_;
}

// functions

void Node::printChildren(int level) {
    if (level == 0)
        std::cout << "." << "- " << this->getName() << std::endl;
    else
        std::cout << std::string(level, '|') << "- " << this->getName() << std::endl;
    for (const auto& child: children_) {
        child->printChildren(level + 1);
    }
}