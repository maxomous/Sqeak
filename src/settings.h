#pragma once
#include "common.h"
#include <initializer_list>

namespace Sqeak { 
    using namespace MaxLib::Vector;
    
    
/*

// a vector wrapper which allows a specific item to be currently selected
template<typename T>
class  Vector_SelectablePtrs
{
public:
    Vector_SelectablePtrs() {};
    Vector_SelectablePtrs(std::initializer_list<T> items) : m_Items(items) {};
    void Add(const T& v)                { m_Items.push_back(std::move(v)); m_CurrentIndex = m_Items.size()-1; } // lvalue & refs
    void Add(T&& v)                     { m_Items.push_back(std::move(v)); m_CurrentIndex = m_Items.size()-1; } // rvalue
    void Remove(size_t index) { 
        assert(index < m_Items.size()); 
        m_Items.erase(m_Items.begin() + index);
        if(m_CurrentIndex != -1 && (int)index <= m_CurrentIndex) 
            m_CurrentIndex--; 
    }
    void RemoveCurrent()                { assert(m_CurrentIndex > -1 && m_CurrentIndex < (int)m_Items.size()); Remove(m_CurrentIndex); }
    T& CurrentItem()                    { assert(m_CurrentIndex > -1 && m_CurrentIndex < (int)m_Items.size()); return m_Items[m_CurrentIndex]; }
    T& Item(size_t index)               { assert(index < m_Items.size()); return m_Items[index]; }
    T& operator [](size_t i)            { return Item(i); }
    // moves value of n_next onto n, and n_new onto n
    void ItemSwap(size_t n, size_t n_Next) {   
        assert(n < m_Items.size()); 
        assert(n_Next < m_Items.size()); 
        T itemBuf = std::move(m_Items[n]);                
        m_Items[n] = m_Items[n_Next];
        m_Items[n_Next] = std::move(itemBuf);
    }
    size_t Size()                       { return m_Items.size(); }
    const std::vector<T>& Data()        { return m_Items; }
    std::vector<T>* DataPtr()           { return &m_Items; }
    T* ItemPtr(size_t index)            { assert(index < m_Items.size()); return &m_Items[index]; }
    void SetCurrentIndex(int index)     { assert(index >= -1 && index < (int)m_Items.size()); m_CurrentIndex = index; }
    int& CurrentIndex()                 { return m_CurrentIndex; }
    bool HasItemSelected()              { return m_CurrentIndex != -1; }
private:
    std::vector<T> m_Items;
    int m_CurrentIndex = -1;
};
*/



struct ParametersList
{
    // define settings here
    struct System {
        std::string     serialDevice        = "/dev/ttyS0"; // "/dev/ttyAMA0"
        std::string     serialBaudrate      = "115200";
        std::string     curDir              = MaxLib::File::ThisDir();
        std::string     saveFileDirectory   = MaxLib::File::ThisDir();
    } system;
    
   
    struct Viewer3DParameters {
        
        struct General {
            glm::vec3 BackgroundColour      = { 0.45f, 0.55f, 0.60f };            
        } general;
        
        struct Toolpath {
            glm::vec3 Colour_Feed   = { 0.694f, 0.063f, 0.048f };
            glm::vec3 Colour_FeedZ  = { 0.843f, 0.612f, 0.068f };
            glm::vec3 Colour_Rapid  = { 0.154f, 0.734f, 0.000f };
            glm::vec3 Colour_Home   = { 0.160f, 0.396f, 0.722f };
        } toolpath;
        
        struct Axis {
            float Size                  = 50.0f;
        } axis;
        
        struct Grid {
            glm::vec3 Position          = { 0.0f, 0.0f, 0.0f };
            glm::vec2 Size              = { 1200.0f, 600.0f };
            float Spacing               =   100.0f;
            glm::vec3 Colour            = { 0.6f, 0.6f, 0.6f };
        } grid;
        
        struct Spindle {
            bool visibility = true;
            
