# include "GeometryNode.hpp"


GeometryNode::GeometryNode(std::string name):
    Node(std::move(name)),
    geometry_{model()}
{}

GeometryNode::GeometryNode(const std::shared_ptr<Node> &node, std::string name) :
        Node(std::move(name), node) {}

GeometryNode::~GeometryNode() = default;

model GeometryNode::getGeometry() const{
    return geometry_;
}

void GeometryNode::setGeometry(model const& geometry) {
    geometry_ = geometry;
}
