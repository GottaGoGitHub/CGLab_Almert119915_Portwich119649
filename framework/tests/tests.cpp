#include "Node.hpp"
#include "SceneGraph.hpp"
#include <iostream>
#include <GeometryNode.hpp>

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
    std::cout << root->getChildren("ch3")->getName() << std::endl;
    auto ch12 = std::make_shared<Node>(Node("ch12", root));
    auto ch22 = std::make_shared<Node>(Node("ch22", ch1));
    root->addChildren(ch12);
    ch1->addChildren(ch22);
    auto szene = std::make_shared<SceneGraph>(SceneGraph("szene", root));
    szene->printGraph();

    auto rooty = std::make_shared<Node>("root");
    SceneGraph solar = SceneGraph("solar", rooty);
    auto gm1 = std::make_shared<GeometryNode>("geo1");
    rooty->addChildren(gm1);
    std::shared_ptr<GeometryNode> xe = std::static_pointer_cast<GeometryNode>(rooty->getChildren("geo1"));
    std::cout << xe->getName();
    std::cout << xe->getPath();

}