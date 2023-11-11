#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


#include "../../libs/qrcodegen.h"
#include "func_qrcode.h"
using namespace qrcodegen;
using namespace std;
             
void FunctionType_QRCode::DrawPopup(Settings& settings)
{
    ImGui::InputText("Name", &m_Name);
    
    ImGui::Dummy(ImVec2());
    
    bool isChanged = false;
    
    isChanged |= ImGui::InputText("QR Code Text", &m_Params.text);
    
    isChanged |= ImGui::InputFloat3("Start", &m_Params.p0[0]);
    isChanged |= ImGuiModules::HereButton(settings.grblVals, m_Params.p0);
    isChanged |= ImGui::InputFloat("Size", &m_Params.size); 
    
    isChanged |= ImGui::Checkbox("Invert", &m_Params.invert);
    isChanged |= ImGui::InputInt("Border", &m_Params.border);
    
    // calls Export GCode and updates viewer
    if(isChanged) {
        Update3DView(settings);
    }
}
        
bool FunctionType_QRCode::IsValidInputs(Settings& settings) 
{
    // check tool and material is selected
    if(!settings.p.tools.IsToolAndMaterialSelected())
        return false;
    // z top and bottom
   /* if(m_Params.p1.z > m_Params.p0.z) {
        Log::Error("Z Bottom must be below or equal to Z Top");
        return false;
    }*/
    // cut depth should have value
    ParametersList::Tools::Tool::ToolData& toolData = settings.p.tools.toolList.CurrentItem().Data.CurrentItem();
    if(toolData.cutDepth <= 0.0f) {
        Log::Error("Cut Depth must be positive");
        return false;
    }
    if(m_Params.size <= 0) {
        Log::Error("Size must be greater than 0");
        return false;
    }
    if(m_Params.border < 0) {
        Log::Error("Border must be zero or greater");
        return false;
    }
    
    return true;
}   

std::string FunctionType_QRCode::HeaderText(Settings& settings) 
{
    QRCode_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
    
    // write header
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tStarting: " << p.p0 << '\n';
    stream << "; \tWith Size: " << p.size << '\n';
    
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    stream << '\n';
    
    return stream.str();
}
 
 
 
 
 
 
   
void FunctionType_QRCode::CutSquare(Settings& settings, FunctionGCodes& gcodes, const glm::vec2& pCentre, float size, float z)
{
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
    
    if(size == tool.Diameter) {
        // just drill a hole
        gcodes.Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", pCentre.x, pCentre.y));
        gcodes.Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", z, toolData.feedPlunge));
        // pull out ;)
        gcodes.MoveToZPlane();  
        return;
    } 
    
    
    float halfSize = size / 2.0f;
    // outer boundary
    glm::vec2 p0 = pCentre + glm::vec2(-halfSize, +halfSize);
    glm::vec2 p1 = pCentre + glm::vec2(+halfSize, -halfSize);
    
    float r = tool.Diameter / 2.0f;
    glm::vec2 pSquare0 = p0 + glm::vec2(r, -r);
    glm::vec2 pSquare1 = p1 + glm::vec2(-r, r);
    
    
        
    if(size > 2.0f * tool.Diameter) {
        // face first, then square
        glm::vec2 pFace0 = p0 + glm::vec2(tool.Diameter, -tool.Diameter);
        glm::vec2 pFace1 = p1 + glm::vec2(-tool.Diameter, tool.Diameter);
        // move to start position and feed in
        gcodes.Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", pFace0.x, pFace0.y));
        gcodes.Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", z, toolData.feedPlunge));
        // cut inside of square
        gcodes.FacingCutXY(settings, pFace0, pFace1);
    } else {
        // move to start position and feed in
        gcodes.Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", pSquare1.x, pSquare1.y));
        gcodes.Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", z, toolData.feedPlunge));
    }
    
    // cut square
    gcodes.Add(va_str("G1 X%.3f Y%.3f F%.0f", pSquare1.x, pSquare1.y, toolData.feedCutting));
    gcodes.Add(va_str("G1 X%.3f F%.0f", pSquare0.x, toolData.feedCutting));
    gcodes.Add(va_str("G1 Y%.3f F%.0f", pSquare0.y, toolData.feedCutting));
    gcodes.Add(va_str("G1 X%.3f F%.0f", pSquare1.x, toolData.feedCutting));
    gcodes.Add(va_str("G1 Y%.3f F%.0f", pSquare1.y, toolData.feedCutting));
    
    // pull out ;)
    gcodes.MoveToZPlane(); 
}

std::pair<bool, std::vector<std::string>> FunctionType_QRCode::ExportGCode(Settings& settings) 
{
    auto err = make_pair(false, std::vector<std::string>());;
    // error check
    if(!IsValidInputs(settings)) {
        return err;
    }

    //QRCode_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
    
    // initalise
    FunctionGCodes gcodes;
    gcodes.Add(HeaderText(settings));
    gcodes.InitCommands(toolData.speed);
    
  
	
	const QrCode qr = QrCode::encodeText(m_Params.text.c_str(), (QrCode::Ecc)3);
	//PrintQr(settings, gcodes, m_Params.p0.z, qr3);
	
    //float size = 20.0f;
    int border = m_Params.border;
    // in px
    float totalSize = (float)qr.getSize() + 2.0f * (float)border;
    // in mm
    float pxSize = (float)m_Params.size / totalSize;
    
    
    // check pixel is big enough
    if(pxSize < tool.Diameter) {
        Log::Error("Size of pixel is smaller than the tool, smallest size possible with current tool is %f", tool.Diameter * totalSize);
        return err;
    }
    
    //std::vector<glm::vec2> path;
    
	for (int y = -border; y < qr.getSize() + border; y++) 
    {
		for (int x = -border; x < qr.getSize() + border; x++) 
        {
            bool isColoured = qr.getModule(x, y);
			if((m_Params.invert && isColoured) || (!m_Params.invert && !isColoured))
            {
                glm::vec2 position = glm::vec2(m_Params.p0.x, m_Params.p0.y) + pxSize * glm::vec2(x+border, y+border);
                CutSquare(settings, gcodes, position, pxSize, m_Params.p0.z);
                
                /*path.push_back({ position.x - pxSize/2.0f, position.y - pxSize/2.0f });
                path.push_back({ position.x + pxSize/2.0f, position.y - pxSize/2.0f });
                path.push_back({ position.x + pxSize/2.0f, position.y + pxSize/2.0f });
                path.push_back({ position.x - pxSize/2.0f, position.y + pxSize/2.0f });
                path.push_back({ position.x - pxSize/2.0f, position.y - pxSize/2.0f });
                * */
            }
		}
	} 
    
    // move to zPlane, end program
    gcodes.EndCommands();
    
    // draw path and offset path in viewer
    //Event<Event_DisplayShapeOffset>::Dispatch( { path, {}, false } );
    
    return make_pair(true, gcodes.Get());
}

    
