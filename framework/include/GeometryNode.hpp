#ifndef OPENGL_FRAMEWORK_GEOMETRYNODE_H
#define OPENGL_FRAMEWORK_GEOMETRYNODE_H

# include "Node.hpp"
#include "model.hpp"
#include "structs.hpp"

class GeometryNode : public Node {
public:
    // constructor
    explicit GeometryNode(std::shared_ptr<Node> node_ptr);

    GeometryNode(std::string name);

    GeometryNode(const std::shared_ptr<Node> &node, std::string name);

    // destructor
    ~GeometryNode();

    // Getter und Setter
    model getGeometry() const;

    void setGeometry(model const &geometry);

    model_object getModelObject() const;

    void setModelObject(model_object const &modelObject);


private:
    // Member
    model geometry_;
    model_object modelObject_;
};


#endif //OPENGL_FRAMEWORK_GEOMETRYNODE_H
