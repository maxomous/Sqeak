
#include <iostream>
#include "func_draw.h"
using namespace std; 

   
void FunctionType_Draw::DrawPopup(Settings& settings)
{ 
    (void)settings;
    
    ImGui::InputText("Name", &m_Name);
     
    ImGui::Dummy(ImVec2());
     
    bool isChanged = false;
    isChanged |= ImGui::Combo("Cut Side", &m_Params.cutSide, "None\0Left\0Right\0Pocket\0\0");
    isChanged |= ImGui::InputFloat("Finishing Pass", &m_Params.finishingPass);
    isChanged |= ImGui::InputFloat2("Z Top/Bottom", &m_Params.z[0]);
    
    ImGui::Separator(); 
    
    ImGui::PushItemWidth(settings.guiSettings.inputBoxWidth);
        isChanged |= m_Params.drawing.DrawImGui();
    ImGui::PopItemWidth();

    if(ImGui::Button("+ Line##AddPoint")) {
        m_Params.drawing.AddLine(glm::vec2(0.0f, 0.0f));
        isChanged = true;
    }
    ImGui::SameLine();
    
    if(ImGui::Button("+ Arc##AddPoint")) {
        m_Params.drawing.AddArc(glm::vec2(0.0f, 0.0f), CLOCKWISE, 100.0f);
        isChanged = true;
    }
    ImGui::SameLine();
    
    if(ImGui::Button("-##RemovePoint")) {
        if(!m_Params.drawing.IsEmpty()) {
            m_Params.drawing.DeleteLastElement();
            isChanged = true;
        }
    }
    
    // calls Export GCode and updates viewer
    if(isChanged) {
        Update3DView(settings);
    }
}
          
bool FunctionType_Draw::IsValidInputs(Settings& settings) 
{
    // check tool and material is selected
    if(settings.p.tools.IsToolAndMaterialSelected())
        return false;
    // start and end point
    if(m_Params.drawing.Size() == 0) {
        Log::Error("At least 1 drawing segment is required");
        return false;
    }
    // z top and bottom
    if(m_Params.z[1] > m_Params.z[0]) {
        Log::Error("Z Bottom must be below or equal to Z Top");
        return false;
    }
    // cut depth should have value
    ParametersList::Tools::Tool::ToolData& toolData = settings.p.tools.toolList.CurrentItem().Data.CurrentItem();
    if(toolData.cutDepth <= 0.0f) {
        Log::Error("Cut Depth must be positive");
        return false;
    }
    return true;
}   

std::string FunctionType_Draw::HeaderText(Settings& settings) 
{
    Draw_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
    
    // write header
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.z[0] << " and " << p.z[1] << '\n';
    stream << "; \tPoints:" << '\n';
    
    m_Params.drawing.AddElementsHeaderText(stream);
    
  /*  if(p.cutSide == CompensateCutter::None) stream << "; \tCompensate: None\n";
    if(p.cutSide == CompensateCutter::Left) stream << "; \tCompensate: Left\n";
    if(p.cutSide == CompensateCutter::Right) stream << "; \tCompensate: Right\n";
    if(p.cutSide == CompensateCutter::Pocket) stream << "; \tCompensate: Pocket\n";
    */
    if(p.finishingPass) stream << "; Finishing Pass: " << p.finishingPass << '\n';
    
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    stream << '\n';
    
    return stream.str();
}
    
