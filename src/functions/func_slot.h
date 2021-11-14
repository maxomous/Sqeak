#pragma once
#include "../common.h"

class FunctionType_Slot : public FunctionType
{
    typedef struct {
        glm::vec3 p0; 
        glm::vec3 p1;
        int compensateCutter = CompensateCutter::None;
    } Slot_Parameters;

public:
    FunctionType_Slot() : FunctionType("Slot") { } 
    FunctionType_Slot(uint count) : FunctionType("Slot " + std::to_string(count)) { } 
    ~FunctionType_Slot() override {}
    
    void DrawPopup(Settings& settings) override;
    std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) override;
    
    std::unique_ptr<FunctionType> CreateNew() override
    {
        static uint counter = 0;
        std::unique_ptr<FunctionType_Slot> newFunction = std::make_unique<FunctionType_Slot>(++counter);
        return move(newFunction);
    }
    
private:
    Slot_Parameters m_Params;
};
