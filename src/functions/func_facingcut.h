#pragma once
#include "../common.h"

class FunctionType_FacingCut : public FunctionType
{
    typedef struct {
        glm::vec3 p0; 
        glm::vec3 p1;
        int isYFirst = false;
    } FacingCut_Parameters;

public:
    FunctionType_FacingCut() : FunctionType("Facing Cut") { } 
    FunctionType_FacingCut(uint count) : FunctionType("Facing Cut " + std::to_string(count)) { } 
    ~FunctionType_FacingCut() override {}
    
    void DrawPopup(Settings& settings) override;
    std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) override;
    
    std::unique_ptr<FunctionType> CreateNew() override
    {
        static uint counter = 0;
        std::unique_ptr<FunctionType_FacingCut> newFunction = std::make_unique<FunctionType_FacingCut>(++counter);
        return move(newFunction);
    }
    
private:
    FacingCut_Parameters m_Params;
    
    bool IsValidInputs(Settings& settings);
    std::string HeaderText(Settings& settings);
};
