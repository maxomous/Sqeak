#pragma once
#include "../common.h" 

/*
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();

    std::string material    = "Material";
    float speed             = 10000.0f;
    float feedCutting       = 1000.0f;
    float feedPlunge        = 300.0f;
    float cutDepth          = 1.0f;
    float cutWidth          = 1.0f;
    */

enum CompensateCutter {
    None, Left, Right
};


class FunctionGCodes
{
public:
    void Add(std::string gcode);
    void Clear();
    void MoveToZPlane();
    void MoveToHomePosition();
    void InitCommands(float spindleSpeed);
    void EndCommands();

    std::vector<std::string> Get() {
        return m_gcodes;
    };
private:
    std::vector<std::string> m_gcodes;
};


    
class FunctionType
{    
public:
    struct ImGuiElements 
    {        
        FunctionType* m_Parent;
        ImGuiElements(FunctionType* parent) : m_Parent(parent) {};
        
        bool Buttons_ViewRunDelete(GRBL& grbl, Settings& settings);
        
    } ImGuiElement{this};

    FunctionType() : FunctionType("Facing Cut") { } 
    FunctionType(uint count) : FunctionType("Facing Cut " + std::to_string(count)) { } 

    FunctionType(const std::string name) : m_Name(name)
    {   // create unique id
        std::stringstream label;
        label << name << "##" << (int)this;
        m_ImGuiName = label.str();
    } 
    virtual ~FunctionType() {}
    
    std::string Name() { return m_Name; }
    std::string ImGuiName() { return m_ImGuiName; }
    
    bool Draw() {
        return ImGui::Button(m_ImGuiName.c_str(), ImVec2(100, 40));
    }
    bool DrawActive() {
        return ImGui::Button(m_Name.c_str(), ImVec2(100, 40));
    }
    virtual void DrawPopup(Settings& settings) { 
        (void)settings;
    }
    virtual std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) { (void)settings; return make_pair(false, std::vector<std::string>()); }

    virtual std::unique_ptr<FunctionType> CreateNew() { return nullptr; }
    
protected:
    std::string m_Name; 

private:
    std::string m_ImGuiName; 
    
};


class Functions
{
    
public:
    Functions();
    void RemoveActive(size_t index) {
        if(index >= m_ActiveFunctions.size()) {
            //Log::Critical("Index out of range");
            return;
        }
        m_ActiveFunctions.erase(m_ActiveFunctions.begin() + index);
    }
    
    void Draw(GRBL& grbl, Settings& settings);
    
private:
    
    std::vector<std::unique_ptr<FunctionType>> m_FunctionTypes;
    std::vector<std::unique_ptr<FunctionType>> m_ActiveFunctions;
    
    void Draw_ToolSettings(Settings& settings);
    void Draw_Functions();
    void Draw_ActiveFunctions(GRBL& grbl, Settings& settings);
};
