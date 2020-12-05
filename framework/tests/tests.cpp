#include "Node.hpp"
#include "SceneGraph.hpp"
#include <iostream>
#include <GeometryNode.hpp>

int main() {
    std::shared_ptr<Node> root = std::make_shared<Node>("root");
    auto solar_system = SceneGraph("solarSystem", root);

    // sun
    std::shared_ptr<Node> sun_holder = std::make_shared<Node>("sun", root);
    std::shared_ptr<GeometryNode> geo_sun = std::make_shared<GeometryNode>(sun_holder, "geo_sun");
    root->addChildren(sun_holder);
    sun_holder->setDistance(0.0f);
    sun_holder->setSize(7.0f);
    sun_holder->addChildren(geo_sun);

    // merkur
    std::shared_ptr<Node> merkur_holder = std::make_shared<Node>("merkur", root);
    std::shared_ptr<GeometryNode> geo_merkur = std::make_shared<GeometryNode>(merkur_holder, "geo_merkur");
    merkur_holder->setSpeed(4.147f);
    merkur_holder->setDistance(5.0f + sun_holder->getSize());
    merkur_holder->setSize(0.38f);
    root->addChildren(merkur_holder);
    merkur_holder->addChildren(geo_merkur);

    // venus
    std::shared_ptr<Node> venus_holder = std::make_shared<Node>("venus", root);
    std::shared_ptr<GeometryNode> geo_venus = std::make_shared<GeometryNode>(merkur_holder, "geo_venus");
    venus_holder->setSpeed(2.624f);
    venus_holder->setDistance(9.31f + sun_holder->getSize());
    venus_holder->setSize(0.94f);
    root->addChildren(venus_holder);
    venus_holder->addChildren(geo_venus);

    // earth planet
    std::shared_ptr<Node> earth_holder = std::make_shared<Node>("earth", root);
    std::shared_ptr<GeometryNode> geo_earth = std::make_shared<GeometryNode>(earth_holder, "geo_earth");
    earth_holder->setSpeed(1.0f);
    earth_holder->setDistance(12.93f + sun_holder->getSize());
    earth_holder->setSize(1.0f);
    root->addChildren(earth_holder);
    earth_holder->addChildren(geo_earth);


    // moon
    std::shared_ptr<Node> moon_holder = std::make_shared<Node>("moon", earth_holder);
    std::shared_ptr<GeometryNode> geo_moon = std::make_shared<GeometryNode>(earth_holder, "geo_moon");
    moon_holder->setSpeed(0.5f);
    moon_holder->setDistance(0.4f + earth_holder->getSize());
    moon_holder->setSize(0.27f);
    earth_holder->addChildren(moon_holder);
    moon_holder->addChildren(geo_moon);

    std::shared_ptr<Node> camera = std::make_shared<Node>("camera", root);
    root->addChildren(camera);


    auto peter = root->getDrawable();

    int c = 2;
}