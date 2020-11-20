# include "GeometryNode.hpp"


GeometryNode::GeometryNode(std::string name):
    Node(name),
    geometry_{model()}
{}

GeometryNode::GeometryNode(std::string name, const std::shared_ptr<Node> &parent, model geometry):
    Node(name, parent),
    geometry_(geometry)
{}

GeometryNode::~GeometryNode() = default;

model GeometryNode::getGeometry() const{
    return geometry_;
}

void GeometryNode::setGeometry(model const& geometry) {
    geometry_ = geometry;
}