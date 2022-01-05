#include "functions.h"
#include "func_facingcut.h"
#include "func_slot.h"
#include "func_square.h"
#include "func_draw.h"
#include "func_qrcode.h"
 
using namespace std;

void FunctionGCodes::Add(std::string gcode) {
    m_gcodes.push_back(gcode);
}
void FunctionGCodes::Clear() {
    m_gcodes.clear();
}
void FunctionGCodes::MoveToZPlane() {
    Add("G91\t(Incremental Mode)");
    Add("G28 Z0\t(Move To ZPlane)");
    Add("G90\t(Absolute Mode)");
}
void FunctionGCodes::MoveToHomePosition() {
    Add("G91\t(Incremental Mode)");
    Add("G28 Z0\t(Move To ZPlane)");
    Add("G28 X0 Y0\t(Move To Home Position)");
    Add("G90\t(Absolute Mode)");
}

void FunctionGCodes::InitCommands(float spindleSpeed) {
    // Add("G54 (Coord System 0)");
    Add("G17\t(Select XY Plane)");
    Add("G90\t(Absolute Mode)");
    Add("G94\t(Feedrate/min Mode)"); 
    Add("G21\t(Set units to mm)");
    
    MoveToZPlane();
    
    if(spindleSpeed) {
        Add("M3 S" + std::to_string((int)spindleSpeed) + "\t(Start Spindle)");
        Add("G4 P" + std::to_string((int)roundf(spindleSpeed / 2000.0f)) + "\t(Wait For Spindle)");
    }
    Add(""); // blank line
}

void FunctionGCodes::EndCommands() {
    Add(""); // blank line
    InitCommands(0);
    Add("M5\t(Stop Spindle)");
    MoveToHomePosition();
    Add("M30\t(End Program)");
}

// executes length in one axis and then moves width of cutter in other axis
void FunctionGCodes::FacingCutXY(Settings& settings, glm::vec2 p0, glm::vec2 p1, bool isYFirst) 
{
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();
    
    bool forward = true;
    glm::vec2 pNext = p0; 
        
    if(isYFirst) {
        int xDirection = ((p1.x - p0.x) > 0) ? FORWARD : BACKWARD;
        
        do {
            // y direction
            pNext.y = (forward) ? p1.y : p0.y;
            Add(va_str("G1 Y%.3f F%.0f", pNext.y, toolData.feedCutting));
            
            // x direction
            if(pNext.x == p1.x) {
                break;
            }
            
            pNext.x += xDirection * toolData.cutWidth;
            
            if((xDirection == FORWARD && pNext.x > p1.x) || (xDirection == BACKWARD && pNext.x < p1.x)) {
                pNext.x = p1.x;
            }
            Add(va_str("G1 X%.3f F%.0f", pNext.x, toolData.feedCutting));
        
            // change direction
            forward = !forward;
        } while(1);
    } 
    else {
        int yDirection = ((p1.y - p0.y) > 0) ? FORWARD : BACKWARD;
        
        do {
            // x direction
            pNext.x = (forward) ? p1.x : p0.x;
            Add(va_str("G1 X%.3f F%.0f", pNext.x, toolData.feedCutting));
            
            // y direction
            if(pNext.y == p1.y) {
                break;
            }
            
            pNext.y += yDirection * toolData.cutWidth;
            
            if((yDirection == FORWARD && pNext.y > p1.y) || (yDirection == BACKWARD && pNext.y < p1.y)) {
                pNext.y = p1.y;
            }
            Add(va_str("G1 Y%.3f F%.0f", pNext.y, toolData.feedCutting));
        
            // change direction
            forward = !forward;
        } while(1);
    }
}
 

std::vector<std::pair<size_t, glm::vec2>> FunctionGCodes::GetTabPositions(Settings& settings, const CutPathParams& params)
{ 
    if(!settings.p.pathCutter.CutTabs) {
        return {}; 
    }
    float& tabSpacing = settings.p.pathCutter.TabSpacing;
    float& tabWidth   = settings.p.pathCutter.TabWidth;
    
    const std::vector<glm::vec2>* points = params.points;
    
    // Tab variables
    glm::vec2 p0 = (*points)[0];
    float distanceAtLastPoint = 0.0f;
    float nextTabPos = tabSpacing;
    int tabCount = 0;
    // vector to return
    std::vector<std::pair<size_t, glm::vec2>> tabPositions;
    
    for (size_t i = 1; i < points->size(); i++) 
    {
        glm::vec2 p1 = (*points)[i];
        // calculate distance
        glm::vec dif = p1-p0;
        float distance = hypotf(dif.x, dif.y);
        
        // make tab
        while(distanceAtLastPoint + distance > nextTabPos) {
            
            float tabPosAlongLine = nextTabPos - distanceAtLastPoint;
            
            // if the next tab falls too close to a corner, keep incrementing it until it's posible to produce
            float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
            float minDistance = (tabWidth/2.0f) + toolRadius + 1.0f; // +1mm just to be sure
            
            if(tabPosAlongLine < minDistance || distance-tabPosAlongLine < minDistance) {
                nextTabPos += 1.0f;
                continue;
            }
            
            glm::vec2 normalised = dif / distance;
            glm::vec2 tabPos = p0 + (tabPosAlongLine * normalised);
            
            // add tab position to vector //
            tabPositions.push_back(std::make_pair(i, tabPos));
            
            tabCount++;
            nextTabPos = tabSpacing * (float)(tabCount+1);
            // check tab is far enough away from p0 and p1
        }
        
        distanceAtLastPoint += distance;
        p0 = p1;
    }
    return move(tabPositions);
}