            struct SpindleColour {
                glm::vec3 tool              = { 0.698f, 0.127f, 0.127f };
                glm::vec3 toolOutline       = { 0.1f, 0.1f, 0.1f };
                glm::vec3 toolHolder        = { 0.294f, 0.294f, 0.294f };
                glm::vec3 toolHolderOutline = { 0.1f, 0.1f, 0.1f };                
            } colours;
            
        } spindle;
        
    } viewer;

    struct Sketch
    {
        struct Point {
            float size                  = 3.0f;
            glm::vec3 colour            = { 0.615f, 0.810f, 0.219f };
            glm::vec3 colourActive      = { 0.815f, 0.500f, 0.419f };
        } point;
        
        struct Line {
            glm::vec3 colour            = { 0.246f, 0.246f, 0.246f };
            glm::vec3 colourDisabled    = { 0.440f, 0.440f, 0.440f };
        } line;
        
        struct Cursor {
            struct Popup {
                bool shouldOpen = false;
            } popup;
            
            std::optional<glm::vec2> Position_Snapped;              // snapped 2d mouse position in current coord sys
            std::optional<glm::vec2> Position_Clicked;      // snapped 2d mouse click position in current coord sys
            std::optional<glm::vec2> Position_Raw;          // raw 2d mouse position in current coord sys
            std::optional<glm::vec2> Position_WorldCoords;  // snapped 2d mouse position in world space
    
            glm::vec3 Colour            = { 0.9f, 0.9f, 0.9f };
            
            float Size                  = 14.0f; // mm
            float Size_Scaled;          // gets updated with change in zoom
            float SnapDistance          = 5.0f; // mm
            float SnapDistance_Scaled;  // gets updated with change in zoom
            glm::vec2 SnapCursor(const glm::vec2& cursorPos) {
                return roundVec2(SnapDistance_Scaled, cursorPos);
            }
            float SelectionTolerance    = 10.0f;
            float SelectionTolerance_Scaled; 
            
        } cursor;
    } sketch;

    struct PathCutter {
        // Tab Parameters
        bool CutTabs                    = true;
        float TabSpacing                = 50.0f;
        float TabHeight                 = 4.0f;
        float TabWidth                  = 8.0f;
        
        float CutOverlap                = 1.0f;     // mm
        float PartialRetractDistance    = 1.0f; // mm
        GeosBufferParams geosParameters;
                   
    } pathCutter;
    
    struct CustomGCode {
        std::string name;
        std::string gcode;
    };
    std::vector<CustomGCode> customGCodes;
    // List of tools and materials. includes imgui methdos
    ToolSettings toolSettings;
};


enum ButtonType { Primary, Secondary, Connect, New, Edit, ToolbarButtonPrimary, ToolbarButton, ToolbarSketchButton, ToolbarConstraintButton, ToolbarHeader, ToolbarBack, Jog };
enum Colour     { Text, HeaderText };

struct ButtonDimension { 
    ImVec2 Size; 
    ImVec2 ImageSize;
};

// internal settings (not added to user settings ini file)
struct GUISettings 
{   
    
    /*             
    struct _
    {
        ButtonDimension dimension;
        Font* font;
        ImageTexture&
    };
    */
    GUISettings()
    {
        button[ButtonType::Primary]                 = {{ 90.0f, 36.0f }, { 16.0f, 16.0f }};   // Primary
        button[ButtonType::Secondary]               = {{ 60.0f, 31.0f }, { 16.0f, 16.0f }};   // Secondary
        button[ButtonType::Connect]                 = {{ 73.0f, 63.0f }, { 24.0f, 24.0f }};   // Functions
        button[ButtonType::New]                     = {{ 28.0f, 28.0f }, { 16.0f, 16.0f }};   // New
        button[ButtonType::Edit]                    = {{ 12.0f, 12.0f }, { 12.0f, 12.0f }};   // Edit
        button[ButtonType::ToolbarButtonPrimary]    = {{ 80.0f, 63.0f }, { 24.0f, 24.0f }};   // toolbar main item
        button[ButtonType::ToolbarButton]           = {{ 56.0f, 63.0f }, { 24.0f, 24.0f }};   // toolbar general item
        button[ButtonType::ToolbarSketchButton]     = {{ 46.0f, 63.0f }, { 24.0f, 24.0f }};   // toolbar sketch item
        button[ButtonType::ToolbarConstraintButton] = {{ 70.0f, 50.0f }, { 24.0f, 24.0f }};   // toolbar constraint item
        button[ButtonType::ToolbarHeader]           = {{ 88.0f, 63.0f }, { 28.0f, 28.0f }};   // toolbar header item
        button[ButtonType::ToolbarBack]             = {{ 47.0f, 46.0f }, { 24.0f, 24.0f }};   // toolbar back button
        button[ButtonType::Jog]                     = {{ 18.0f, 18.0f }, { 12.0f, 12.0f }};   // Jog
    }
    
    
    ButtonDimension button[12];       //      Button Size,      Image Size
    float functionButtonTextOffset  = 52.0f;
    float functionButtonImageOffset = 8.0f;

