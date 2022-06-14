#pragma once

#include <map>
#include "glcore.h"

struct Material {
    std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};


static std::string material_VertexShader = R"(
#version 140

in vec3 in_Position;
in vec3 in_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform vec3 u_LightPosition;

out vec3 v_FragmentPosition;
out vec3 v_Normal;
out vec3 v_LightPosition;

void main(void)
{
	gl_Position = u_Proj * u_View * u_Model * vec4(in_Position, 1.0f);
    
    // Calculate in view space (transform all relavant vectors with the view matrix)
    v_FragmentPosition = vec3(u_View * u_Model * vec4(in_Position, 1.0f));
    
    // The normal matrix updates the normals based on the modelview matrix
    // We cast to mat3 to lose the translation component
    v_Normal = mat3(transpose(inverse(u_View * u_Model))) * in_Normal;
    
    v_LightPosition = vec3(u_View * vec4(u_LightPosition, 1.0));
}
)";

// phong lighting model

static std::string material_FragmentShader = R"(
#version 140

precision highp float; // needed only for version 1.30

in vec3 v_Normal;
in vec3 v_LightPosition;
in vec3 v_FragmentPosition;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
uniform Material u_Material;
uniform vec3 u_LightColour;

out vec4 out_Colour;


void main(void)
{
// Ambient
    vec3 ambient = u_LightColour * u_Material.ambient;
    
// Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 lightDirection = normalize(v_LightPosition - v_FragmentPosition);
    // If Normal direction == Light direction, then it will be brightest
    // At 90degrees, it will be darkest & ignore -ve (<90 degrees)
    float diffuseAngle = max(dot(norm, lightDirection), 0.0f);
    vec3 diffuse =  u_LightColour * (diffuseAngle * u_Material.diffuse);

// Specular
    vec3 viewDirection = normalize(-v_FragmentPosition);
    vec3 reflectDirection = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0f), u_Material.shininess);
    vec3 specular = u_LightColour * (spec * u_Material.specular);

    vec3 result = ambient + diffuse + specular;
	out_Colour = vec4(result, 1.0f);
}
)";

namespace MaterialType
{    
    
