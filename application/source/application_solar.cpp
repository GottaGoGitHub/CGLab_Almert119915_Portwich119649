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
#include <Color.hpp>
#include <PointLightNode.hpp>
#include <pixel_data.hpp>
#include <texture_loader.hpp>

ApplicationSolar::ApplicationSolar(std::string const &resource_path)
        : Application{resource_path}, planet_object{}, star_object{}, skybox_object{},
          m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})},
          m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)},
          solar_system_{},
          current_planet_shader_{"planet"}, color_map{}, texture_map{} {
    initializeGeometry();
    initializeShaderPrograms();
    initializeSolarSystem();
    initializeTextures();
    initializeStarsGeometry();
    initializeOrbits();
}

ApplicationSolar::~ApplicationSolar() {
    glDeleteBuffers(1, &planet_object.vertex_BO);
    glDeleteBuffers(1, &planet_object.element_BO);
    glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

void ApplicationSolar::render() const {
    renderSkybox();
    renderPlanets();
    renderStars();
    renderOrbits();
}

void ApplicationSolar::renderPlanets() const {
    auto children = solar_system_.getRoot()->getDrawable();
    int index = 0;
    // iteration through all planets and moons
    for (auto child: children) {
        auto parent = child->getParent();

        child->setLocalTransform(glm::rotate(parent->getLocalTransform(), float(glfwGetTime() * child->getSpeed()),
                                             glm::fvec3{0.0f, 1.0f, 0.0f}));

        child->setLocalTransform(
                glm::translate(child->getLocalTransform(), glm::fvec3{0.0f, 0.0f, child->getDistance()}));

        child->setLocalTransform(glm::scale(child->getLocalTransform(),
                                            glm::vec3(child->getSize(), child->getSize(), child->getSize())));

        // bind shader to upload uniforms
        glUseProgram(m_shaders.at(current_planet_shader_).handle);

        auto model_mat = child->getLocalTransform();
        glUniformMatrix4fv(m_shaders.at(current_planet_shader_).u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(model_mat));

        auto normal_mat = glm::inverseTranspose(glm::inverse(m_view_transform) * child->getLocalTransform());
        glUniformMatrix4fv(m_shaders.at(current_planet_shader_).u_locs.at("NormalMatrix"),
                           1, GL_FALSE, glm::value_ptr(normal_mat));

        // bind the VAO to draw
        glBindVertexArray(planet_object.vertex_AO);

        texture_object texture = texture_map.at(child->getName() + "_tex");
        texture_object normal_texture = texture_map.at(child->getName() + "_normal_tex");

        glActiveTexture(GL_TEXTURE1 + 2 * index);
        // bind texture
        glBindTexture(texture.target, texture.handle);
        // add sampler
        int samplerLocation = glGetUniformLocation(m_shaders.at(current_planet_shader_).handle, "TextureSampler");
        glUniform1i(samplerLocation, texture.handle);

        glActiveTexture(GL_TEXTURE1 + 2 * index + 1);
        glBindTexture(normal_texture.target, normal_texture.handle);
        int normalSamplerLocation = glGetUniformLocation(m_shaders.at(current_planet_shader_).handle, "NormalSampler");
        glUniform1i(normalSamplerLocation, normal_texture.handle);

        // add planet color
        int planetColorLocation = glGetUniformLocation(m_shaders.at(current_planet_shader_).handle, "planet_color");
        Color planet_color = color_map.find(child->getName())->second;
        glUniform3f(planetColorLocation, planet_color.r / 255.0f, planet_color.g / 255.0f, planet_color.b / 255.0f);

        //update the position, intensity and color of the point light
        auto light_node = solar_system_.getRoot()->getChildren("sun");
        auto light = std::static_pointer_cast<PointLightNode>(light_node);
        Color light_color = light->getColor();
        float light_intensity = light->getLightIntensity();
        glm::fvec4 light_position = light->getWorldTransform() * glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f);

        int lightPositionLocation = glGetUniformLocation(m_shaders.at(current_planet_shader_).handle, "light_position");
        glUniform3f(lightPositionLocation, light_position.x, light_position.y, light_position.z);

        int lightIntensityLocation = glGetUniformLocation(m_shaders.at(current_planet_shader_).handle,
                                                          "light_intensity");
        glUniform1f(lightIntensityLocation, light_intensity);

        int lightColorLocation = glGetUniformLocation(m_shaders.at(current_planet_shader_).handle, "light_color");
        glUniform3f(lightColorLocation, light_color.r / 255.0f, light_color.g / 255.0f, light_color.b / 255.0f);

        int ambientStrengthLocation = glGetUniformLocation(m_shaders.at(current_planet_shader_).handle,
                                                           "ambient_intensity");

        if (child->getName() == "sun") {
            glUniform1f(ambientStrengthLocation, 1.0);
        } else {
            glUniform1f(ambientStrengthLocation, 0.5);
        }

        // draw bound vertex array using bound shader
        glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
        index++;
    }
}

