#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include <SceneGraph.hpp>
#include <Color.hpp>
#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

// gpu representation of model
class ApplicationSolar : public Application {
public:
    // allocate and initialize objects
    ApplicationSolar(std::string const &resource_path);

    // free allocated objects
    ~ApplicationSolar();

    // react to key input
    void keyCallback(int key, int action, int mods);

    //handle delta mouse movement input
    void mouseCallback(double pos_x, double pos_y);

    //handle resizing
    void resizeCallback(unsigned width, unsigned height);

    // draw all objects
    void render() const;

    void renderPlanets() const;

    void renderStars() const;

    // draw nice orbits (Extra)
    void renderOrbits() const;

    void renderSkybox() const;

    void renderScreenQuad() const;

protected:

    void initializeSolarSystem();

    void initializeShaderPrograms();

    void initializeGeometry();

    void initializeStarsGeometry();

    void initializeOrbits();

    void initializeTextures();

    void initializeScreenquad();

    bool initializeFramebuffer(unsigned width, unsigned height);

    // update uniform values
    void uploadUniforms();

    // upload projection matrix
    void uploadProjection();

    // upload view matrix
    void uploadView();

    // scenegraph
    SceneGraph solar_system_;

    // cpu representation of model
    model_object planet_object;
    model_object star_object;
    model_object orbit_object;
    model_object skybox_object;

    model_object screenquad_object;
    framebuffer_object framebuffer_obj;

    // camera transform matrix
    glm::fmat4 m_view_transform;
    // camera projection matrix
    glm::fmat4 m_view_projection;

    std::string current_planet_shader_;

    std::map<std::string, Color> color_map;

    std::map<std::string, texture_object> texture_map;

    // need vector to hold pixel-data of skybox-tex
    std::vector<pixel_data> skybox_contain_pixdata_;
    // add skybox texture_object
    texture_object skybox_texture_obj_;

private:
    bool horizontal_mirroring = false;
    bool vertical_mirroring = false;
    bool greyscale = false;
    bool blur = false;
    unsigned img_width;
    unsigned img_height;
    bool time = true;
};

#endif