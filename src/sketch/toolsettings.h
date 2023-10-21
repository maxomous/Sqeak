#pragma once    
#include "../glcore/glcore.h"
#include <MaxLib.h>

    

namespace Sqeak { 
    
using namespace MaxLib;
using namespace MaxLib::Geom;
using namespace MaxLib::Vector;

class ToolSettings
{
public:
    struct Tools 
    {   
        struct Tool 
        {
            struct ToolData {
                std::string material    = "Material";
                float speed             = 10000.0f;
                float feedCutting       = 1000.0f;
                float feedPlunge        = 300.0f;
                float cutDepth          = 1.0f;
            };
            
            Tool(std::string Name = "Tool", float Diameter = 6.0f, float Length = 20.0f) : name(Name), diameter(Diameter), length(Length) {}
            
            Vec3 Dimensions() { return Vec3(diameter, diameter, length); }
            
            std::string name;
            float diameter;
            float length;    
            Vector_SelectablePtrs<ToolData> data; // feeds, speeds & depths for different materials
        };
        Vector_SelectablePtrs<Tool> toolList;
        // check if tool & material is selected
        bool IsToolAndMaterialSelected();
        Vec3 GetToolScale();
        
    } tools;

    // ImGui Methods (returns true if changed)
    // Tool dropdown
    bool DrawTool();
    // Material dropdown
    bool DrawMaterial(); 
    // Popup to allow creating new tools / materials
    bool DrawPopup(); 

private:
    // How many (tools or materials) can fit in a list
    int listHeight      = 5; // in items
    // Width of (tool or materials) list
    float listWidth     = 300.0f;
    // Spacer between list & specs
    float xSpacer       = 30.0f;
    
    // ImGui visualisation of tools / materials
    int Draw_SelectTool();
    void Draw_ToolData();
    int Draw_SelectMaterial();
    void Draw_MaterialData();
};

} // end namespace Sqeak