    float dockPadding               =   20.0f;
    float toolbarHeight             =   164.0f;
    float toolbarTableHeight        =   91.0f; // toolbarHeight - 62.0f
    float toolbarTableScrollbarSize =   12.0f;
    float toolbarSpacer             =   22.0f;
    float toolbarItemHeight         =   65.0f;
    float toolbarWidgetWidth        =   80.0f;
    
    ImVec2 FramePosition_UnderToolbar();
    //ImVec2 FramePosition_BottomOfScreen() { return { dockPadding, ImGui::GetMainViewport()->WorkSize.y - windowSize.y - dockPadding }; } //under toolbar
        
    float widgetWidth               =   180.0f; // general widget width
        
    float popupPosition_alpha       =   0.6f;
    float popupPosition_offsetPos   =   12.0f;
        
        
    float popupMessage_YSpacing     =   40.0f;  // distance between popup messages
    float popupMessage_Time         =   2.0f;   // time to display popup messages
    uint popupMessage_MaxCount      =   5;      // max no. popup messages to display
    uint max_FilePathDisplay        =   50;     // max no. characters to display in open file string
        
    // colours  
    ImVec4 colour[2]                = { { 1.0f,     1.0f,   1.0f,   1.0f },   // Text
                                        { 0.659f,   0.745f, 0.620f, 1.0f } }; // HeaderText

    // Fonts
    ImFont* font_small;
    ImFont* font_medium;
    ImFont* font_large;

    // these need to be intialised in imgui_Settings()
    ImageTexture img_Icon;
    ImageTexture img_Restart;
    ImageTexture img_Play;
    ImageTexture img_Pause;
    ImageTexture img_Settings;
    ImageTexture img_Edit;
    ImageTexture img_Add;
    ImageTexture img_Open;
    ImageTexture img_Connect;
    ImageTexture img_ArrowUp;
    ImageTexture img_ArrowDown;
    ImageTexture img_ArrowLeft;
    ImageTexture img_ArrowRight;
    // sketch images
    ImageTexture img_Sketch;
    ImageTexture img_Sketch_Draw;
    ImageTexture img_Sketch_Measure;
    ImageTexture img_Sketch_Select;
    ImageTexture img_Sketch_SelectLoop;
    ImageTexture img_Sketch_Point;
    ImageTexture img_Sketch_Line;
    ImageTexture img_Sketch_Arc;
    ImageTexture img_Sketch_Circle;
    
