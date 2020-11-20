#include "CameraNode.hpp"

CameraNode::CameraNode(bool isPerspective, bool isEnabled) :
        Node(),
        isPerspective_{isPerspective},
        isEnabled_{isPerspective} {};

CameraNode::CameraNode() :
        Node() {};

CameraNode::CameraNode(bool isPerspective, bool isEnabled, const glm::mat4 &projectionMatrix) :
        Node(),
        isPerspective_(isPerspective),
        isEnabled_(isEnabled),
        projectionMatrix_(projectionMatrix) {};

void CameraNode::setProjectionMatrix(const glm::mat4 &mat) {
    projectionMatrix_ = mat;
}

glm::mat4 CameraNode::getProjectionMatrix() {
    return projectionMatrix_;
}

void CameraNode::setEnabled(bool enabled) {
    isEnabled_ = enabled;
}

bool CameraNode::getEnabled() {
    return isEnabled_;
}

bool CameraNode::getPerspective() {
    return isPerspective_;
}

void CameraNode::setPerspective(bool perspective) {
    isPerspective_ = perspective;
}
