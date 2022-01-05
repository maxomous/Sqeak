#pragma once
#include "../../common.h" 

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


class FunctionGCodes
{
public:

    // TODO this should inlclude a tool & tab settings, this would prevent needing to take Settings&
    typedef struct {
        std::vector<glm::vec2>* points;
        float z0;
        float z1;
        float cutDepth;
        float feedPlunge;
        float feedCutting;
        bool isLoop = false;
    } CutPathParams;
    
    void Add(std::string gcode);
    void Clear();
    void MoveToZPlane();
    void MoveToHomePosition();
    void InitCommands(float spindleSpeed);
    void EndCommands();
        
    std::vector<std::string> Get() {
        return m_gcodes;
    };
    
    // executes length in one axis and then moves width of cutter in other axis
    void FacingCutXY(Settings& settings, glm::vec2 p0, glm::vec2 p1, bool isYFirst = false);
    // adds gcodes to cut a generic path or loop at depths between z0 and z1
    // moves to safe z position, then moves to p0
    // returns value on error
    int CutPathDepths(Settings& settings, const CutPathParams& params);

private:
    std::vector<std::string> m_gcodes;
        // determines the positions of tabs along path
    std::vector<std::pair<size_t, glm::vec2>> GetTabPositions(Settings& settings, const CutPathParams& params);
    void CheckForTab(Settings& settings, const CutPathParams& params, std::vector<std::pair<size_t, glm::vec2>> tabPositions, glm::vec2 pDif, float zCurrent, bool isMovingForward, int& tabIndex, size_t i);
};
    
class FunctionType
{    
public:
    struct ImGuiElements {        
        FunctionType* m_Parent;
        ImGuiElements(FunctionType* parent) : m_Parent(parent) {};
    } ImGuiElement{this};

    FunctionType() : FunctionType("Function") { } 
    FunctionType(uint count) : FunctionType("Function " + std::to_string(count)) { } 

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
        return ImGui::Selectable(m_ImGuiName.c_str());
        //return ImGui::Button(m_ImGuiName.c_str(), settings.guiSettings.buttonSize[0]);
    }
    bool DrawActive(ImVec2 size, bool isCurrentItem) {        
        if(isCurrentItem) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
        bool isClicked = ImGui::Button(m_Name.c_str(), size);
        if(isCurrentItem) ImGui::PopStyleColor();
        return isClicked;
    }
    virtual void DrawPopup(Settings& settings) { 
        (void)settings;
    }
    virtual std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) { (void)settings; return make_pair(false, std::vector<std::string>()); }

    virtual std::unique_ptr<FunctionType> CreateNew() { return nullptr; }
    
    virtual void Update(glm::vec2 mouseClickPos) { (void) mouseClickPos; }
    
    int InterpretGCode(Settings& settings, std::function<int(std::vector<std::string> gcode)> callback);
    void Update3DView(Settings& settings);
    int SaveGCode(Settings& settings, std::string filepath);
    void RunGCode(GRBL& grbl, Settings& settings);
    
protected:
    std::string m_Name; 

private:
    std::string m_ImGuiName; 
    
};


class Functions
{
    
public:
    Functions(Settings& settings);
       
    
    //void Draw(GRBL& grbl, Settings& settings);
    void Draw_Functions(Settings& settings);
    void Draw_ActiveFunctions(Settings& settings);
    std::string ActiveFunctionName();
    void RunActiveFunction(GRBL& grbl, Settings& settings);
    void Update3DViewOfActiveFunction(Settings& settings);
    void DeleteActiveFunction(Settings& settings);
    void SaveActiveFunction(Settings& settings, std::string filename);
    bool IsActiveFunctionSelected(bool showWarning = true);
    void DeselectActive();
    std::string GetActiveFunctionFilepath(const std::string& folderPath);

private:
    std::vector<std::unique_ptr<FunctionType>> m_FunctionTypes;
    VectorSelectable<std::unique_ptr<FunctionType>> m_ActiveFunctions;
    
};