void ApplicationSolar::renderStars() const {
    glUseProgram(m_shaders.at("star").handle);
    // bind the VAO to draw
    glBindVertexArray(star_object.vertex_AO);
    // draw bound vertex array using bound shader
    glDrawArrays(star_object.draw_mode, GLint(0), star_object.num_elements);
}

void ApplicationSolar::renderOrbits() const {
    //declare the shader we want to use
    glUseProgram(m_shaders.at("orbit").handle);
    auto drawables = solar_system_.getRoot()->getDrawable();
    //for every orbit of a planet draw it
    for (auto object : drawables) {
        if (object->getName() == "moon") {
            continue;
        }
        //get the geometry of the orbit that is stored in the node
        auto orbit = object->getChildren(object->getName() + "_geom_orbit");

        auto orbit_geom = std::static_pointer_cast<GeometryNode>(orbit);
        auto orbit_world_transform = orbit->getWorldTransform();
        model orbit_model = orbit_geom->getGeometry();
        std::vector<GLfloat> orbit_data = (*orbit_geom).getGeometry().data;

        //create the ModelMatrix using the WorldTransform of the orbit
        glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                           1, GL_FALSE, glm::value_ptr(orbit_world_transform));

        //bind the buffer of the orbit to set the data of the current orbit
        glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * orbit_data.size(), orbit_data.data(), GL_STATIC_DRAW);

        //bind the VertexArray for drawing
        glBindVertexArray(orbit_object.vertex_AO);

        //draw the array
        glDrawArrays(orbit_object.draw_mode, GLint(0), orbit_object.num_elements);
    }
}

void ApplicationSolar::renderSkybox() const {
    glDepthMask(GL_FALSE);
    glUseProgram(m_shaders.at("skybox").handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_obj_.handle);
    glBindVertexArray(skybox_object.vertex_AO);
    glDrawElements(skybox_object.draw_mode, skybox_object.num_elements, model::INDEX.type, NULL);
    glDepthMask(GL_TRUE);
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

    glUseProgram(m_shaders.at("cel_shading").handle);
    // upload matrix to gpu
    glUniformMatrix4fv(m_shaders.at("cel_shading").u_locs.at("ViewMatrix"),
                       1, GL_FALSE, glm::value_ptr(view_matrix));

    glUseProgram(m_shaders.at("orbit").handle);

    glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
                       1, GL_FALSE, glm::value_ptr(view_matrix));

    //bind & upload ViewMatric for skybox
    glUseProgram(m_shaders.at("skybox").handle);
    glUniformMatrix4fv(m_shaders.at("skybox").u_locs.at("ViewMatrix"),
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

    glUseProgram(m_shaders.at("cel_shading").handle);

    // upload matrix to gpu
    glUniformMatrix4fv(m_shaders.at("cel_shading").u_locs.at("ProjectionMatrix"),
                       1, GL_FALSE, glm::value_ptr(m_view_projection));

    // upload matrix to gpu
    glUseProgram(m_shaders.at("orbit").handle);

    glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
                       1, GL_FALSE, glm::value_ptr(m_view_projection));

    // upload matrix to gpu
    glUseProgram(m_shaders.at("skybox").handle);
    glUniformMatrix4fv(m_shaders.at("skybox").u_locs.at("ProjectionMatrix"),
                       1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
    // upload uniform values to new locations
    uploadView();
    uploadProjection();
}

