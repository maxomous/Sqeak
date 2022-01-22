#pragma once    
#include "../common.h"

struct ImGuiCustomModules
{    
    class ImGuiWindow
    {
    public:
        ImGuiWindow(Settings& settings, std::string name, const ImVec2& defaultSize);
        bool Begin();
        void End();
    private:
        std::string m_Name;
        ImVec2 m_Size;
        ImVec2 m_Pos;
    };

    // Disable all widgets when not connected to grbl
    static void BeginDisableWidgets(GRBLVals& grblVals);
    static void EndDisableWidgets(GRBLVals& grblVals);
    static bool HereButton(GRBLVals& grblVals, glm::vec3& p);
    static bool ImageButtonWithText_Function(Settings& settings, std::string name, ImageTexture& image, bool isActive = false, ButtonType buttonType = ButtonType::FunctionButton);


};
