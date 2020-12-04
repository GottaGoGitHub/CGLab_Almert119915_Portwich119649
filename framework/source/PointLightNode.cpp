#include "PointLightNode.hpp"

PointLightNode::PointLightNode(std::string name, std::shared_ptr<Node> parent):
    Node(std::move(name), parent){}

PointLightNode::PointLightNode(std::string name, std::shared_ptr<Node> parent, Color color, float lightIntensity):
    Node(std::move(name), parent),
    color_{color},
    lightIntensity_{lightIntensity}{}

Color PointLightNode::getColor() {
    return color_;
}

float PointLightNode::getLightIntensity() {
    return lightIntensity_;
}

void PointLightNode::setColor(Color color) {
    color_ = color;
}

void PointLightNode::setLightIntensity(float lightIntensity) {
    lightIntensity_ = lightIntensity;
}
