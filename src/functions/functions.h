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
    // executes length in one axis and then moves width of cutter in other axis
    void FacingCutXY(Settings& settings, glm::vec2 p0, glm::vec2 p1, bool isYFirst = false);
     
    std::vector<std::string> Get() {
        return m_gcodes;
    };
private:
    std::vector<std::string> m_gcodes;
};

class FunctionsGeneral 
{
public:
    typedef struct {
        std::vector<glm::vec2> points;
        float z0;
        float z1;
        float cutDepth;
        float feedPlunge;
        float feedCutting;
        bool isLoop = false;
    } CutPathParams;
    // adds gcodes to cut a generic path or loop at depths between z0 and z1
    // assumes we are at a safe z position as first move is to move to params.points[0]
    // returns value on error
    static int CutPath(Settings& settings, FunctionGCodes& gcodes, const CutPathParams& params);
private:
    // determines the positions of tabs along path
    static std::vector<std::pair<size_t, glm::vec2>> GetTabPositions(Settings& settings, const CutPathParams& params);
    static void CheckForTab(Settings& settings, FunctionGCodes& gcodes, const CutPathParams& params, std::vector<std::pair<size_t, glm::vec2>> tabPositions, glm::vec2 pDif, float zCurrent, bool isMovingForward, int& tabIndex, size_t i);
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
    
    bool Draw(Settings& settings) {
        (void)settings;
        return ImGui::Selectable(m_ImGuiName.c_str());
        //return ImGui::Button(m_ImGuiName.c_str(), settings.guiSettings.buttonSize[0]);
    }
    bool DrawActive(Settings& settings) {
        return ImGui::Button(m_Name.c_str(), settings.guiSettings.buttonSize[0]);
    }
    virtual void DrawPopup(Settings& settings) { 
        (void)settings;
    }
    virtual std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) { (void)settings; return make_pair(false, std::vector<std::string>()); }

    virtual std::unique_ptr<FunctionType> CreateNew() { return nullptr; }
    
    int InterpretGCode(Settings& settings, std::function<void(std::vector<std::string> gcode)> callback);
    void Update3DView(Settings& settings);
    void SaveGCode(Settings& settings, std::string filepath);
    void RunGCode(GRBL& grbl, Settings& settings);
    
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
    
    void Draw_Functions(Settings& settings);
    void Draw_ActiveFunctions(GRBL& grbl, Settings& settings);
};
