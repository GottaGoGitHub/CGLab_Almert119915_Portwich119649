#include "Node.hpp"
#include <iostream>

int main() {
    auto ch1 = std::make_shared<Node>(Node("ch1"));
    auto ch2 = std::make_shared<Node>(Node("ch2", ch1));
    auto ch3 = std::make_shared<Node>(Node("ch3", ch2));
    auto root = std::make_shared<Node>(Node("root"));
    ch1->setParent(root);
    std::cout << ch3->getPath() << ", " << ch3->getDepth() << std::endl;
    root->addChildren(ch1);
    ch1->addChildren(ch2);
    ch2->addChildren(ch3);
    std::cout << root->getChildren("ch3")->getName();
    root->removeChildren("ch3");
    root->getChildren("ch3");

}