void ApplicationSolar::initializeOrbits() {
    auto drawables = solar_system_.getRoot()->getDrawable();
    float angle = 0.1f;
    size_t num_points = 65;

    std::vector<GLfloat> orbit_points;
    for (auto object: drawables) {
        orbit_points.clear();
        glm::mat4x4 rot_mat{};
        if (object->getName().find("moon") != std::string::npos) {

            rot_mat = glm::translate(object->getParent()->getLocalTransform(),
                                     glm::fvec3{0.0f, 0.0f, object->getDistance()});
            rot_mat = glm::rotate(rot_mat, angle, glm::fvec3{0.0f, 1.0f, 0.0f});
        } else {
            rot_mat = glm::rotate(glm::mat4x4{}, angle, glm::fvec3{0.0f, 1.0f, 0.0f});
        }
        auto point = object->getLocalTransform() * glm::fvec4{0.0f, 0.0f, 0.0f, 1.0f};
        for (size_t i = 0; i < num_points; ++i) {
            orbit_points.push_back(point.x);
            orbit_points.push_back(point.y);
            orbit_points.push_back(point.z);
            point = rot_mat * point;
        }
        model orbit_model{};
        orbit_model.data = orbit_points;
        orbit_model.vertex_num = num_points;

        auto orbit_node = std::make_shared<GeometryNode>(object, object->getName() + "_geom_orbit", orbit_model);
        object->addChildren(orbit_node);
    }

    // create new VAO
    glGenVertexArrays(1, &orbit_object.vertex_AO);
    glBindVertexArray(orbit_object.vertex_AO);

    // generate a new Buffer and bind it to the new VertexArray
    glGenBuffers(1, &orbit_object.vertex_BO);
    glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
    //specify the size of the data
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * orbit_points.size(), orbit_points.data(), GL_STATIC_DRAW);

    // attribute Array for positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(3 * sizeof(float)), 0);

    // draw mode and the number of points
    orbit_object.draw_mode = GL_LINE_STRIP;
    orbit_object.num_elements = GLsizei(num_points);
}


///////////////////////////// intialisation functions /////////////////////////
void ApplicationSolar::initializeSolarSystem() {
    // scenegraph with root
    std::shared_ptr<Node> root = std::make_shared<Node>("root");
    solar_system_ = SceneGraph("solarSystem", root);

    // sun
    std::shared_ptr<PointLightNode> sun_holder = std::make_shared<PointLightNode>("sun", root);
    std::shared_ptr<GeometryNode> geo_sun = std::make_shared<GeometryNode>(sun_holder, "geo_sun");
    root->addChildren(sun_holder);
    sun_holder->setLightIntensity(1);
    sun_holder->setColor(Color{255, 255, 255});
    sun_holder->setDistance(0.0f);
    sun_holder->setSize(7.0f);
    sun_holder->addChildren(geo_sun);

    // merkur
    std::shared_ptr<Node> merkur_holder = std::make_shared<Node>("mercury", root);
    std::shared_ptr<GeometryNode> geo_merkur = std::make_shared<GeometryNode>(merkur_holder, "geo_mercury");
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
    std::shared_ptr<Node> neptun_holder = std::make_shared<Node>("neptune", root);
    std::shared_ptr<GeometryNode> geo_neptun = std::make_shared<GeometryNode>(saturn_holder, "geo_neptune");
    neptun_holder->setSpeed(0.607f);
    neptun_holder->setDistance(60.0f + sun_holder->getSize()); //388.706
    neptun_holder->setSize(2.0f);
    root->addChildren(neptun_holder);
    neptun_holder->addChildren(geo_neptun);

    color_map.insert({"sun", {255, 255, 0}});
    color_map.insert({"uranus", {188, 255, 252}});
    color_map.insert({"venus", {251, 213, 152}});
    color_map.insert({"earth", {78, 153, 255}});
    color_map.insert({"moon", {219, 219, 219}});
    color_map.insert({"mercury", {157, 157, 157}});
    color_map.insert({"mars", {255, 80, 0}});
    color_map.insert({"jupiter", {255, 207, 128}});
    color_map.insert({"saturn", {229, 212, 186}});
    color_map.insert({"neptune", {99, 204, 251}});
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
    m_shaders.at("planet").u_locs["TextureSampler"] = -1;


    // store shader program objects in container
    m_shaders.emplace("cel_shading", shader_program{{{GL_VERTEX_SHADER, m_resource_path + "shaders/cel_shading.vert"},
                                                            {GL_FRAGMENT_SHADER,
                                                                    m_resource_path + "shaders/cel_shading.frag"}}});
    // request uniform locations for shader program
    m_shaders.at("cel_shading").u_locs["NormalMatrix"] = -1;
    m_shaders.at("cel_shading").u_locs["ModelMatrix"] = -1;
    m_shaders.at("cel_shading").u_locs["ViewMatrix"] = -1;
    m_shaders.at("cel_shading").u_locs["ProjectionMatrix"] = -1;
    m_shaders.at("planet").u_locs["TextureSampler"] = -1;


    m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER, m_resource_path + "shaders/vao.vert"},
                                                     {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});

    // request uniform locations for shader program
    m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
    m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;

    // store orbit shader in container
    m_shaders.emplace("orbit", shader_program{{{GL_VERTEX_SHADER, m_resource_path + "shaders/orbit.vert"},
                                                      {GL_FRAGMENT_SHADER, m_resource_path + "shaders/orbit.frag"}}});
    // request uniform locations for shader program
    m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
    m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
    m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;

    // now initialize shaders for skybox
    m_shaders.emplace("skybox", shader_program{{{GL_VERTEX_SHADER, m_resource_path + "shaders/skybox.vert"},
                                                       {GL_FRAGMENT_SHADER, m_resource_path + "shaders/skybox.frag"}}});

    // request uniform location for shader program
    m_shaders.at("skybox").u_locs["ProjectionMatrix"] = -1;
    m_shaders.at("skybox").u_locs["ViewMatrix"] = -1;
}