void FunctionGCodes::CheckForTab(Settings& settings, const CutPathParams& params, std::vector<std::pair<size_t, glm::vec2>> tabPositions, glm::vec2 pDif, float zCurrent, bool isMovingForward, int& tabIndex, size_t i) 
{
    if(!settings.p.pathCutter.CutTabs) {
        return; 
    }
    float& tabHeight  = settings.p.pathCutter.TabHeight;
    float& tabWidth   = settings.p.pathCutter.TabWidth;
    float tabZPos = params.z1 + tabHeight;
    
    auto addTab = [&]() {
        // get tab position
        glm::vec2& tabPosition = tabPositions[tabIndex].second;
        // calculate tab start / end
        float distance = hypotf(pDif.x, pDif.y);
        
        glm::vec2 normalised = pDif / distance;
        float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
        glm::vec2 tabOffset = ((tabWidth / 2.0f) +  toolRadius) * normalised;
        glm::vec2 tabStart = tabPosition - tabOffset;
        glm::vec2 tabEnd = tabPosition + tabOffset;
        
        // start of tab
        Add(va_str("G1 X%.3f Y%.3f Z%.3f F%.0f", tabStart.x, tabStart.y, zCurrent, params.feedCutting));
        Add(va_str("G1 Z%.3f F%.0f\t(Start Tab)", tabZPos, params.feedCutting));
        // end of tab
        Add(va_str("G1 X%.3f Y%.3f F%.0f", tabEnd.x, tabEnd.y, params.feedCutting));
        Add(va_str("G1 Z%.3f F%.0f\t(End Tab)", zCurrent, params.feedCutting));
    };
    // continue if below top of tab
    if(zCurrent >= tabZPos)
        return;
        
    // if moving forward along path
    if(isMovingForward && (tabIndex < (int)tabPositions.size())) { 
        // add a tab if index matches with position
        while(tabPositions[tabIndex].first == i) {
            addTab();
            if(++tabIndex >= (int)tabPositions.size()) {
                break;
            }
        }
    } // if moving backward along path
    else if(!isMovingForward && (tabIndex >= 0)) {   
        // add a tab if index matches with position
        while(tabPositions[tabIndex].first == params.points->size()-i) {
            addTab();
            if(--tabIndex < 0) {
                break;
            }
        }
    }
}
    
int FunctionGCodes::CutPathDepths(Settings& settings, const CutPathParams& params) {
    
    // error check
    if(params.points->size() < 2) {
        Log::Error("There should be 2 or more points");
        return -1;
    }
    if(params.feedPlunge == 0.0f) {
        Log::Error("Plunge feedrate requires a value");
        return -1;
    }
    if(params.feedCutting == 0.0f) {
        Log::Error("Cutting feedrate requires a value");
        return -1;
    }
    // get the positions of where tabs should lie and their indexes within points[]
    std::vector<std::pair<size_t, glm::vec2>> tabPositions = GetTabPositions(settings, params);
    
    float zCurrent = params.z0;
    int zDirection = ((params.z1 - params.z0) > 0) ? FORWARD : BACKWARD; // 1 or -1
    bool isMovingForward = true;
    
    const std::vector<glm::vec2>* points = params.points;
        
    do {
        // if first run or start and end points are different in loop (for pocketing)
        if((zCurrent == params.z0) || ((params.isLoop && (points->front() != points->back())))) {
            // move to safe z and then to the initial x & y position
            MoveToZPlane();
            Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", (*points)[0].x, (*points)[0].y));
        }
        // plunge to next z
        Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", zCurrent, params.feedPlunge));
        
        int tabIndex = (isMovingForward) ? 0 : tabPositions.size()-1;
        // Feed along path
        for (size_t i = 1; i < (*points).size(); i++) {
            // get start and end points of current line
            const glm::vec2& pLast = (isMovingForward) ? (*points)[i-1] : (*points)[points->size()-i];
            const glm::vec2& pNext = (isMovingForward) ? (*points)[i]   : (*points)[points->size()-i-1];
            // check for and draw tabs
            CheckForTab(settings, params, tabPositions, pNext-pLast, zCurrent, isMovingForward, tabIndex, i);
            // move to next point in linestring
            Add(va_str("G1 X%.3f Y%.3f F%.0f", pNext.x, pNext.y, params.feedCutting));
        }
        // reverse direction at end of linestring
        if(!params.isLoop) {
            isMovingForward = !isMovingForward;
        }
        // if we have reached the final z depth, break out of loop
        if(zCurrent == params.z1) {
            break;
        }
        // update z
        zCurrent += zDirection * fabsf(params.cutDepth);
        
        // if z zepth is further than final depth, adjust to final depth
        if((zDirection == FORWARD && zCurrent > params.z1) || (zDirection == BACKWARD && zCurrent < params.z1)) {
            zCurrent = params.z1;
        }
    } while(1);

    return 0;
}

