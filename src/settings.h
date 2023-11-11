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
            
            std::optional<glm::vec2> Position_Snapped;      // snapped 2d mouse position in current coord sys
            std::optional<glm::vec2> Position_Clicked;      // snapped 2d mouse click position in current coord sys
            std::optional<glm::vec2> Position_Raw;          // raw 2d mouse position in current coord sys
            std::optional<glm::vec2> Position_WorldCoords;  // snapped 2d mouse position in world space
    
            glm::vec3 Colour            = { 0.9f, 0.9f, 0.9f };
            
            float Size                  = 14.0f; // mm
            float Size_Scaled;          // gets updated with change in zoom
            
            float SelectionTolerance    = 10.0f;
            
            float SnapDistance          = 5.0f; // mm
            float SnapDistance_Scaled;  // gets updated with change in zoom
            glm::vec2 SnapCursor(const glm::vec2& cursorPos) {
                return roundVec2(SnapDistance_Scaled, cursorPos);
            }
            
        } cursor;
    } sketch;
    
    struct CustomGCode {
        std::string name;
        std::string gcode;
    };
    std::vector<CustomGCode> customGCodes;
    // List of tools and materials. includes imgui methdos
    ToolSettings toolSettings;
    // Parameters for geos buffer offset
    GeosCPP::Operation::OffsetParameters geosParameters;
};




enum Colour     { Text, HeaderText };


// internal settings (not added to user settings ini file)
struct GUISettings 
{   
    typedef ImGuiModules::ImageButtonStyle ImageButtonStyle;
    
    Vector_Ptrs<ImageButtonStyle> imageButtons;
    
    ImageButtonStyle* imageButton_Toolbar_Header;
    ImageButtonStyle* imageButton_Toolbar_HeaderInactive;
    ImageButtonStyle* imageButton_Toolbar_Header_Back;
    ImageButtonStyle* imageButton_Toolbar_Back;
    ImageButtonStyle* imageButton_Toolbar_LevelToggler;
    ImageButtonStyle* imageButton_Toolbar_Settings;
    ImageButtonStyle* imageButton_Toolbar_Connect;
    
    ImageButtonStyle* imageButton_Toolbar_ButtonPrimary;
    //ImageButtonStyle* imageButton_Toolbar_ButtonSecondary;
    ImageButtonStyle* imageButton_Toolbar_Button;
    ImageButtonStyle* imageButton_Toolbar_SketchPrimary;
    ImageButtonStyle* imageButton_Toolbar_Sketch;
    ImageButtonStyle* imageButton_Toolbar_Constraint;
    ImageButtonStyle* imageButton_Toolbar_Jog;
    ImageButtonStyle* imageButton_SubToolbar_Button;
    
    ImageButtonStyle* imageButton_Confirm;
    

    // Initialise the image buttons with fonts, sizes etc. These must be initialised after image fonts etc
    void InitialiseImageButtons() 
    {
        
        // Add Text Buttons                                                              Name                    Button Size                        Image Size          Font            Text / Image Offset
            
        imageButton_Toolbar_Header          = imageButtons.Addp(ImageButtonStyle("Toolbar Header",              ImVec2(78.0f, toolbarItemHeight),   imageSize_Huge,         font_small,     imageButtonOffset_Vertical));
        imageButton_Toolbar_HeaderInactive  = imageButtons.Addp(ImageButtonStyle("Toolbar HeaderInactive",      ImVec2(20.0f, toolbarItemHeight),   imageSize_Medium,       font_small,     { { CENTRED,  CENTRED },    { CENTRED,  -23.0f } }));
        
        imageButton_Toolbar_Header_Back     = imageButtons.Addp(ImageButtonStyle("Toolbar Header Back",         ImVec2(18.0f, 18.0f),               imageSize_Smaller,      font_small,     imageButtonOffset_Centred));
        
        imageButton_Toolbar_Back            = imageButtons.Addp(ImageButtonStyle("Toolbar Back",                ImVec2(28.0f, 40.0F),               imageSize_Medium,       font_small,     imageButtonOffset_Centred));
        imageButton_Toolbar_LevelToggler    = imageButtons.Addp(ImageButtonStyle("Toolbar Draw / Run Toggler",  ImVec2(93.0f, 40.0f),               imageSize_Large,        font_small,     imageButtonOffset_Horizontal));
        
        imageButton_Toolbar_Settings        = imageButtons.Addp(ImageButtonStyle("Toolbar Settings",            ImVec2(40.0f, 40.0f),               imageSize_Medium,       font_small,     imageButtonOffset_Centred));
        imageButton_Toolbar_Connect         = imageButtons.Addp(ImageButtonStyle("Toolbar Connect",             ImVec2(66.0f, 70.0f),               imageSize_Large,        font_small,     imageButtonOffset_Vertical));
        
        imageButton_Toolbar_ButtonPrimary   = imageButtons.Addp(ImageButtonStyle("Toolbar Button Primary",      ImVec2(66.0f, 70.0f),               imageSize_Large,        font_small,     imageButtonOffset_Vertical));
        //imageButton_Toolbar_ButtonSecondary = imageButtons.Addp(ImageButtonStyle("Toolbar Button Secondary",    ImVec2(56.0f, 60.0f),               imageSize_Large,    font_small,     imageButtonOffset_Vertical));
        imageButton_Toolbar_Button          = imageButtons.Addp(ImageButtonStyle("Toolbar Button",              ImVec2(56.0f, 50.0f),               imageSize_Large,        font_small,     imageButtonOffset_Vertical));
        imageButton_Toolbar_SketchPrimary   = imageButtons.Addp(ImageButtonStyle("Toolbar SketchPrimary",       ImVec2(70.0f, 70.0f),               imageSize_Large,        font_small,     imageButtonOffset_Vertical));
        imageButton_Toolbar_Sketch          = imageButtons.Addp(ImageButtonStyle("Toolbar Sketch",              ImVec2(46.0f, 60.0f),               imageSize_Large,        font_small,     imageButtonOffset_Vertical));
        imageButton_Toolbar_Constraint      = imageButtons.Addp(ImageButtonStyle("Toolbar Constraints",         ImVec2(70.0f, 50.0f),               imageSize_Large,        font_small,     imageButtonOffset_Constraint));
        imageButton_Toolbar_Jog             = imageButtons.Addp(ImageButtonStyle("Toolbar Jog",                 ImVec2(18.0f, 18.0f),               imageSize_Smaller,      font_small,     imageButtonOffset_Centred));
        imageButton_SubToolbar_Button       = imageButtons.Addp(ImageButtonStyle("Sub-Toolbar Button",          ImVec2(80.0f, 30.0f),               imageSize_SmallMedium,  font_small,     imageButtonOffset_HorizontalImageButton));
        imageButton_Confirm                 = imageButtons.Addp(ImageButtonStyle("Confirm Buttons",             ImVec2(24.0f, 33.0f),               imageSize_Small,        font_small,     imageButtonOffset_Centred));
   
    }
    

