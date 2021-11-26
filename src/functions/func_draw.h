#pragma once
#include "../common.h"

class FunctionType_Draw : public FunctionType
{
    typedef struct {
        glm::vec2 z;
        std::vector<glm::vec2> path;
        int cutSide = CompensateCutter::None;
    } Draw_Parameters;

public:
    FunctionType_Draw() : FunctionType("Draw") { } 
    FunctionType_Draw(uint count) : FunctionType("Draw " + std::to_string(count)) { 
        m_Params.path.push_back({ 50.0f, 50.0f });
        m_Params.path.push_back({ 150.0f, 50.0f });
        m_Params.path.push_back({ 150.0f, 150.0f });
        m_Params.path.push_back({ 50.0f, 150.0f });
    } 
    ~FunctionType_Draw() override {}
    
    void DrawPopup(Settings& settings) override;
    std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) override;
    
    std::unique_ptr<FunctionType> CreateNew() override
    {
        static uint counter = 0;
        std::unique_ptr<FunctionType_Draw> newFunction = std::make_unique<FunctionType_Draw>(++counter);
        return move(newFunction);
    }
    
private:
    Draw_Parameters m_Params;
    
    // error checks
    bool IsValidInputs(Settings& settings);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings);
};
