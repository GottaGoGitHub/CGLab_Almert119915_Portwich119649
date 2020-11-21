#ifndef OPENGL_FRAMEWORK_GEOMETRYNODE_H
#define OPENGL_FRAMEWORK_GEOMETRYNODE_H

# include "Node.hpp"
#include "model.hpp"
#include "structs.hpp"

class GeometryNode : public Node {
public:
    // constructor

    explicit GeometryNode(std::string name);

    GeometryNode(const std::shared_ptr<Node> &parent, std::string name);

    // destructor
    ~GeometryNode();

    // Getter und Setter
    model getGeometry() const;

    void setGeometry(model const &geometry);


private:
    // Member
    model geometry_;
};


#endif //OPENGL_FRAMEWORK_GEOMETRYNODE_H
