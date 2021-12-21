#pragma once

#include "../../common.h"
class FunctionType_QRCode : public FunctionType
{
    typedef struct {
        glm::vec3 p0; 
        float size = 100.0f;

        std::string text = "https://www.blurb.com/b/4296031-poor-old-mr-kettle";
        bool invert = false;
        int border = 4;
    } QRCode_Parameters;

public:
    FunctionType_QRCode() : FunctionType("QRCode") { } 
    FunctionType_QRCode(uint count) : FunctionType("QRCode " + std::to_string(count)) { } 
    ~FunctionType_QRCode() override {}
    
    void DrawPopup(Settings& settings) override;
    std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) override;
    
    std::unique_ptr<FunctionType> CreateNew() override
    {
        static uint counter = 0;
        std::unique_ptr<FunctionType_QRCode> newFunction = std::make_unique<FunctionType_QRCode>(++counter);
        return move(newFunction);
    }
    
    void Update(glm::vec2 mouseClickPos) override { 
        (void) mouseClickPos; 
    }
private:
    QRCode_Parameters m_Params;
    
    // error checks
    bool IsValidInputs(Settings& settings);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings);
    void CutSquare(Settings& settings, FunctionGCodes& gcodes, const glm::vec2& pCentre, float size, float z);
    //void PrintQr(Settings& settings, FunctionGCodes& gcodes, float z, const QrCode &qr);
};