void ApplicationSolar::initializeStarsGeometry() {
    int const numberStars = 5000;
    std::vector<GLfloat> stars; //= std::vector<GLfloat>(6 * numberStars * sizeof(float));
    stars.reserve(6 * numberStars * sizeof(float));

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
    model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD);

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
    // activate third attribute on gpu
    glEnableVertexAttribArray(2);
    // third attribute is 2 floats with no offset & stride
    glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes,
                          planet_model.offsets[model::TEXCOORD]);

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

    // add skybox model
    model skybox_model = model_loader::obj(m_resource_path + "models/skybox.obj");
    // starting with VAO
    glGenVertexArrays(1, &skybox_object.vertex_AO);
    // bind that
    glBindVertexArray(skybox_object.vertex_AO);
    // generic buffer
    glGenBuffers(1, &skybox_object.vertex_BO);
    // again : binding as vertex array buffer with all attributes
    glBindBuffer(GL_ARRAY_BUFFER, skybox_object.vertex_BO);
    // configuration
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * skybox_model.data.size(), skybox_model.data.data(), GL_STATIC_DRAW);
    // activation of first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, skybox_model.vertex_bytes,
                          skybox_model.offsets[model::POSITION]);
    // generate generic buffer
    glGenBuffers(1, &skybox_object.element_BO);
    // bind as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_object.element_BO);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * skybox_model.indices.size(), skybox_model.indices.data(),
                 GL_STATIC_DRAW);
    // store type of primitive to draw
    skybox_object.draw_mode = GL_TRIANGLES;
    // transfer number of indices to model object
    skybox_object.num_elements = GLsizei(skybox_model.indices.size());
}