int FunctionType::InterpretGCode(Settings& settings, std::function<int(std::vector<std::string> gcode)> callback)
{
    // error check
    if(settings.p.tools.IsToolAndMaterialSelected())
        return -1;
    // export gcode
    std::pair<bool, vector<string>> gcodes = ExportGCode(settings);
    // check if gcode could be read
    if(!gcodes.first) {
        Log::Error("Could not interpret this GCode");
        return -1;
    }
    // gcode was interpretted successfully, call function provided
    return callback(gcodes.second);
};

void FunctionType::Update3DView(Settings& settings) 
{
    int err = InterpretGCode(settings, [](auto gcodes){
        Event<Event_Update3DModelFromVector>::Dispatch({ gcodes }); 
        return 0;
    });
    if(err) { // clear screen
        Event<Event_Update3DModelFromVector>::Dispatch({ std::vector<std::string>(/*empty*/) }); 
        //Event<Event_Viewer_AddLineLists>::Dispatch( { std::vector<DynamicBuffer::DynamicVertexList>*(/*empty*/) } );
    }
}
int FunctionType::SaveGCode(Settings& settings, std::string filepath) 
{
    return InterpretGCode(settings, [&](auto gcodes){
        File::WriteArray(filepath, gcodes);
        return 0;
    });
}
void FunctionType::RunGCode(GRBL& grbl, Settings& settings) 
{
    InterpretGCode(settings, [&](auto gcodes){
        // start file timer
        Event<Event_ResetFileTimer>::Dispatch({});
        // send gocdes to grbl
        if(grbl.sendArray(gcodes)) {
            Log::Error("Couldn't send file to grbl");
            return -1;
        };
        return 0;
    });
}


Functions::Functions(Settings& settings) 
{
    (void)settings;   
    
    auto function_FacingCut = std::make_unique<FunctionType_FacingCut>();
    m_FunctionTypes.push_back(move(function_FacingCut));
    
    auto function_Slot = std::make_unique<FunctionType_Slot>();
    m_FunctionTypes.push_back(move(function_Slot));
    
    auto function_Square = std::make_unique<FunctionType_Square>();
    m_FunctionTypes.push_back(move(function_Square));
    
    auto function_Draw = std::make_unique<FunctionType_Draw>();
    m_FunctionTypes.push_back(move(function_Draw));
    
    auto function_QRCode = std::make_unique<FunctionType_QRCode>();
    m_FunctionTypes.push_back(move(function_QRCode));

}

/*
void Functions::Draw(GRBL& grbl, Settings& settings) 
{ 
    (void)grbl; (void)settings;
    static ToolSettings toolSettings;
    
    ImGui::BeginGroup();
        // updates viewer if tool or material is changed
        if(toolSettings.Draw(settings)) {
            Update3DViewOfActiveFunction(settings);
        }
    ImGui::EndGroup();
    
    sameLineSeperator();

    ImGui::BeginGroup();
        Draw_Functions(settings); 
    ImGui::EndGroup();
    
    ImGui::SameLine();
    
    ImGui::BeginGroup();
        Draw_ActiveFunctions(settings); 
    ImGui::EndGroup();
        
}*/