    // Standard image sizes
    ImVec2 imageSize_Smaller        = { 12.0f, 12.0f };
    ImVec2 imageSize_Small          = { 16.0f, 16.0f };
    ImVec2 imageSize_SmallMedium    = { 18.0f, 18.0f };
    ImVec2 imageSize_Medium         = { 20.0f, 20.0f };
    ImVec2 imageSize_Large          = { 24.0f, 24.0f };
    ImVec2 imageSize_Larger         = { 28.0f, 28.0f };
    ImVec2 imageSize_Huge           = { 32.0f, 32.0f };

    const float CENTRED = 0.0f;
    // Image Button text / image offsets                            Text Offset             Image Offset
    ImageButtonStyle::Offsets imageButtonOffset_Horizontal      = { { -3.0f,    CENTRED },  { 17.0f,     CENTRED } };
    ImageButtonStyle::Offsets imageButtonOffset_Vertical        = { { CENTRED,  17.0f },    { CENTRED,  -10.0f } };
    ImageButtonStyle::Offsets imageButtonOffset_Centred         = { { CENTRED,  CENTRED },  { CENTRED,  CENTRED } };
    ImageButtonStyle::Offsets imageButtonOffset_Constraint      = { { CENTRED,  12.0f },    { CENTRED,  -7.0f } };
    ImageButtonStyle::Offsets imageButtonOffset_HorizontalImageButton = { { 1,  CENTRED },    { -17.0f,  CENTRED } };
    
    
    




    float dockPadding               =   20.0f;
    float toolbarHeight             =   134.0f;
    float toolbarTableHeight        =   118.0f;
    float toolbarTableScrollbarSize =   12.0f;
    float toolbarSpacer             =   22.0f;
    float toolbarItemHeight         =   88.0f;
    float toolbarWidgetWidth        =   80.0f;
    float toolbarToolMaterialWidth  =   150.0f;
    
    ImVec2 FramePosition_UnderToolbar();
    //ImVec2 FramePosition_BottomOfScreen() { return { dockPadding, ImGui::GetMainViewport()->WorkSize.y - windowSize.y - dockPadding }; } //under toolbar
        
    float widgetWidth               =   100.0f; // general widget width
        
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
    ImageTexture img_Home;
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
    ImageTexture img_ArrowForward;
    ImageTexture img_ArrowBackward;
    ImageTexture img_Tick1;
    ImageTexture img_Tick2;
    ImageTexture img_Cross1;
    ImageTexture img_Cross2;
    ImageTexture img_Tool;
    ImageTexture img_GCode;
    ImageTexture img_2D;
    ImageTexture img_3D;
    
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

    ImageTexture img_Function_CutPath;
    ImageTexture img_Function_Drill;
    
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

enum class SqeakUpdate {
    None            = 0x00,
    Viewer          = 0x01,
    Full            = 0xFF
};
inline SqeakUpdate operator|(SqeakUpdate a, SqeakUpdate b) { return static_cast<SqeakUpdate>(static_cast<int>(a) | static_cast<int>(b)); }
inline bool operator&(SqeakUpdate a, SqeakUpdate b) { return static_cast<bool>(static_cast<int>(a) & static_cast<int>(b)); }



// main class - Contains all settings / dynamic settings
class Settings 
{    
public:
    Settings(const std::string& filename);
    
    void SaveToFile();
    int UpdateFromFile();
    
    // viewer update
    SqeakUpdate GetUpdateFlag() { return m_UpdateFlag; }
    void SetUpdateFlag(SqeakUpdate flag) { m_UpdateFlag = m_UpdateFlag | flag; }
    void ResetUpdateFlag() { m_UpdateFlag = SqeakUpdate::None; }

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
    SqeakUpdate m_UpdateFlag = SqeakUpdate::Full;
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
