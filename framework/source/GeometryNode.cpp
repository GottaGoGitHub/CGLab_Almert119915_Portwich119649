# include "GeometryNode.hpp"


GeometryNode::GeometryNode(std::string name):
    Node(std::move(name)),
    geometry_{model()}
{}

GeometryNode::GeometryNode(const std::shared_ptr<Node> &parent, std::string name) :
        Node(std::move(name)) {
    this->setParent(parent);
}

GeometryNode::GeometryNode(const std::shared_ptr<Node> &parent, std::string name, model geometry) :
        Node(std::move(name)) {
    this->setParent(parent);
    this->geometry_ = geometry;
}

GeometryNode::~GeometryNode() = default;

model GeometryNode::getGeometry() const{
    return geometry_;
}

void GeometryNode::setGeometry(model const& geometry) {
    geometry_ = geometry;
}
