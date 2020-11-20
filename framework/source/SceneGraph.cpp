#include "SceneGraph.hpp"
#include <utility>
#include <iostream>

SceneGraph::SceneGraph(std::string name) :
        name_(std::move(name)) {
}

SceneGraph::SceneGraph(std::string name, std::shared_ptr<Node> node) :
        name_(std::move(name)),
        root_(std::move(node)) {
}

std::string SceneGraph::getName() {
    return name_;
}

std::shared_ptr<Node> SceneGraph::getRoot() {
    return root_;
}

void SceneGraph::setName(const std::string &name) {
    name_ = name;
}

void SceneGraph::setRoot(const std::shared_ptr<Node> &node) {
    root_ = node;
}

std::string SceneGraph::printGraph() {
    root_->printChildren();
}