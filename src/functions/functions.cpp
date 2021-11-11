#include "functions.h"
#include "func_facingcut.h"
#include "func_slot.h"

using namespace std;

void FunctionGCodes::Add(std::string gcode) {
    m_gcodes.push_back(gcode);
}
void FunctionGCodes::Clear() {
    m_gcodes.clear();
}
void FunctionGCodes::MoveToZPlane() {
    Add("G91\t(Incremental Mode)");
    Add("G28 Z0\t(Move To ZPlane)");
    Add("G90\t(Absolute Mode)");
}
void FunctionGCodes::MoveToHomePosition() {
    Add("G91\t(Incremental Mode)");
    Add("G28 Z0\t(Move To ZPlane)");
    Add("G28 X0 Y0\t(Move To Home Position)");
    Add("G90\t(Absolute Mode)");
}

void FunctionGCodes::InitCommands(float spindleSpeed) {
    // Add("G54 (Coord System 0)");
    Add("G17\t(Select XY Plane)");
    Add("G90\t(Absolute Mode)");
    Add("G94\t(Feedrate/min Mode)"); 
    Add("G21\t(Set units to mm)");
    
    MoveToZPlane();
    
    if(spindleSpeed) {
        Add("M3 S" + std::to_string((int)spindleSpeed) + "\t(Start Spindle)");
        Add("G4 P" + std::to_string((int)roundf(spindleSpeed / 2000.0f)) + "\t(Wait For Spindle)");
    }
    Add(""); // blank line
}

void FunctionGCodes::EndCommands() {
    Add(""); // blank line
    InitCommands(0);
    Add("M5\t(Stop Spindle)");
    MoveToHomePosition();
    Add("M30\t(End Program)");
}