 static Material Emerald = {
    /* name */      "Emerald",
    /* ambient */   glm::vec3(0.0215f, 0.1745f, 0.0215f),
    /* diffuse */   glm::vec3(0.07568f, 0.61424f, 0.07568f),
    /* specular */  glm::vec3(0.633f, 0.727811f, 0.633f),
    /* shininess */ 76.8f
};
static Material Jade = {
    /* name */      "Jade",
    /* ambient */   glm::vec3(0.135f, 0.2225f, 0.1575f),
    /* diffuse */   glm::vec3(0.54f, 0.89f, 0.63f),
    /* specular */  glm::vec3(0.316228f, 0.316228f, 0.316228f),
    /* shininess */ 12.8f
};
static Material Obsidian = {
    /* name */      "Obsidian",
    /* ambient */   glm::vec3(0.05375f, 0.05f, 0.06625f),
    /* diffuse */   glm::vec3(0.18275f, 0.17f, 0.22525f),
    /* specular */  glm::vec3(0.332741f, 0.328634f, 0.346435f),
    /* shininess */ 38.4f
};
static Material Pearl = {
    /* name */      "Pearl",
    /* ambient */   glm::vec3(0.25f, 0.20725f, 0.20725f),
    /* diffuse */   glm::vec3(1.0f, 0.829f, 0.829f),
    /* specular */  glm::vec3(0.296648f, 0.296648f, 0.296648f),
    /* shininess */ 11.264f
};
static Material Ruby = {
    /* name */      "Ruby",
    /* ambient */   glm::vec3(0.1745f, 0.01175f, 0.01175f),
    /* diffuse */   glm::vec3(0.61424f, 0.04136f, 0.04136f),
    /* specular */  glm::vec3(0.727811f, 0.626959f, 0.626959f),
    /* shininess */ 76.8f
};
static Material Turquoise = {
    /* name */      "Turquoise",
    /* ambient */   glm::vec3(0.1f, 0.18725f, 0.1745f),
    /* diffuse */   glm::vec3(0.396f, 0.74151f, 0.69102f),
    /* specular */  glm::vec3(0.297254f, 0.30829f, 0.306678f),
    /* shininess */ 12.8f
};
static Material Brass = {
    /* name */      "Brass",
    /* ambient */   glm::vec3(0.329412f, 0.223529f, 0.027451f),
    /* diffuse */   glm::vec3(0.780392f, 0.568627f, 0.113725f),
    /* specular */  glm::vec3(0.992157f, 0.941176f, 0.807843f),
    /* shininess */ 27.89743616f
};
static Material Bronze = {
    /* name */      "Bronze",
    /* ambient */   glm::vec3(0.2125f, 0.1275f, 0.054f),
    /* diffuse */   glm::vec3(0.714f, 0.4284f, 0.18144f),
    /* specular */  glm::vec3(0.393548f, 0.271906f, 0.166721f),
    /* shininess */ 25.6f
};
static Material Chrome = {
    /* name */      "Chrome",
    /* ambient */   glm::vec3(0.25f, 0.25f, 0.25f),
    /* diffuse */   glm::vec3(0.4f, 0.4f, 0.4f),
    /* specular */  glm::vec3(0.774597f, 0.774597f, 0.774597f),
    /* shininess */ 76.8f
};
static Material Copper = {
    /* name */      "Copper",
    /* ambient */   glm::vec3(0.19125f, 0.0735f, 0.0225f),
    /* diffuse */   glm::vec3(0.7038f, 0.27048f, 0.0828f),
    /* specular */  glm::vec3(0.256777f, 0.137622f, 0.086014f),
    /* shininess */ 12.8f
};
static Material Gold = {
    /* name */      "Gold",
    /* ambient */   glm::vec3(0.24725f, 0.1995f, 0.0745f),
    /* diffuse */   glm::vec3(0.75164f, 0.60648f, 0.22648f),
    /* specular */  glm::vec3(0.628281f, 0.555802f, 0.366065f),
    /* shininess */ 51.2f
};
static Material Silver = {
    /* name */      "Silver",
    /* ambient */   glm::vec3(0.19225f, 0.19225f, 0.19225f),
    /* diffuse */   glm::vec3(0.50754f, 0.50754f, 0.50754f),
    /* specular */  glm::vec3(0.508273f, 0.508273f, 0.508273f),
    /* shininess */ 51.2f
};
static Material Plastic_Black = {
    /* name */      "Plastic - Black",
    /* ambient */   glm::vec3(0.0f, 0.0f, 0.0f),
    /* diffuse */   glm::vec3(0.01f, 0.01f, 0.01f),
    /* specular */  glm::vec3(0.5f, 0.5f, 0.5f),
    /* shininess */ 32.0f
};
static Material Plastic_Cyan = {
    /* name */      "Plastic - Cyan",
    /* ambient */   glm::vec3(0.0f, 0.1f, 0.06f),
    /* diffuse */   glm::vec3(0.0f, 0.50980392f, 0.50980392f),
    /* specular */  glm::vec3(0.50196078f, 0.50196078f, 0.50196078f),
    /* shininess */ 32.0f
};
static Material Plastic_Green = {
    /* name */      "Plastic - Green",
    /* ambient */   glm::vec3(0.0f, 0.0f, 0.0f),
    /* diffuse */   glm::vec3(0.1f, 0.35f, 0.1f),
    /* specular */  glm::vec3(0.45f, 0.55f, 0.45f),
    /* shininess */ 32.0f
};
static Material Plastic_Red = {
    /* name */      "Plastic - Red",
    /* ambient */   glm::vec3(0.0f, 0.0f, 0.0f),
    /* diffuse */   glm::vec3(0.5f, 0.0f, 0.0f),
    /* specular */  glm::vec3(0.7f, 0.6f, 0.6f),
    /* shininess */ 32.0f
};
static Material Plastic_White = {
    /* name */      "Plastic - White",
    /* ambient */   glm::vec3(0.0f, 0.0f, 0.0f),
    /* diffuse */   glm::vec3(0.55f, 0.55f, 0.55f),
    /* specular */  glm::vec3(0.7f, 0.7f, 0.7f),
    /* shininess */ 32.0f
};
static Material Plastic_Yellow = {
    /* name */      "Plastic - Yellow",
    /* ambient */   glm::vec3(0.0f, 0.0f, 0.0f),
    /* diffuse */   glm::vec3(0.5f, 0.5f, 0.0f),
    /* specular */  glm::vec3(0.6f, 0.6f, 0.5f),
    /* shininess */ 32.0f
};
static Material Rubber_Black = {
    /* name */      "Rubber - Black",
    /* ambient */   glm::vec3(0.02f, 0.02f, 0.02f),
    /* diffuse */   glm::vec3(0.01f, 0.01f, 0.01f),
    /* specular */  glm::vec3(0.4f, 0.4f, 0.4f),
    /* shininess */ 10.0f
};
static Material Rubber_Cyan = {
    /* name */      "Rubber - Cyan",
    /* ambient */   glm::vec3(0.0f, 0.05f, 0.05f),
    /* diffuse */   glm::vec3(0.4f, 0.5f, 0.5f),
    /* specular */  glm::vec3(0.04f, 0.7f, 0.7f),
    /* shininess */ 10.0f
};
static Material Rubber_Green = {
    /* name */      "Rubber - Green",
    /* ambient */   glm::vec3(0.0f, 0.05f, 0.0f),
    /* diffuse */   glm::vec3(0.4f, 0.5f, 0.4f),
    /* specular */  glm::vec3(0.04f, 0.7f, 0.04f),
    /* shininess */ 10.0f
};
static Material Rubber_Red = {
    /* name */      "Rubber - Red",
    /* ambient */   glm::vec3(0.05f, 0.0f, 0.0f),
    /* diffuse */   glm::vec3(0.5f, 0.4f, 0.4f),
    /* specular */  glm::vec3(0.7f, 0.04f, 0.04f),
    /* shininess */ 10.0f
};
static Material Rubber_White = {
    /* name */      "Rubber - White",
    /* ambient */   glm::vec3(0.05f, 0.05f, 0.05f),
    /* diffuse */   glm::vec3(0.5f, 0.5f, 0.5f),
    /* specular */  glm::vec3(0.7f, 0.7f, 0.7f),
    /* shininess */ 10.0f
};
static Material Rubber_Yellow = {
    /* name */      "Rubber - Yellow",
    /* ambient */   glm::vec3(0.05f, 0.05f, 0.0f),
    /* diffuse */   glm::vec3(0.5f, 0.5f, 0.4f),
    /* specular */  glm::vec3(0.7f, 0.7f, 0.04f),
    /* shininess */ 10.0f
};
} // end MaterialType namespace


// holds a lsit of all materials in a vector and allows the current one to be selected using an imgui listbox
class MaterialPicker
{
public:
    MaterialPicker();
    // add material to picker
    void AddMaterial(const Material& material);
    
    bool IsMaterialSelected();
    // returns current material
    const Material& GetCurrentMaterial();
    // set current material by index
    void SetMaterial(size_t index);
    // set current material by name
    void SetMaterial(const std::string& name);
    
    /* draws imgui list box of materials list (returns true if changed)
        Usage:
        if(m_MaterialPicker.ListBox(4)) { // height in items        
            if(m_MaterialPicker.IsMaterialSelected()) { m_Material = m_MaterialPicker.GetCurrentMaterial(); }
        }
    */
    bool ListBox(int height_in_items = -1);
    
private:
    std::vector<Material> m_Materials;
    // current item selected in listbox
    int m_CurrentMaterial = -1;
};
