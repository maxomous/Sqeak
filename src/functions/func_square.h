#pragma once
#include "../common.h"

class FunctionType_Square : public FunctionType
{
    typedef struct {
        glm::vec3 p0; 
        glm::vec3 p1;
        int cutSide = CompensateCutter::None;
    } Square_Parameters;

public:
    FunctionType_Square() : FunctionType("Square") { } 
    FunctionType_Square(uint count) : FunctionType("Square " + std::to_string(count)) { } 
    ~FunctionType_Square() override {}
    
    void DrawPopup(Settings& settings) override;
    std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) override;
    
    std::unique_ptr<FunctionType> CreateNew() override
    {
        static uint counter = 0;
        std::unique_ptr<FunctionType_Square> newFunction = std::make_unique<FunctionType_Square>(++counter);
        return move(newFunction);
    }
    
private:
    Square_Parameters m_Params;
    
    // error checks
    bool IsValidInputs(Settings& settings);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings);
};