void ApplicationSolar::initializeTextures() {
    auto drawables = solar_system_.getRoot()->getDrawable();
    int planetIndex = 0;
    pixel_data planet_texture;
    for (auto object: drawables) {
        try {
            planet_texture = texture_loader::file(m_resource_path + "textures/" + object->getName() + ".png");
        }
        catch (std::exception e) {
            std::cout << "Error loading texturefile for " + object->getName() + ". \n " + e.what() + "\n";
        }

        GLsizei width = (GLsizei) planet_texture.width;
        GLsizei height = (GLsizei) planet_texture.height;
        GLenum channel_number = planet_texture.channels;
        GLenum channel_type = planet_texture.channel_type;

        //glActiveTexture(GL_TEXTURE+planetIndex);
        glActiveTexture(GL_TEXTURE1 + 2 * planetIndex);
        texture_object texture;
        glGenTextures(1, &texture.handle);
        texture.target = GL_TEXTURE_2D;
        std::string texture_name = object->getName() + "_tex";
        texture_map.insert({texture_name, texture});

        glBindTexture(texture.target, texture.handle);

        //optional

        //Texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //Texture filteriglTexImage2D(GL_TEXTURE_2D, 0, ng
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, channel_number, width, height, 0, channel_number, channel_type,
                     planet_texture.ptr());
        glGenerateMipmap(GL_TEXTURE_2D);
        planetIndex++;

        try {
            planet_texture = texture_loader::file(m_resource_path + "normal_maps/" + object->getName() + ".png");
        }
        catch (std::exception e) {
            planet_texture = texture_loader::file(m_resource_path + "normal_maps/sun.png");
            std::cout << "Error loading texturefile for " + object->getName() + ". \n " + e.what() +
                         ". Default normal was loaded.\n";
        }

        width = (GLsizei) planet_texture.width;
        height = (GLsizei) planet_texture.height;
        channel_number = planet_texture.channels;
        channel_type = planet_texture.channel_type;

        //glActiveTexture(GL_TEXTURE+planetIndex);
        glActiveTexture(GL_TEXTURE1 + 2 * planetIndex + 1);
        texture_object normal_texture;
        glGenTextures(1, &normal_texture.handle);
        normal_texture.target = GL_TEXTURE_2D;
        texture_name = object->getName() + "_normal_tex";
        texture_map.insert({texture_name, normal_texture});

        glBindTexture(normal_texture.target, normal_texture.handle);

        //optional

        //Texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //Texture filteriglTexImage2D(GL_TEXTURE_2D, 0, ng
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, channel_number, width, height, 0, channel_number, channel_type,
                     planet_texture.ptr());
        //glGenerateMipmap(GL_TEXTURE_2D);
        planetIndex++;
    }

    /* used as reference :
    https://learnopengl.com/Advanced-OpenGL/Cubemaps
    */
    std::cout << "Loading Skybox textures..." << std::endl;
    // load the textures for the skybox
    // try with sequence they are looped through
    //  RIGHT
    pixel_data sideright = texture_loader::file(m_resource_path + "textures/skybox/right.png");
    skybox_contain_pixdata_.push_back(sideright);

    // LEFT
    pixel_data sideleft = texture_loader::file(m_resource_path + "textures/skybox/left.png");
    skybox_contain_pixdata_.push_back(sideleft);
    // BOTTOM
    pixel_data up = texture_loader::file(m_resource_path + "textures/skybox/bottom.png");
    skybox_contain_pixdata_.push_back(up);
    // TOP
    pixel_data down = texture_loader::file(m_resource_path + "textures/skybox/top.png");
    skybox_contain_pixdata_.push_back(down);
    // FRONT
    pixel_data back = texture_loader::file(m_resource_path + "textures/skybox/front.png");
    skybox_contain_pixdata_.push_back(back);
    // BACK
    pixel_data front = texture_loader::file(m_resource_path + "textures/skybox/back.png");
    skybox_contain_pixdata_.push_back(front);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &skybox_texture_obj_.handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_obj_.handle);
    // The WARP_S/T/R set the warping methods for the textures s/t/r coordinates
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    for (unsigned int i = 0; i < skybox_contain_pixdata_.size(); ++i) {
        // starting off with GL_TEXTURE_CUBE_MAP_POSITIVE_X and incrementing it with +i so it will loop through the
        // texture targets
        // POSITIVE_X being right, NEGATIVE_X being left etc. etc.
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, skybox_contain_pixdata_[i].channels,
                     (GLsizei) skybox_contain_pixdata_[i].width, (GLsizei) skybox_contain_pixdata_[i].height,
                     0, skybox_contain_pixdata_[i].channels, skybox_contain_pixdata_[i].channel_type,
                     skybox_contain_pixdata_[i].ptr());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
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
    } else if (key == GLFW_KEY_1 && (action == GLFW_PRESS)) {
        current_planet_shader_ = "planet";
        uploadView();
    } else if (key == GLFW_KEY_2 && (action == GLFW_PRESS)) {
        current_planet_shader_ = "cel_shading";
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