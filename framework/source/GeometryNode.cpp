# include "GeometryNode.hpp"


GeometryNode::GeometryNode(std::string name):
    Node(name),
    geometry_{model()}
{}

GeometryNode::GeometryNode(const std::shared_ptr<Node> &node, std::string name) :
        Node(name, node) {}

GeometryNode::~GeometryNode() = default;

model GeometryNode::getGeometry() const {
    return geometry_;
}

void GeometryNode::setGeometry(model const &geometry) {
    geometry_ = geometry;
}

model_object GeometryNode::getModelObject() const {
    return modelObject_;
}

void GeometryNode::setModelObject(const model_object &modelObject) {
    modelObject_ = modelObject;
}