#include "PointLightNode.hpp"

PointLightNode::PointLightNode(std::string name, std::shared_ptr<Node> parent):
    Node(std::move(name), parent){}

PointLightNode::PointLightNode(std::string name, std::shared_ptr<Node> parent, color color, float lightIntensity):
    Node(std::move(name), parent),
    color_{color},
    lightIntensity_{lightIntensity}{}

color PointLightNode::getColor() {
    return color_;
}

float PointLightNode::getLightIntensity() {
    return lightIntensity_;
}

void PointLightNode::setColor(color color) {
    color_ = color;
}

void PointLightNode::setLightIntensity(float lightIntensity) {
    lightIntensity_ = lightIntensity;
}