    // sketch constraints
    ImageTexture img_Sketch_Constraint_Coincident;
    ImageTexture img_Sketch_Constraint_Midpoint;
    ImageTexture img_Sketch_Constraint_Vertical;
    ImageTexture img_Sketch_Constraint_Horizontal;
    ImageTexture img_Sketch_Constraint_Parallel;
    ImageTexture img_Sketch_Constraint_Perpendicular;
    ImageTexture img_Sketch_Constraint_Tangent;
    ImageTexture img_Sketch_Constraint_Equal;
    ImageTexture img_Sketch_Constraint_Distance;
    ImageTexture img_Sketch_Constraint_Radius;
    ImageTexture img_Sketch_Constraint_Angle;

    
};

// for setting static variables
class Setting 
{    
public:
    Setting(const std::string& Name, uint Id) : name(Name), id(Id) {}
    template <typename T>
    void AddParameter(const std::string paramName, T dataLocation) { 
        data.push_back(std::make_pair(paramName, dataLocation)); 
    }
    template <typename T>
    void AddParameterWithPrefix(const std::string name, uint index, T dataLocation) {    
        AddParameter(PrefixName(index, name), dataLocation);
    }
    std::string PrefixName(uint index, const std::string& name) {
        std::stringstream stream;
        stream << "<" << index << ">"  << name;
        return stream.str();
    }
    
    std::string GetParamName(size_t i);
    auto GetDataLocation(size_t i);
    
    std::string name;
    uint id;
    std::vector<std::pair<std::string, std::variant<bool*, char*, int*, float*, double*, std::string*, glm::vec2*, glm::vec3*, void*>>> data;
};

// for setting dynamic vectors of variables
// creates Settings object for each element
class DynamicSetting
{
public:
    DynamicSetting(const std::string& name, void* data, 
        std::function<size_t(void* data)> cb_GetSize,
        std::function<void(Setting& setting, void* data, uint id)> cb_AddParameters,
        std::function<void(void* data, std::string& name, uint id, std::string& paramName)>  cb_UpdateVectorSize
    );
    
private:
    std::string m_Name;
    void* m_Data;  
    // callbacks
    std::function<size_t(void* data)> m_cb_GetSize;
    std::function<void(Setting& setting, void* data, uint id)> m_cb_AddParameters;
    std::function<void(void* data, std::string& name, uint id, std::string& paramName)> m_cb_UpdateVectorSize;
    
    size_t GetSize();
    void AddParameters(Setting& setting, size_t index);
    void UpdateVectorSize(std::string name, size_t index, std::string& paramName);
    std::string Name();
    
    friend class Settings;
};

enum class ViewerUpdate {
    None            = 0x00,
    Clear           = 0x01,
    Sketch          = 0x02,
    ActiveDrawing   = 0x04,
    ActiveFunction  = 0x08,
    Full            = ActiveDrawing | ActiveFunction
};
inline ViewerUpdate operator|(ViewerUpdate a, ViewerUpdate b) { return static_cast<ViewerUpdate>(static_cast<int>(a) | static_cast<int>(b)); }

// main class - Contains all settings / dynamic settings
class Settings 
{    
public:
    Settings(const std::string& filename);
    
    void SaveToFile();
    int UpdateFromFile();
    
    // viewer update
    ViewerUpdate GetUpdateFlag() { return m_UpdateFlag; }
    void SetUpdateFlag(ViewerUpdate flag) { m_UpdateFlag = m_UpdateFlag | flag; }
    void ResetUpdateFlag() { m_UpdateFlag = ViewerUpdate::None; }

    GRBLVals grblVals;
    ParametersList p;
    GUISettings guiSettings;
private:
// *************************************************************************************************************
// ********************************************* USER SETTINGS *************************************************
    void AddSettings();
    void AddDynamicSettings();

// ****************************************** END OF USER SETTINGS *********************************************
// *************************************************************************************************************
        
private:
    ViewerUpdate m_UpdateFlag = ViewerUpdate::None;
    std::string m_Filename;
    std::vector<Setting> m_SettingsList;
    std::vector<DynamicSetting> m_VectorList;
    
    std::string GetSettingString(Setting& setting, size_t paramIndex);
    void CheckVectorIsBigEnough(std::string& name, uint id, std::string& paramName);
    int SetParameter(std::string& name, uint id, std::string& paramName, std::string& dataString);
    int GetVectorIndex(std::string name);
    void SetSettingFromString(Setting& setting, size_t paramIndex, std::string& dataString);
    // removes old settings from and updates m_SettingsList to the contents of customGCodes
    void UpdateDynamicSettings();
    void UpdateDynamicSetting(DynamicSetting& dSetting);

};


} // end namespace Sqeak
