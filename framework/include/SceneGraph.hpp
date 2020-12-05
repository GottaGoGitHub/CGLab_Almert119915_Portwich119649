#ifndef OPENGL_FRAMEWORK_SCENEGRAPH_HPP
#define OPENGL_FRAMEWORK_SCENEGRAPH_HPP

#include "Node.hpp"

class SceneGraph {
private:
    std::string name_;
    std::shared_ptr<Node> root_;

    void setName(std::string const &name);

    void setRoot(std::shared_ptr<Node> const &node);

public:
    SceneGraph() = default;

    SceneGraph(std::string name);

    SceneGraph(std::string name, std::shared_ptr<Node> node);

    std::string getName();

    std::shared_ptr<Node> getRoot() const;

    std::string printGraph();
};


#endif //OPENGL_FRAMEWORK_SCENEGRAPH_HPP
