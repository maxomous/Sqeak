#pragma once

#include <map>
#include "glcore.h"

struct Material {
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
        .ambient = glm::vec3(0.0215f, 0.1745f, 0.0215f),
        .diffuse = glm::vec3(0.07568f, 0.61424f, 0.07568f),
        .specular = glm::vec3(0.633f, 0.727811f, 0.633f),
        .shininess = 76.8f
    };
    static Material Jade = {
        .ambient = glm::vec3(0.135f, 0.2225f, 0.1575f),
        .diffuse = glm::vec3(0.54f, 0.89f, 0.63f),
        .specular = glm::vec3(0.316228f, 0.316228f, 0.316228f),
        .shininess = 12.8f
    };
    static Material Obsidian = {
        .ambient = glm::vec3(0.05375f, 0.05f, 0.06625f),
        .diffuse = glm::vec3(0.18275f, 0.17f, 0.22525f),
        .specular = glm::vec3(0.332741f, 0.328634f, 0.346435f),
        .shininess = 38.4f
    };
    static Material Pearl = {
        .ambient = glm::vec3(0.25f, 0.20725f, 0.20725f),
        .diffuse = glm::vec3(1.0f, 0.829f, 0.829f),
        .specular = glm::vec3(0.296648f, 0.296648f, 0.296648f),
        .shininess = 11.264f
    };
    static Material Ruby = {
        .ambient = glm::vec3(0.1745f, 0.01175f, 0.01175f),
        .diffuse = glm::vec3(0.61424f, 0.04136f, 0.04136f),
        .specular = glm::vec3(0.727811f, 0.626959f, 0.626959f),
        .shininess = 76.8f
    };
    static Material Turquoise = {
        .ambient = glm::vec3(0.1f, 0.18725f, 0.1745f),
        .diffuse = glm::vec3(0.396f, 0.74151f, 0.69102f),
        .specular = glm::vec3(0.297254f, 0.30829f, 0.306678f),
        .shininess = 12.8f
    };
    static Material Brass = {
        .ambient = glm::vec3(0.329412f, 0.223529f, 0.027451f),
        .diffuse = glm::vec3(0.780392f, 0.568627f, 0.113725f),
        .specular = glm::vec3(0.992157f, 0.941176f, 0.807843f),
        .shininess = 27.89743616f
    };
    static Material Bronze = {
        .ambient = glm::vec3(0.2125f, 0.1275f, 0.054f),
        .diffuse = glm::vec3(0.714f, 0.4284f, 0.18144f),
        .specular = glm::vec3(0.393548f, 0.271906f, 0.166721f),
        .shininess = 25.6f
    };
    static Material Chrome = {
        .ambient = glm::vec3(0.25f, 0.25f, 0.25f),
        .diffuse = glm::vec3(0.4f, 0.4f, 0.4f),
        .specular = glm::vec3(0.774597f, 0.774597f, 0.774597f),
        .shininess = 76.8f
    };
    static Material Copper = {
        .ambient = glm::vec3(0.19125f, 0.0735f, 0.0225f),
        .diffuse = glm::vec3(0.7038f, 0.27048f, 0.0828f),
        .specular = glm::vec3(0.256777f, 0.137622f, 0.086014f),
        .shininess = 12.8f
    };
    static Material Gold = {
        .ambient = glm::vec3(0.24725f, 0.1995f, 0.0745f),
        .diffuse = glm::vec3(0.75164f, 0.60648f, 0.22648f),
        .specular = glm::vec3(0.628281f, 0.555802f, 0.366065f),
        .shininess = 51.2f
    };
    static Material Silver = {
        .ambient = glm::vec3(0.19225f, 0.19225f, 0.19225f),
        .diffuse = glm::vec3(0.50754f, 0.50754f, 0.50754f),
        .specular = glm::vec3(0.508273f, 0.508273f, 0.508273f),
        .shininess = 51.2f
    };
    static Material BlackPlastic = {
        .ambient = glm::vec3(0.0f, 0.0f, 0.0f),
        .diffuse = glm::vec3(0.01f, 0.01f, 0.01f),
        .specular = glm::vec3(0.5f, 0.5f, 0.5f),
        .shininess = 32.0f
    };
    static Material CyanPlastic = {
        .ambient = glm::vec3(0.0f, 0.1f, 0.06f),
        .diffuse = glm::vec3(0.0f, 0.50980392f, 0.50980392f),
        .specular = glm::vec3(0.50196078f, 0.50196078f, 0.50196078f),
        .shininess = 32.0f
    };
    static Material GreenPlastic = {
        .ambient = glm::vec3(0.0f, 0.0f, 0.0f),
        .diffuse = glm::vec3(0.1f, 0.35f, 0.1f),
        .specular = glm::vec3(0.45f, 0.55f, 0.45f),
        .shininess = 32.0f
    };
    static Material RedPlastic = {
        .ambient = glm::vec3(0.0f, 0.0f, 0.0f),
        .diffuse = glm::vec3(0.5f, 0.0f, 0.0f),
        .specular = glm::vec3(0.7f, 0.6f, 0.6f),
        .shininess = 32.0f
    };
    static Material WhitePlastic = {
        .ambient = glm::vec3(0.0f, 0.0f, 0.0f),
        .diffuse = glm::vec3(0.55f, 0.55f, 0.55f),
        .specular = glm::vec3(0.7f, 0.7f, 0.7f),
        .shininess = 32.0f
    };
    static Material YellowPlastic = {
        .ambient = glm::vec3(0.0f, 0.0f, 0.0f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.0f),
        .specular = glm::vec3(0.6f, 0.6f, 0.5f),
        .shininess = 32.0f
    };
    static Material BlackRubber = {
        .ambient = glm::vec3(0.02f, 0.02f, 0.02f),
        .diffuse = glm::vec3(0.01f, 0.01f, 0.01f),
        .specular = glm::vec3(0.4f, 0.4f, 0.4f),
        .shininess = 10.0f
    };
    static Material CyanRubber = {
        .ambient = glm::vec3(0.0f, 0.05f, 0.05f),
        .diffuse = glm::vec3(0.4f, 0.5f, 0.5f),
        .specular = glm::vec3(0.04f, 0.7f, 0.7f),
        .shininess = 10.0f
    };
    static Material GreenRubber = {
        .ambient = glm::vec3(0.0f, 0.05f, 0.0f),
        .diffuse = glm::vec3(0.4f, 0.5f, 0.4f),
        .specular = glm::vec3(0.04f, 0.7f, 0.04f),
        .shininess = 10.0f
    };
    static Material RedRubber = {
        .ambient = glm::vec3(0.05f, 0.0f, 0.0f),
        .diffuse = glm::vec3(0.5f, 0.4f, 0.4f),
        .specular = glm::vec3(0.7f, 0.04f, 0.04f),
        .shininess = 10.0f
    };
    static Material WhiteRubber = {
        .ambient = glm::vec3(0.05f, 0.05f, 0.05f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(0.7f, 0.7f, 0.7f),
        .shininess = 10.0f
    };
    static Material YellowRubber = {
        .ambient = glm::vec3(0.05f, 0.05f, 0.0f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.4f),
        .specular = glm::vec3(0.7f, 0.7f, 0.04f),
        .shininess = 10.0f
    };
}

