# include <Node.hpp>
#include <Color.hpp>
#ifndef OPENGL_FRAMEWORK_POINTLIGHTNODE_HPP
#define OPENGL_FRAMEWORK_POINTLIGHTNODE_HPP
class PointLightNode: public Node{
private:
    Color color_;
    float lightIntensity_;

public:
    PointLightNode(std::string name, std::shared_ptr<Node> parent);
    PointLightNode(std::string name, std::shared_ptr<Node> parent, Color color, float lightIntensity);
    Color getColor();
    void setColor(Color color);
    float getLightIntensity();
    void setLightIntensity(float lightIntensity);
};
#endif //OPENGL_FRAMEWORK_POINTLIGHTNODE_HPP