void Functions::Draw_Functions(Settings& settings) 
{  
    ImGui::BeginGroup();
        ImVec2& buttonSize      = settings.guiSettings.button[ButtonType::New].Size;
        ImVec2& buttonImgSize   = settings.guiSettings.button[ButtonType::New].ImageSize;
        ImGuiModules::CentreItemVertically(2, buttonSize.y);
        if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_Add)) {
            ImGui::OpenPopup("addFunctionPopup");
        }
    ImGui::EndGroup();
    
    
    bool openNewFunctionPopup = false;
    
    // draw popup
    if (ImGui::BeginPopup("addFunctionPopup")) {
        for (size_t i = 0; i < m_FunctionTypes.size(); i++) { 
            bool clicked = m_FunctionTypes[i]->Draw();
            if(clicked) {
                m_ActiveFunctions.Add(m_FunctionTypes[i]->CreateNew());
                Update3DViewOfActiveFunction(settings);
                openNewFunctionPopup = true;
            }
        }
        ImGui::EndPopup();
    }    
    if(openNewFunctionPopup) {
        if(m_ActiveFunctions.HasItemSelected()) {
            ImGui::OpenPopup(m_ActiveFunctions.CurrentItem()->ImGuiName().c_str());
        } else {
            Log::Error("Somehow, no function is active...");
        }
    }
}   
 
void Functions::Draw_ActiveFunctions(Settings& settings)   
{
    ImVec2& buttonSize = settings.guiSettings.button[ButtonType::Primary].Size;
    
    ImGui::BeginGroup();
        ImGuiModules::CentreItemVertically(2, buttonSize.y);
        
        for (size_t i = 0; i < m_ActiveFunctions.Size(); i++)  
        {
            if(i > 0) {
                ImGui::SameLine();
            }
            std::unique_ptr<FunctionType>& f = m_ActiveFunctions.Item(i);
            
            // draw active functions and make selected if clicked
            bool isCurrentItem = ((int)i == m_ActiveFunctions.CurrentIndex());
            if(f->DrawActive(buttonSize, isCurrentItem) || ImGuiModules::RightClickedLastItem()) { 
                m_ActiveFunctions.SetCurrentIndex(i);
                Update3DViewOfActiveFunction(settings);
                ImGui::OpenPopup(f->ImGuiName().c_str()); 
            }  
            if (ImGui::BeginPopup(f->ImGuiName().c_str())) {
                //popupOpen = true;
                f->DrawPopup(settings);
                ImGui::EndPopup();
            } 
            /*else { // if popup has just been closed
                if (popupOpen == true) {
                    //exportFunctionPopups();
                    popupOpen = false;
                }  
           }*/
        }
    ImGui::EndGroup();
} 

std::string Functions::ActiveFunctionName() 
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        return "";
    }
    auto& currentFunction = m_ActiveFunctions.CurrentItem();
    return currentFunction->Name();
}

void Functions::RunActiveFunction(GRBL& grbl, Settings& settings) 
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        Log::Error("No active function selected");
        return;
    }
    auto& currentFunction = m_ActiveFunctions.CurrentItem();
    currentFunction->RunGCode(grbl, settings);
}

void Functions::Update3DViewOfActiveFunction(Settings& settings)
{
    if(m_ActiveFunctions.HasItemSelected()) {
        // Clear path and offset path in 3d viewer
        //Event<Event_DisplayShapeOffset>::Dispatch( { std::vector<glm::vec2>(/*empty*/), std::vector<glm::vec2>(/*empty*/), false } );
        m_ActiveFunctions.CurrentItem()->Update3DView(settings);
    }
}

void Functions::DeleteActiveFunction(Settings& settings)
{
    if(m_ActiveFunctions.HasItemSelected()) {
        m_ActiveFunctions.RemoveCurrent();
        Update3DViewOfActiveFunction(settings);
    }
}

void Functions::SaveActiveFunction(Settings& settings, std::string filename)
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        Log::Error("No active function selected");
        return; 
    }
    if(m_ActiveFunctions.CurrentItem()->SaveGCode(settings, filename)) {
        Log::Error("Unable to save file: %s", filename.c_str());
    }
}

bool Functions::IsActiveFunctionSelected(bool showWarning)
{    
    if(!m_ActiveFunctions.HasItemSelected()) {
        if(showWarning) 
            Log::Error("No active function selected");
        return false;
    }
    return true;
}
void Functions::DeselectActive()
{
    m_ActiveFunctions.SetCurrentIndex(-1);
}

std::string Functions::GetActiveFunctionFilepath(const std::string& folderPath)
{    
    if(!m_ActiveFunctions.HasItemSelected()) {
        Log::Error("No active function selected");
        return "";
    }
    std::string filePath = folderPath;
    // make sure theres a / at the end of folder
    if(filePath.empty()) filePath += '/';
    if(filePath.back() != '/') filePath += '/';
    
    filePath += m_ActiveFunctions.CurrentItem()->Name();
    filePath += ".nc";
    return filePath;
}