class MaterialPicker
{
public:
    MaterialPicker()
    {
        AddMaterial("Emerald", MaterialType::Emerald);
        AddMaterial("Jade", MaterialType::Jade);
        AddMaterial("Obsidian", MaterialType::Obsidian);
        AddMaterial("Pearl", MaterialType::Pearl);
        AddMaterial("Ruby", MaterialType::Ruby);
        AddMaterial("Turquoise", MaterialType::Turquoise);
        AddMaterial("Brass", MaterialType::Brass);
        AddMaterial("Bronze", MaterialType::Bronze);
        AddMaterial("Chrome", MaterialType::Chrome);
        AddMaterial("Copper", MaterialType::Copper);
        AddMaterial("Gold", MaterialType::Gold);
        AddMaterial("Silver", MaterialType::Silver);
        AddMaterial("BlackPlastic", MaterialType::BlackPlastic);
        AddMaterial("CyanPlastic", MaterialType::CyanPlastic);
        AddMaterial("GreenPlastic", MaterialType::GreenPlastic);
        AddMaterial("RedPlastic", MaterialType::RedPlastic);
        AddMaterial("WhitePlastic", MaterialType::WhitePlastic);
        AddMaterial("YellowPlastic", MaterialType::YellowPlastic);
        AddMaterial("BlackRubber", MaterialType::BlackRubber);
        AddMaterial("CyanRubber", MaterialType::CyanRubber);
        AddMaterial("GreenRubber", MaterialType::GreenRubber);
        AddMaterial("RedRubber", MaterialType::RedRubber);
        AddMaterial("WhiteRubber", MaterialType::WhiteRubber);
        AddMaterial("YellowRubber", MaterialType::YellowRubber);
    }
    void AddMaterial(const std::string& name, Material& material) 
    { 
        m_Materials.insert({ name, material });
        // make an array of pointers to the names inside m_Materials
        m_NameList.push_back(m_Materials.find(name)->first.c_str());
    }
    Material& GetMaterial(const std::string& name) { return m_Materials.find(name)->second; }
    std::vector<const char*>& GetMaterialList() { return m_NameList; }
private:
    std::map<std::string, Material&> m_Materials;
    std::vector<const char*> m_NameList;
};
