#pragma once
#include "common.h" 


struct ParametersGeneral {
    float CutterDiam = 6.0f;
};


class FunctionGCodes
{
public:
    void Add(std::string gcode);
    void Clear();
    void MoveToZPlane();
    void MoveToMachineZero();
    
    std::vector<std::string> Get() {
        return m_gcodes;
    };
private:
    std::vector<std::string> m_gcodes;
};


class FunctionType
{    
public:
    FunctionType(const std::string name, ParametersGeneral& generalParams) 
        : m_Name(name), m_ParametersGeneral(generalParams)
    { 
        std::stringstream label;
        label << m_Name << "##" << (int)this;
        m_ImGuiName = label.str();
        
        std::cout << "Creating FunctionType: " << m_Name << " : " << m_ImGuiName << std::endl; 
    } 
    virtual ~FunctionType() {}
    std::string Name() { return m_Name; }
    std::string ImGuiName() { return m_ImGuiName; }
    
    bool Draw() {
        return ImGui::Button(m_ImGuiName.c_str(), ImVec2(100, 40));
    }
    bool DrawActive(int id) {
        std::ostringstream label;;
        label << m_Name << " " << id + 1;
        return ImGui::Button(label.str().c_str(), ImVec2(100, 40));
    }
    virtual void DrawPopup(GRBLVals &grblVals) { (void)grblVals; };

private:
    std::string m_Name; 
    std::string m_ImGuiName; 
    
protected:
    ParametersGeneral& m_ParametersGeneral; 
};

class FunctionType_FacingCut : public FunctionType
{
    typedef struct {
        glm::vec3 p0; 
        glm::vec3 p1;
        float zSpacing          = 2.0f;
        float plungeFeedRate    = 100.0f; // mm/min  
        float cutFeedRate       = 1000.0f; // mm/min
        float cutOverlap        = 1.0f; // mm
    } FacingCut_Parameters;

public:
    FunctionType_FacingCut(ParametersGeneral& generalParams) : FunctionType("Facing Cut", generalParams) { } 
    ~FunctionType_FacingCut() override {}
    void DrawPopup(GRBLVals &grblVals) override;
    
    std::pair<bool, std::vector<std::string>> ExportGCode();

private:
    FacingCut_Parameters m_Params;
    
    // executes length in one axis and then moves width of cutter in other axis
    void FacingCutXY(FacingCut_Parameters& p, FunctionGCodes& gcodes);
};


class Functions
{
    
public:
    Functions();
    ~Functions() {
        for (FunctionType* activeFunction : m_ActiveFunctions) {
            delete activeFunction;
        }
    }
    void AddActive(FunctionType* functionType) {
        (void)functionType;
        std::cout << "We need to make a new instance of the function type here" << std::endl;
        
        //if(id = 0)
        FunctionType_FacingCut* newFunction = new FunctionType_FacingCut(m_ParametersGeneral);
        m_ActiveFunctions.push_back(newFunction);
    }
    
    void Draw(GRBL &grbl, GRBLVals &grblVals);

protected:
    ParametersGeneral m_ParametersGeneral; 
    
private:
    std::unique_ptr<FunctionType_FacingCut> m_FacingCut;
    //std::unique_ptr<FunctionType_FacingCut> m_FacingCut2;
    
    std::vector<FunctionType*> m_FunctionTypes;
    std::vector<FunctionType*> m_ActiveFunctions;
    
    void Draw_Generic();
    void Draw_Functions();
    void Draw_ActiveFunctions(GRBLVals& grblVals);
};



/*
class FunctionItem
{
public:
    FunctionItem(std::string name) : m_Name(name) {}
    virtual ~FunctionItem() {
        std::cout << "destructor for FunctionItem Called" << std::endl;
    }
    
    virtual std::pair<bool, std::vector<std::string>> OutputGCode() = 0;
        
    std::string GetName() {
        return m_Name;
    }
    //virtual std::pair<bool, std::vector<std::string>> OutputGCode() = 0;
private:
    std::string m_Name;
};

typedef struct {
    glm::vec3 p0; 
    glm::vec3 p1;
    float cutterDiam;
    
    float zSpacing          = 2.0f;
    float plungeFeedRate    = 100.0f; // mm/min  
    float cutFeedRate       = 1000.0f; // mm/min
    float cutOverlap        = 1.0f; // mm
} FacingCut_Parameters;

class FunctionItem_FacingCut : public FunctionItem
{
public:
    FunctionItem_FacingCut(std::string name) : FunctionItem(name) {}
    ~FunctionItem_FacingCut() {
        std::cout << "destructor for FunctionItem_FacingCut Called" << std::endl;
    }
    void SetParameters(FacingCut_Parameters& parameters) { m_Parameters = parameters; }
    
    
    std::pair<bool, std::vector<std::string>> OutputGCode() override {
        return FacingCut(m_Parameters);       
    }
private:
    
    FacingCut_Parameters m_Parameters;
    
    void FacingCutXY(FacingCut_Parameters& p, FunctionGCodes& gcodes);
    std::pair<bool, std::vector<std::string>> FacingCut(FacingCut_Parameters& p);
};


class FunctionStack
{
public:
    FunctionStack() {}
    ~FunctionStack() {
        for(FunctionItem* item : m_FunctionItems) {
            delete item;
            std::cout << "Deleting item" << std::endl;
        }
        //delete[] m_FunctionItems;
    }
    void Add(FunctionItem* ptr) {
        m_FunctionItems.push_back(ptr);
    }
    
    std::vector<std::string> GetNames() {
        std::vector<std::string> names;
        for(FunctionItem* item : m_FunctionItems) {
            names.push_back(item->GetName());
        }
        return names;
    }
    
    void printGCodes() 
    {
        std::vector<std::string> functionGCodes;
        
        for(FunctionItem* item : m_FunctionItems) 
        {
            std::pair<bool, std::vector<std::string>> gcodes = item->OutputGCode();
            if(gcodes.first) {       
                functionGCodes.insert(functionGCodes.end(), gcodes.second.begin(), gcodes.second.end());
            } else {
                Log::Error("Could not compute Facing Cut");
            } 
        }
        
        if(functionGCodes.size() > 0) {
            Event<Event_Update3DModelFromVector>::Dispatch({ functionGCodes }); 
        }
    }
    
private:
    std::vector<FunctionItem*> m_FunctionItems;
};


struct Functions {
    FunctionStack m_FunctionStack;
    
    void Draw_General();
    void Draw_FunctionStack();
    
    void Draw_FunctionPopups(GRBLVals &grblVals);
    void Draw_FacingCut(GRBLVals &grblVals);
    
    void Draw(GRBL &grbl, GRBLVals &grblVals);
};





extern std::pair<bool, std::vector<std::string>> FacingCut(FacingCut_Parameters& p);

*/



