#include "functions.h"
using namespace std;


/*
    before and after every move we should:

    return to z Plane
    set absolute
*/


void FunctionGCodes::Add(std::string gcode) {
    m_gcodes.push_back(gcode);
}
void FunctionGCodes::Clear() {
    m_gcodes.clear();
}
void FunctionGCodes::MoveToZPlane() {
    m_gcodes.push_back("G91 (Incremental Mode)");
    m_gcodes.push_back("G28 Z0 (Move To ZPlane)");
    m_gcodes.push_back("G90 (Absolute Mode)");
}
void FunctionGCodes::MoveToMachineZero() {
    m_gcodes.push_back("G91 (Absolute Mode)");
    m_gcodes.push_back("G28 Z0 (Move To ZPlane)");
    m_gcodes.push_back("G28 X0 Y0 (Move To Machine Zero)");
    m_gcodes.push_back("G90 (Absolute Mode)");
}



void FunctionType_FacingCut::DrawPopup(GRBLVals &grblVals)
{
    if (ImGui::BeginPopup(ImGuiName().c_str())) {
        //popupOpen = true;
        
        ImGui::TextUnformatted("Facing Cut");
        ImGui::InputFloat3("Start", &m_Params.p0[0]);
        HereButton(grblVals, m_Params.p0);
        ImGui::InputFloat3("End", &m_Params.p1[0]); 
        HereButton(grblVals, m_Params.p1);
        ImGui::InputFloat("Cut Depth", &m_Params.zSpacing, 0.1f, 1.0f, "%.2f");
        
        ImGui::InputFloat("Plunge Feed Rate", &m_Params.plungeFeedRate, 0.1f, 1.0f, "%.2f");
        ImGui::InputFloat("Cutting Feed Rate", &m_Params.cutFeedRate, 0.1f, 1.0f, "%.2f");
        ImGui::InputFloat("Overlap Cut", &m_Params.cutOverlap, 0.1f, 1.0f, "%.2f");
        
        if(ImGui::Button("Show", ImVec2(100, 40))) 
        {
            std::pair<bool, vector<string>> gcodes = ExportGCode();
        
            if(gcodes.first) {
                Event_Update3DModelFromVector data = { gcodes.second };
                Event<Event_Update3DModelFromVector>::Dispatch(data); 
            } else {
                Log::Error("3D Viewer could not interpret this GCode");
            }
        }
        
        ImGui::EndPopup();
    } 
    /*else { // if popup has just been closed
        if (popupOpen == true) {
            //exportFunctionPopups();
            popupOpen = false;
        }
    }*/
}
    
std::pair<bool, std::vector<std::string>> FunctionType_FacingCut::ExportGCode() {
    
    FacingCut_Parameters& p = m_Params;
    
    glm::vec3 pDif = p.p1 - p.p0;
    
    if(pDif.x == 0 || pDif.y == 0) {
        Log::Error("Invalid start and end points");
        return make_pair(false, std::vector<std::string>());
    }
    
    FunctionGCodes gcodes;
    // initalise
    gcodes.Clear();
    gcodes.MoveToZPlane();
    gcodes.Add("G90 (Absolute Mode)");
    // move to initial x & y position
    gcodes.Add(va_str("G0 X%f Y%f (Move To Initial X & Y)", p.p0.x, p.p0.y));
    
//if x first:
   
    int zDirection = ((p.p1.z - p.p0.z) > 0) ? FORWARD : BACKWARD;
    float zCurrent = p.p0.z;
    
    do {
        // move to next z
        gcodes.Add(va_str("G1 Z%f F%f (Move To Z)", zCurrent, p.plungeFeedRate));
        // cut plane
        FacingCutXY(p, gcodes);
        // error check
        if(p.zSpacing <= 0.0f) {
            Log::Error("Spacing must have value");
            return make_pair(false, std::vector<std::string>());
        }
        if(zCurrent == p.p1.z) {
            break;
        }
        zCurrent += zDirection * p.zSpacing;
        
        if((zDirection == FORWARD && zCurrent > p.p1.z) || (zDirection == BACKWARD && zCurrent < p.p1.z)) {
            zCurrent = p.p1.z;
        }
    } while(1);

    // move to zPlane
    gcodes.MoveToMachineZero();
    
    return make_pair(true, gcodes.Get());
}

// executes length in one axis and then moves width of cutter in other axis
void FunctionType_FacingCut::FacingCutXY(FacingCut_Parameters& p, FunctionGCodes& gcodes) 
{
    //int xDirection = (pDif.x > 0) ? FORWARD : BACKWARD;
    int yDirection = ((p.p1.y - p.p0.y) > 0) ? FORWARD : BACKWARD;
    bool forward = true;
    
    glm::vec3 pNext = p.p0; 
    
    do {
        // x direction
        pNext.x = (forward) ? p.p1.x : p.p0.x;
        gcodes.Add(va_str("G1 X%f F%f", pNext.x, p.cutFeedRate));
        
        // y direction
        if(pNext.y == p.p1.y) {
            break;
        }
        float cutSpacing = m_ParametersGeneral.CutterDiam - p.cutOverlap;
        pNext.y += yDirection * cutSpacing;
        
        if((yDirection == FORWARD && pNext.y > p.p1.y) || (yDirection == BACKWARD && pNext.y < p.p1.y)) {
            pNext.y = p.p1.y;
        }
        gcodes.Add(va_str("G1 Y%f F%f", pNext.y, p.cutFeedRate));
    
        // 
        forward = !forward;
    } while(1);
    
}

Functions::Functions() {
    m_FacingCut = std::make_unique<FunctionType_FacingCut>(m_ParametersGeneral);
    //m_FacingCut2 = std::make_unique<FunctionType_FacingCut>(m_ParametersGeneral);
    m_FunctionTypes.push_back(m_FacingCut.get());
    //m_FunctionTypes.push_back(m_FacingCut2.get());
}
 
void Functions::Draw(GRBL &grbl, GRBLVals &grblVals) {
    
    (void)grbl; (void) grblVals;
     
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
    if (!ImGui::Begin("Functions", NULL)) {
        ImGui::End(); 
        return;
    }
    // Disable all widgets when not connected to grbl
    BeginDisableWidgets(grblVals);
     
    Draw_Generic();
    ImGui::Separator();
    Draw_Functions();
    ImGui::Separator();
    Draw_ActiveFunctions(grblVals); 

    // Disable all widgets when not connected to grbl
    EndDisableWidgets(grblVals);
    ImGui::End();
}
  
void Functions::Draw_Generic() { 
    ImGui::TextUnformatted("General");
    ImGui::InputFloat("Cutter Diameter", &(m_ParametersGeneral.CutterDiam), 0.1f, 1.0f, "%.2f"); 
}  

void Functions::Draw_Functions() {
    for (size_t i = 0; i < m_FunctionTypes.size(); i++) { 
        bool clicked = m_FunctionTypes[i]->Draw();
        if(clicked) {
            cout << m_FunctionTypes[i]->Name() << endl;
            AddActive(m_FunctionTypes[i]);
        }
    }
}
 
void Functions::Draw_ActiveFunctions(GRBLVals& grblVals) {
    (void)grblVals;
    for (size_t i = 0; i < m_ActiveFunctions.size(); i++) {
        bool clicked = m_ActiveFunctions[i]->DrawActive(i);
        if(clicked) {
            ImGui::OpenPopup(m_ActiveFunctions[i]->ImGuiName().c_str());
        }
        m_ActiveFunctions[i]->DrawPopup(grblVals);
    }
} 


