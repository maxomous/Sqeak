#include "material.h"

MaterialPicker::MaterialPicker()
{
    AddMaterial(MaterialType::Emerald);
    AddMaterial(MaterialType::Jade);
    AddMaterial(MaterialType::Obsidian);
    AddMaterial(MaterialType::Pearl);
    AddMaterial(MaterialType::Ruby);
    AddMaterial(MaterialType::Turquoise);
    AddMaterial(MaterialType::Brass);
    AddMaterial(MaterialType::Bronze);
    AddMaterial(MaterialType::Chrome);
    AddMaterial(MaterialType::Copper);
    AddMaterial(MaterialType::Gold);
    AddMaterial(MaterialType::Silver);
    AddMaterial(MaterialType::Plastic_Black);
    AddMaterial(MaterialType::Plastic_Cyan);
    AddMaterial(MaterialType::Plastic_Green);
    AddMaterial(MaterialType::Plastic_Red);
    AddMaterial(MaterialType::Plastic_White);
    AddMaterial(MaterialType::Plastic_Yellow);
    AddMaterial(MaterialType::Rubber_Black);
    AddMaterial(MaterialType::Rubber_Cyan);
    AddMaterial(MaterialType::Rubber_Green);
    AddMaterial(MaterialType::Rubber_Red);
    AddMaterial(MaterialType::Rubber_White);
    AddMaterial(MaterialType::Rubber_Yellow);
    // set initial material
    SetMaterial(0);
}
// add material
void MaterialPicker::AddMaterial(const Material& material) 
{ 
    m_Materials.push_back(material);
}


bool MaterialPicker::IsMaterialSelected() 
{
    return (m_CurrentMaterial >= 0);
}

const Material& MaterialPicker::GetCurrentMaterial()
{
    assert(IsMaterialSelected() && "No material selected");
    return m_Materials[m_CurrentMaterial];
}

void MaterialPicker::SetMaterial(size_t index)
{
    assert(index < m_Materials.size() && "Material index out of range");
    m_CurrentMaterial = (int)index;
}

void MaterialPicker::SetMaterial(const std::string& name)
{
    for(size_t i = 0; i < m_Materials.size(); i++) {
        if(m_Materials[i].name == name) {
            m_CurrentMaterial = (int)i;
            return;
        }
    }
    assert(0 && "Material does not exist, add the material first");
}

// draws imgui list box for material list (returns true if changed)
bool MaterialPicker::ListBox(int height_in_items)
{  
    auto cb_GetName = [](void* data, int n, const char** out_str){ *out_str = (*(std::vector<Material>*)data)[n].name.c_str(); return true; };
    if(ImGui::ListBox("Materials", &m_CurrentMaterial, cb_GetName, &m_Materials, m_Materials.size(), height_in_items)) {
        SetMaterial(m_CurrentMaterial);
        return true;
    }
    return false;
}