bool FunctionType::ImGuiElements::Buttons_ViewRunDelete(GRBL& grbl, Settings& settings) 
{        
    // check if tool & material is selected
    auto isToolAndMaterialSelected = [&settings](){
        if(!settings.p.tools.toolList.HasItemSelected()) {
            Log::Error("No Tool Selected");
            return true;
        }
        if(!settings.p.tools.toolList.CurrentItem().Data.HasItemSelected()) {
            Log::Error("No Material Selected");
            return true;
        }
        return false;
    };
    
    if(ImGui::Button("View 3D", ImVec2(100, 40))) {
        
        if(isToolAndMaterialSelected())
            return false;
    
        std::pair<bool, vector<string>> gcodes = m_Parent->ExportGCode(settings);
        if(gcodes.first) {
            Event<Event_Update3DModelFromVector>::Dispatch( {gcodes.second} ); 
        } else {
            Log::Error("3D Viewer could not interpret this GCode");
        }
        
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Save", ImVec2(100, 40))) {
        
        if(isToolAndMaterialSelected())
            return false;
            
        std::pair<bool, vector<string>> gcodes = m_Parent->ExportGCode(settings);
        if(gcodes.first) {
            // add GCodes to file
            File::WriteArray("/home/pi/Desktop/GCODE test.nc", gcodes.second);
        } else {
            Log::Error("Could not execute this GCode");
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Run", ImVec2(100, 40))) {
        
        if(isToolAndMaterialSelected())
            return false;
            
        std::pair<bool, vector<string>> gcodes = m_Parent->ExportGCode(settings);
        if(gcodes.first) {
            // send gcodes to grbl
            if(grbl.sendArray(gcodes.second)) {
                Log::Error("Couldn't send file to grbl");
            }
        } else {
            Log::Error("Could not execute this GCode");
        }
    }
    
    ImGui::SameLine();
    
    if(ImGui::Button("Delete", ImVec2(100, 40))) {
        return true;
    }
    return false;
}


struct ToolSettings
{
    float listWidth     = 300.0f;
    float xSpacer       = 30.0f;
    int listHeight      = 5; // in items
    float hoverHeight   = 9.0f + listHeight; // in items
    
    bool simpleView = true; 
     
            
    void Draw(Settings& settings) 
    { 
        auto& tools = settings.p.tools;  
                
        ImGui::BeginGroup();
            static std::function<std::string(ParametersList::Tools::Tool& item)> cb_GetToolName = [](ParametersList::Tools::Tool& item) { return item.Name; };
            ImGuiModules::ComboBox("Select Tool", tools.toolList, cb_GetToolName);
            
            if(tools.toolList.HasItemSelected()) {
                static std::function<std::string(ParametersList::Tools::Tool::ToolData& item)> cb_GetMaterialName = [](ParametersList::Tools::Tool::ToolData& item) { return item.material; };
                ImGuiModules::ComboBox("Select Material", settings.p.tools.toolList.CurrentItem().Data, cb_GetMaterialName);
            } else {
                static int dummyCombo = 0;
                ImGui::Combo("Select Material", &dummyCombo, "\0");
            }
        ImGui::EndGroup();
        
        DrawEditTools(settings);
    }   
    
    void DrawEditTools(Settings& settings) 
    {
        ImGui::SameLine();
        float buttonSize = ImGui::GetTextLineHeight() * 2;
        ImGuiModules::CentreItemVertically(2, buttonSize);
        
        if (ImGui::Button("Edit", ImVec2(1.5f * buttonSize, buttonSize)))
            ImGui::OpenPopup("EditTools");
        
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("EditTools", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted("Select Tool");
            ImGui::BeginGroup();
            
                ImGui::BeginGroup(); 
                    if(Draw_SelectTool(settings)) {
                        ImGui::EndGroup(); ImGui::EndGroup(); ImGui::EndPopup(); return; // to prevent reading an element which doesnt exist anymore
                    }
                ImGui::EndGroup();
                
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer);
                
                ImGui::BeginGroup();
                    Draw_ToolData(settings);
                ImGui::EndGroup();
                
            ImGui::EndGroup();
             
            if(settings.p.tools.toolList.HasItemSelected()) 
            {
                ImGui::Separator();
            
                ImGui::TextUnformatted("Select Material");
                ImGui::BeginGroup();
                     
                    ImGui::BeginGroup();
                        if(Draw_SelectMaterial(settings)) {
                            ImGui::EndGroup(); ImGui::EndGroup(); ImGui::EndPopup(); return; // to prevent reading an element which doesnt exist anymore
                        }
                    ImGui::EndGroup();
                    
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer);
                      
                    ImGui::BeginGroup();
                        Draw_MaterialData(settings);
                    ImGui::EndGroup(); 
                    
                ImGui::EndGroup();
            }
            
            ImGui::Separator();
                
            ImGui::SetItemDefaultFocus();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) / 2.0f);
            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) { 
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::EndPopup();
        }
    }
    
    
          
    int Draw_SelectTool(Settings& settings)
    {
        auto& tools = settings.p.tools;  
        bool isToolSelected = tools.toolList.HasItemSelected();
         
        static std::function<std::string(ParametersList::Tools::Tool& item)> cb_GetToolName = [](ParametersList::Tools::Tool& item) { return item.Name; };
        ImGuiModules::DraggableListBox("##Tools", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), tools.toolList, cb_GetToolName);
        
        if(ImGui::Button("+##AddTool")) {
            tools.toolList.Add(ParametersList::Tools::Tool("Tool 1"));
            auto& lastTool = tools.toolList.Item(tools.toolList.Size()-1);
            lastTool.Data.Add(ParametersList::Tools::Tool::ToolData());
            return -1;
        }
        ImGui::SameLine();
        // grey out if no item selected
        if(!isToolSelected) {
            ImGuiModules::BeginDisableWidget();
        } 
        if(ImGui::Button("-##RemoveTool")) {
            if(isToolSelected) {
                tools.toolList.RemoveCurrent();
                return -1;
            }
        }
        if(!isToolSelected) {
            ImGuiModules::EndDisableWidget();
        }
        return 0;
    }
    
    void Draw_ToolData(Settings& settings)
    {
        auto& tools = settings.p.tools;  
        bool isToolSelected = tools.toolList.HasItemSelected();
        // if tool selected
        if(isToolSelected) 
        {
            ParametersList::Tools::Tool& currentTool = settings.p.tools.toolList.CurrentItem();
            ImGui::InputText("Name", &currentTool.Name);
            ImGui::InputFloat("Cutter Diameter", &currentTool.Diameter, 0.1f, 1.0f, "%.2f"); 
            ImGui::InputFloat("Tool Length", &currentTool.Length, 0.1f, 1.0f, "%.2f"); 
        }
    }
     

    int Draw_SelectMaterial(Settings& settings)
    {
        auto& tools = settings.p.tools;  
        bool isToolSelected = tools.toolList.HasItemSelected();
        bool isMaterialSelected = tools.toolList.CurrentItem().Data.HasItemSelected();  
        
        if(isToolSelected) 
        { 
            ParametersList::Tools::Tool& currentTool = settings.p.tools.toolList.CurrentItem();
                
            static std::function<std::string(ParametersList::Tools::Tool::ToolData& item)> cb_GetMaterialName = [](ParametersList::Tools::Tool::ToolData& item) { return item.material; };
            ImGuiModules::DraggableListBox("##ToolData", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), currentTool.Data, cb_GetMaterialName);
            
            if(ImGui::Button("+##AddToolData")) { 
                currentTool.Data.Add(ParametersList::Tools::Tool::ToolData());
                return -1;
            }
            ImGui::SameLine();
            
            // grey out if no material selected
            if(!isMaterialSelected)
                ImGuiModules::BeginDisableWidget();
                
            if(ImGui::Button("-##RemoveToolData")) {
                if(isMaterialSelected) {
                    currentTool.Data.RemoveCurrent();
                    return -1;
                }
            }
            
            if(!isMaterialSelected)
                ImGuiModules::EndDisableWidget();   
        }
        return 0;
    }
    void Draw_MaterialData(Settings& settings)
    {
        auto& tools = settings.p.tools;   
        bool isToolSelected = tools.toolList.HasItemSelected();
        bool isMaterialSelected = tools.toolList.CurrentItem().Data.HasItemSelected();  
    
        if(isToolSelected && isMaterialSelected) {
            ParametersList::Tools::Tool& currentTool = settings.p.tools.toolList.CurrentItem();
            auto& data = currentTool.Data.CurrentItem();
            
            ImGui::InputText("Material", &data.material); 
            ImGui::Separator();
            ImGui::InputFloat("Spindle Speed", &data.speed, 100.0f, 1000.0f, "%.0f"); 
            ImGui::Separator();
            ImGui::InputFloat("Cutting Feed Rate", &data.feedCutting, 100.0f, 1000.0f, "%.0f");
            ImGui::InputFloat("Plunge Feed Rate", &data.feedPlunge, 100.0f, 1000.0f, "%.0f");
            ImGui::Separator();
            ImGui::InputFloat("Depth of Cut", &data.cutDepth, 0.1f, 1.0f, "%.3f");
            ImGui::InputFloat("Width of Cut", &data.cutWidth, 0.1f, 1.0f, "%.3f");
        }
    }
};

