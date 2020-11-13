#ifndef NODE_HPP
#define NODE_HPP

#include <string>
#include <list>
#include <memory>
#include "glm/glm.hpp"

class Node {

private:
    std::string name_;
    std::string path_;
    int depth_;
    glm::mat4 localTransform_;
    glm::mat4 worldTransform_;
    std::shared_ptr<Node> parent_;
    std::list<std::shared_ptr<Node>> children_;

public:
    // constructor
    Node(std::string const &name);

    Node(std::string const &name, std::shared_ptr<Node> const &parent);

    // destructor
    ~Node();

    std::shared_ptr<Node> getParent();

    void setParent(std::shared_ptr<Node> const &node);

    std::shared_ptr<Node> getChildren();

    std::shared_ptr<Node> removeChildren(std::string const &name);

    std::list<Node> getChildrenList();

    void addChildren(std::shared_ptr<Node> const &node);

    std::string getName();

    std::string getPath();

    int getDepth();

    glm::mat4 getLocalTransform();

    void setLocalTransform(glm::mat4 const &mat);

    glm::mat4 getWorldTransform();

    void setWorldTransform(glm::mat4 const &mat);
};

#endif //NODE_HPP
