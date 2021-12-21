#pragma once    
#include "../../common.h"

class ToolSettings
{
public:
    // returns true if changed
    bool Draw(Settings& settings);
    bool DrawPopup_Tools(Settings& settings); 
    int Draw_SelectTool(Settings& settings);
    void Draw_ToolData(Settings& settings);
    int Draw_SelectMaterial(Settings& settings);
    void Draw_MaterialData(Settings& settings);
    void Draw_TabParameters(Settings& settings);

private:
    float listWidth     = 300.0f;
    float xSpacer       = 30.0f;
    int listHeight      = 5; // in items
};