Functions::Functions() 
{
    auto function_FacingCut = std::make_unique<FunctionType_FacingCut>();
    m_FunctionTypes.push_back(move(function_FacingCut));
    
    auto function_Slot = std::make_unique<FunctionType_Slot>();
    m_FunctionTypes.push_back(move(function_Slot));

}
 
void Functions::Draw(GRBL& grbl, Settings& settings) 
{
    static ToolSettings toolSettings;
    toolSettings.Draw(settings);
    ImGui::Separator();
    Draw_Functions();
    ImGui::Separator();
    Draw_ActiveFunctions(grbl, settings); 
}
  
void Functions::Draw_Functions() 
{
    ImGui::TextUnformatted("Add New Function");
    for (size_t i = 0; i < m_FunctionTypes.size(); i++) { 
        if(i > 0) ImGui::SameLine();
        // draw the function buttons
        bool clicked = m_FunctionTypes[i]->Draw();
        if(clicked) {
            //AddActive(m_FunctionTypes[i]); 
            m_ActiveFunctions.push_back(m_FunctionTypes[i]->CreateNew());
        }
    }
}   
 
void Functions::Draw_ActiveFunctions(GRBL& grbl, Settings& settings)   
{
    ImGui::TextUnformatted("Active Functions");
    for (size_t i = 0; i < m_ActiveFunctions.size(); i++)  
    {
        std::unique_ptr<FunctionType>& f = m_ActiveFunctions[i];
        
        // draw active functions and open popup if clicked
        if(f->DrawActive()) { 
            ImGui::OpenPopup(f->ImGuiName().c_str()); 
        }
         
        bool hasBeenDeleted = false;
        
        if (ImGui::BeginPopup(f->ImGuiName().c_str())) {
            //popupOpen = true;
            
            f->DrawPopup(settings);
            
            ImGui::Dummy(ImVec2());
            hasBeenDeleted = f->ImGuiElement.Buttons_ViewRunDelete(grbl, settings);
            
            ImGui::EndPopup();
        } 
        /*else { // if popup has just been closed
            if (popupOpen == true) {
                //exportFunctionPopups();
                popupOpen = false;
            } 
       }*/
       //return hasBeenDeleted;
        
   
        // draw active function popups (if visible)
        //int deleteThis = f->DrawPopup(grbl, settings);
        if(hasBeenDeleted) {
            RemoveActive(i);
        }
    }
} 

