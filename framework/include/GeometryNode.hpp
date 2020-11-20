#ifndef OPENGL_FRAMEWORK_GEOMETRYNODE_H
#define OPENGL_FRAMEWORK_GEOMETRYNODE_H

# include "Node.hpp"
#include "model.hpp"

class GeometryNode : public Node {
    public:
    // constructor
    explicit GeometryNode(std::string name);
    GeometryNode(std::string name, std::shared_ptr<Node> const &parent, model geometry);

    // destructor
    ~GeometryNode();

    // Getter und Setter
    model getGeometry() const;
    void setGeometry(model const& geometry);

    private:
    // Member
    model geometry_;
};


#endif //OPENGL_FRAMEWORK_GEOMETRYNODE_H
