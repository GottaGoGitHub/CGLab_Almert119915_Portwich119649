#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <memory>
#include <Node.hpp>
#include <SceneGraph.hpp>
#include <GeometryNode.hpp>

ApplicationSolar::ApplicationSolar(std::string const &resource_path)
        : Application{resource_path}, planet_object{}, star_object{},
          m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})},
          m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)},
          solar_system_{} {
    initializeGeometry();
    initializeShaderPrograms();
    initializeSolarSystem();
    initializeStarsGeometry();
}

ApplicationSolar::~ApplicationSolar() {
    glDeleteBuffers(1, &planet_object.vertex_BO);
    glDeleteBuffers(1, &planet_object.element_BO);
    glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

void ApplicationSolar::render() const {

    // bind shader to upload uniforms
    glUseProgram(m_shaders.at("planet").handle);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto children = solar_system_.getRoot()->getDrawable();
//    auto children = solar_system_.getRoot()->getChildrenList();
//    // Adding moons to solarSystem
//    children.push_back(solar_system_.getRoot()->getChildren("moon"));

    // iteration through all planets and moons
    for (auto child: children) {
            auto parent = child->getParent();

            child->setLocalTransform(glm::rotate(parent->getLocalTransform(), float(glfwGetTime() * child->getSpeed()),
                                                 glm::fvec3{0.0f, 1.0f, 0.0f}));

            child->setLocalTransform(
                    glm::translate(child->getLocalTransform(), glm::fvec3{0.0f, 0.0f, child->getDistance()}));

            child->setLocalTransform(glm::scale(child->getLocalTransform(),
                                                glm::vec3(child->getSize(), child->getSize(), child->getSize())));


            auto model_mat = child->getLocalTransform();
            glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                               1, GL_FALSE, glm::value_ptr(model_mat));

            auto normal_mat = glm::inverseTranspose(glm::inverse(m_view_transform) * child->getLocalTransform());
            glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                               1, GL_FALSE, glm::value_ptr(normal_mat));

            // bind the VAO to draw
            glBindVertexArray(planet_object.vertex_AO);

            // draw bound vertex array using bound shader
            glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);


    }

    glUseProgram(m_shaders.at("star").handle);
    // bind the VAO to draw
    glBindVertexArray(star_object.vertex_AO);
    // draw bound vertex array using bound shader
    glDrawArrays(star_object.draw_mode, GLint(0), star_object.num_elements);


}

void ApplicationSolar::uploadView() {
    // vertices are transformed in camera space, so camera transform must be inverted
    glm::fmat4 view_matrix = glm::inverse(m_view_transform);

    glUseProgram(m_shaders.at("star").handle);

    glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
                       1, GL_FALSE, glm::value_ptr(view_matrix));

    glUseProgram(m_shaders.at("planet").handle);
    // upload matrix to gpu
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                       1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
    // bind shader to which to upload unforms
    glUseProgram(m_shaders.at("star").handle);

    // upload matrix to gpu
    glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                       1, GL_FALSE, glm::value_ptr(m_view_projection));

    glUseProgram(m_shaders.at("planet").handle);

    // upload matrix to gpu
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                       1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
    // bind shader to which to upload unforms
    glUseProgram(m_shaders.at("planet").handle);
    // upload uniform values to new locations
    uploadView();
    uploadProjection();
}

void ApplicationSolar::initializeOrbits() {
    auto drawables = solar_system_.getRoot()->getDrawable();
    float angle = 0.1f;
    int num_points = 65;

    std::vector<GLfloat> orbit_points;
    for (auto object: drawables) {
        orbit_points.clear();
        auto rot_mat = glm::rotate(glm::mat4x4{}, angle, glm::fvec3{0.0f, 1.0f, 0.0f});
        auto point = object->getLocalTransform() * glm::fvec4{0.0f, 0.0f, 0.0f, 1.0f};
        for (int i = 0; i < num_points; ++i) {
            orbit_points.push_back(point.x);
            orbit_points.push_back(point.y);
            orbit_points.push_back(point.z);
            point = rot_mat * point;
        }
    }
}


///////////////////////////// intialisation functions /////////////////////////
void ApplicationSolar::initializeSolarSystem() {
    // scenegraph with root
    std::shared_ptr<Node> root = std::make_shared<Node>("root");
    solar_system_ = SceneGraph("solarSystem", root);

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

    // mars
    std::shared_ptr<Node> mars_holder = std::make_shared<Node>("mars", root);
    std::shared_ptr<GeometryNode> geo_mars = std::make_shared<GeometryNode>(mars_holder, "geo_mars");
    mars_holder->setSpeed(0.831f);
    mars_holder->setDistance(19.65f + sun_holder->getSize());
    mars_holder->setSize(0.53f);
    root->addChildren(mars_holder);
    mars_holder->addChildren(geo_mars);

    // jupiter
    std::shared_ptr<Node> jupiter_holder = std::make_shared<Node>("jupiter", root);
    std::shared_ptr<GeometryNode> geo_jupiter = std::make_shared<GeometryNode>(jupiter_holder, "geo_jupiter");
    jupiter_holder->setSpeed(0.943f);
    jupiter_holder->setDistance(30.0f + sun_holder->getSize()); //67.068
    jupiter_holder->setSize(4.0f);
    root->addChildren(jupiter_holder);
    jupiter_holder->addChildren(geo_jupiter);

    // saturn
    std::shared_ptr<Node> saturn_holder = std::make_shared<Node>("saturn", root);
    std::shared_ptr<GeometryNode> geo_saturn = std::make_shared<GeometryNode>(saturn_holder, "geo_saturn");
    saturn_holder->setSpeed(0.74f);
    saturn_holder->setDistance(38.0f + sun_holder->getSize()); // 123.017
    saturn_holder->setSize(3.0f);
    root->addChildren(saturn_holder);
    saturn_holder->addChildren(geo_saturn);

    // uranus
    std::shared_ptr<Node> uranus_holder = std::make_shared<Node>("uranus", root);
    std::shared_ptr<GeometryNode> geo_uranus = std::make_shared<GeometryNode>(uranus_holder, "geo_uranus");
    uranus_holder->setSpeed(0.65f);
    uranus_holder->setDistance(50.0f + sun_holder->getSize()); //248.620
    uranus_holder->setSize(2.0f);
    root->addChildren(uranus_holder);
    uranus_holder->addChildren(geo_uranus);

    // neptun
    std::shared_ptr<Node> neptun_holder = std::make_shared<Node>("neptun", root);
    std::shared_ptr<GeometryNode> geo_neptun = std::make_shared<GeometryNode>(saturn_holder, "geo_neptun");
    neptun_holder->setSpeed(0.607f);
    neptun_holder->setDistance(60.0f + sun_holder->getSize()); //388.706
    neptun_holder->setSize(2.0f);
    root->addChildren(neptun_holder);
    neptun_holder->addChildren(geo_neptun);
}

// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
    // store shader program objects in container
    m_shaders.emplace("planet", shader_program{{{GL_VERTEX_SHADER, m_resource_path + "shaders/simple.vert"},
                                                       {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}}});
    // request uniform locations for shader program
    m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
    m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
    m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
    m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;

    m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER, m_resource_path + "shaders/vao.vert"},
                                                     {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});

    // request uniform locations for shader program
    m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
    m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;
}

void ApplicationSolar::initializeStarsGeometry() {
    int const numberStars = 5000;
    std::vector<GLfloat> stars; //= std::vector<GLfloat>(6 * numberStars * sizeof(float));
    stars.reserve(6 * numberStars * sizeof(float));
    std::cout << stars.size();

    for (int i = 0; i < numberStars; i++) {
        float rand_x = static_cast<float>(std::rand() % 100) - 50.0f;
        stars.push_back(rand_x);
        float rand_y = static_cast<float>(std::rand() % 100) - 50.0f;
        stars.push_back(rand_y);
        float rand_z = static_cast<float>(std::rand() % 100) - 50.0f;
        stars.push_back(rand_z);
        float rand_r = static_cast<float>(std::rand() % 255) / 255.0f;
        stars.push_back(rand_r);
        float rand_g = static_cast<float>(std::rand() % 255) / 255.0f;
        stars.push_back(rand_g);
        float rand_b = static_cast<float>(std::rand() % 255) / 255.0f;
        stars.push_back(rand_b);
    }

    glGenVertexArrays(1, &star_object.vertex_AO);
    glBindVertexArray(star_object.vertex_AO);

    glGenBuffers(1, &star_object.vertex_BO);
    glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * stars.size(), stars.data(), GL_STATIC_DRAW);

    // first attribArray for positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(6 * sizeof(float)), 0);

    // second attribArray for colors
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GLsizei(6 * sizeof(float)), (void *) (sizeof(float) * 3));
    //the index of the vertexattribarray corresponds to layout position

    star_object.draw_mode = GL_POINTS;
    star_object.num_elements = GLsizei(numberStars);
}

// load models
void ApplicationSolar::initializeGeometry() {
    model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

    // generate vertex array object
    glGenVertexArrays(1, &planet_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(planet_object.vertex_AO);

    // generate generic buffer
    glGenBuffers(1, &planet_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes,
                          planet_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes,
                          planet_model.offsets[model::NORMAL]);

    // generate generic buffer
    glGenBuffers(1, &planet_object.element_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(),
                 GL_STATIC_DRAW);

    // store type of primitive to draw
    planet_object.draw_mode = GL_TRIANGLES;
    // transfer number of indices to model object
    planet_object.num_elements = GLsizei(planet_model.indices.size());
}

///////////////////////////// callback functions for window events ////////////
// handle key input
void ApplicationSolar::keyCallback(int key, int action, int mods) {
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -1.0f});
        uploadView();
    } else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 1.0f});
        uploadView();
    } else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        m_view_transform = glm::translate(m_view_transform, glm::fvec3{-1.0f, 0.0f, 0.0f});
        uploadView();
    } else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        m_view_transform = glm::translate(m_view_transform, glm::fvec3{1.0f, 0.0f, 0.0f});
        uploadView();
    } else if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        m_view_transform = glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f});
        uploadView();
    } else if (key == GLFW_KEY_U && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        m_view_transform = glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 50.0f, 0.0f});
        uploadView();
    }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
    // mouse handling
    float mouse_sens = 20.0f;
    float angle_pan = -pos_x / mouse_sens;
    float angle_tilt = -pos_y / mouse_sens;

// use the higher value and apply the rotation around this value
    if (std::abs(angle_pan) > std::abs(angle_tilt)) {
        m_view_transform = glm::rotate(m_view_transform, glm::radians(angle_pan), glm::vec3{0, 1, 0});
    } else {
        m_view_transform = glm::rotate(m_view_transform, glm::radians(angle_tilt), glm::vec3{1, 0, 0});
    }

    // update ViewMatrix
    uploadView();
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
    // recalculate projection matrix for new aspect ration
    m_view_projection = utils::calculate_projection_matrix(float(width) / float(height));
    // upload new projection matrix
    uploadProjection();
}


// exe entry point
int main(int argc, char *argv[]) {
    Application::run<ApplicationSolar>(argc, argv, 3, 2);
}