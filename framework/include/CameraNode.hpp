#ifndef OPENGL_FRAMEWORK_CAMERANODE_HPP
#define OPENGL_FRAMEWORK_CAMERANODE_HPP

#include "Node.hpp"
#include "glm/glm.hpp"

class CameraNode : public Node {
private:
    bool isPerspective_;
    bool isEnabled_;
    glm::mat4 projectionMatrix_;

public:
    // constructors
    CameraNode();

    CameraNode(bool isPerspective, bool isEnabled);

    CameraNode(bool isPerspective, bool isEnabled, glm::mat4 const &projectionMatrix);

    // getter and setter
    bool getPerspective();

    void setPerspective(bool perspective);

    bool getEnabled();

    void setEnabled(bool enabled);

    glm::mat4 getProjectionMatrix();

    void setProjectionMatrix(glm::mat4 const &mat);
};


#endif //OPENGL_FRAMEWORK_CAMERANODE_HPP