std::pair<bool, std::vector<std::string>> FunctionType_Draw::ExportGCode(Settings& settings) 
{
    auto err = make_pair(false, std::vector<std::string>());;
    return err;
    // error check
    if(!IsValidInputs(settings)) {
        return err;
    }
  /*  
    //Draw_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
    */
    // initalise
    //FunctionGCodes gcodes;
    //gcodes.Add(HeaderText(settings));
    //gcodes.InitCommands(toolData.speed);
     
    // define offset path parameters
    // 0 = no compensation, 1 = compensate left / pocket, -1 = compensate right
   /* int cutSide;
    if(m_Params.cutSide == CompensateCutter::None)
        cutSide = 0;
    if(m_Params.cutSide == CompensateCutter::Right)
        cutSide = -1; 
    if(m_Params.cutSide == CompensateCutter::Left || m_Params.cutSide == CompensateCutter::Pocket)
        cutSide = 1;
    
    float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
    float offset = fabsf(toolRadius) + m_Params.finishingPass;
    // define cut path parameters & offset path
    Geos geos;
    // number of line segments per 90 degrees of arc
    int arcSegments = settings.p.pathCutter.geosParameters.QuadrantSegments;
    // make a path of line segments (arcs are converted to many line segments)
    std::vector<glm::vec2> path = m_Params.drawing.Path(arcSegments);
    // calculate offset
    FunctionGCodes::CutPathParams pathParams;

    // populate parameters
    pathParams.z0 = m_Params.z[0];
    pathParams.z1 = m_Params.z[1];
    pathParams.cutDepth = toolData.cutDepth;
    pathParams.feedPlunge = toolData.feedPlunge;
    pathParams.feedCutting = toolData.feedCutting;
    pathParams.isLoop = m_Params.drawing.IsLoop();
    */
  /*  // populate offset path
    if(m_Params.drawing.IsLoop()) 
    {  
        
        #define MAX_ITERATIONS 500
        #define CUT_OVERLAP 1 // mm
        // dont treat as loop as we are putting all offsets on same vector
        pathParams.isLoop = false;
        // bore out the internal of a shape
        // cut the shape repeatly whilst increasing the offset until the entire shape is gone
        int nIterations = 0;
        
        while(1) {
            cout << "running" << endl;
            // add offset to points list
            if(!geos.OffsetPolygon_AddToVector(pathParams.points, path, cutSide * offset, arcSegments)) {
                cout << "end of offsets" << endl;
                // err if failed on first attempt, break otherwise as we have finished boring
                if(nIterations) { break; }
                else { return err; }
            }
            // repeat if we're boring out entire shape
            if(m_Params.cutSide != CompensateCutter::Pocket) { break; }
            if(++nIterations > MAX_ITERATIONS) { 
                Log::Error("Iterations is maxed out"); 
                break;
            }
            // move tool in 
            offset += 2.0f * fabsf(toolRadius) - CUT_OVERLAP;
        };
        // add gcodes for path at depths
        if(gcodes.CutPathDepths(settings, pathParams)) {
            return err;
        }
        
    } 
    else  
    {
        if(m_Params.cutSide == CompensateCutter::Pocket) { 
            Log::Error("Pocket must be a loop");
            return err; 
        }
        
        if(!geos.OffsetLine_AddToVector(pathParams.points, path, cutSide * offset, arcSegments)) {
            return err;
        }
        // add gcodes for path at depths
        if(gcodes.CutPathDepths(settings, pathParams)) {
            return err;
        }
    }
    
    // add gcodes for finishing pass
    if(m_Params.finishingPass) {
        // make a new set of parameters where z doesnt chance
        FunctionGCodes::CutPathParams finishPassParams = pathParams;
        // clear the points vector
        finishPassParams.points.clear();
        finishPassParams.z0 = finishPassParams.z1;
        if(!geos.OffsetPolygon_AddToVector(finishPassParams.points, path, cutSide * toolRadius, arcSegments)) {
            return err; // not sure why it would fail
        }
        // add gcodes for path at depths
        if(gcodes.CutPathDepths(settings, finishPassParams)) {
            return err;
        }
    }
    // move to zPlane, end program
    gcodes.EndCommands();
    
    // draw path and offset path in viewer
    //Event<Event_DisplayShapeOffset>::Dispatch( { path, pathParams.points, pathParams.isLoop } );
    */
    return err;
}

    
