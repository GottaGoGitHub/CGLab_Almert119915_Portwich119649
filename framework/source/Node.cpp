#include "Node.hpp"

Node::Node(std::string const &name) :
        name_{name},
        path_{"/" + name},
        parent_{nullptr} {
    depth_ = 0;
}

Node::Node(const std::string &name, const std::shared_ptr<Node> &parent) :
        name_{name},
        parent_{parent} {
    // recursivly calls path of parents and adds own name
    path_ = getPath();
    depth_ = parent->getDepth() + 1;
}