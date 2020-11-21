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
    float speed_;
    float distance_;
    float size_;


public:
    // constructor
    Node();

    explicit Node(std::string name);

    Node(std::string name, std::shared_ptr<Node> const &parent);

    // destructor
    ~Node();

    // return parent node
    std::shared_ptr<Node> getParent();

    // set parent and update depth & path
    void setParent(std::shared_ptr<Node> const &parent);

    std::shared_ptr<Node> getChildren(std::string const &name);

    std::shared_ptr<Node> removeChildren(std::string const &name);

    std::list<std::shared_ptr<Node>> getChildrenList();

    void addChildren(std::shared_ptr<Node> const &node);

    void printChildren(int level = 0);

    // return name
    std::string getName();

    // return path
    std::string getPath();

    // return depth
    int getDepth();

    glm::mat4 getLocalTransform();

    void setLocalTransform(glm::mat4 const &mat);

    glm::mat4 getWorldTransform();

    void setWorldTransform(glm::mat4 const &mat);

    // getter
    float getSpeed() const;
    float getDistance() const;
    float getSize() const;

    // setter
    void setSpeed(float speed);
    void setDistance(float distance);
    void setSize(float size);

};

#endif //NODE_HPP
