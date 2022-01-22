#pragma once

#include <initializer_list>

// a vector wrapper which allows a specific item to be currently selected
template<typename T>
class  VectorSelectable
{
public:
    VectorSelectable() {};
    VectorSelectable(std::initializer_list<T> items) : m_Items(items) {};
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


struct ParametersList
{
    // define settings here
    struct System {
        std::string     serialDevice        = "/dev/ttyS0"; // "/dev/ttyAMA0"
        std::string     serialBaudrate      = "115200";
        std::string     curDir              = File::ThisDir();
        std::string     saveFileDirectory   = File::ThisDir();
    } system;
    

    struct Tools 
    { 
        struct Tool 
        {
            struct ToolData {
                std::string material    = "Material";
                float speed             = 10000.0f;
                float feedCutting       = 1000.0f;
                float feedPlunge        = 300.0f;
                float cutDepth          = 1.0f;
                float cutWidth          = 1.0f;
            };
            
            Tool(std::string name = "Tool", float diameter = 6.0f, float length = 20.0f) : Name(name), Diameter(diameter), Length(length) {}
            
            VectorSelectable<ToolData> Data;
            std::string Name;
            float Diameter;
            float Length;    
        };
        VectorSelectable<Tool> toolList;
        // check if tool & material is selected
        bool IsToolAndMaterialSelected();
        glm::vec3 GetToolScale();
        
    } tools;

    struct Viewer3DParameters {
        
        glm::vec3 BackgroundColour      = { 0.45f, 0.55f, 0.60f };
        glm::vec3 ToolpathColour_Feed   = { 0.133f, 0.133f, 0.133f };
        glm::vec3 ToolpathColour_Rapid  = { 0.810f, 0.706f, 0.000f };
        glm::vec3 ToolpathColour_Home   = { 0.154f, 0.734f, 0.000f };
        
        struct ViewerPoint {
            float size              = 3.0f;
            glm::vec3 colour        = { 0.615f, 0.810f, 0.219f };
        } point;
        
        struct ViewerLine {
            glm::vec3 colour            = { 0.099f, 0.778f, 0.794f };
            glm::vec3 colourDisabled    = { 0.403f, 0.403f, 0.403f };
        } line;
        
        struct Cursor {
            glm::vec3 Colour        = { 0.9f, 0.9f, 0.9f };
            
            float Size                  = 14.0f; // mm
            float Size_Scaled;          // gets updated with change in zoom
            float SnapDistance          = 5.0f; // mm
            float SnapDistance_Scaled;  // gets updated with change in zoom
            glm::vec2 SnapCursor(const glm::vec2& cursorPos) {
                return roundVec2(SnapDistance_Scaled, cursorPos);
            }
            float SelectionTolerance    = 3.0f;
            float SelectionTolerance_Scaled; 
            
        } cursor;
        
        
        struct Axis {
            float Size = 50.0f;
        } axis;
        
        struct Grid {
            glm::vec3 Position  = { 0.0f, 0.0f, 0.0f };
            glm::vec2 Size      = { 1200.0f, 600.0f };
            float Spacing       =   100.0f;
            glm::vec3 Colour    = { 0.6f, 0.6f, 0.6f };
        } grid;
        
        struct Spindle {
            glm::vec3 toolColour = { 1.0f, 0.196f, 0.0 };
            glm::vec3 toolColourOutline = { 0.1f, 0.1f, 0.1f };
        } spindle;
        
    } viewer;

    struct PathCutter {
        // Tab Parameters
        bool CutTabs = true;
        float TabSpacing        = 50.0f;
        float TabHeight         = 4.0f;
        float TabWidth          = 8.0f;
        
        float CutOverlap        = 1.0f;     // mm
        
        GeosBufferParams geosParameters;
                   
    } pathCutter;
    
    struct CustomGCode {
        std::string name;
        std::string gcode;
    };
    std::vector<CustomGCode> customGCodes;
};


enum ButtonType { Primary, Secondary, Connect, New, Edit, FunctionButton, Jog };
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
        button[ButtonType::Primary]         = {{ 90.0f, 36.0f }, { 16.0f, 16.0f }};   // Primary
        button[ButtonType::Secondary]       = {{ 60.0f, 27.0f }, { 16.0f, 16.0f }};   // Secondary
        button[ButtonType::Connect]         = {{ 73.0f, 63.0f }, { 24.0f, 24.0f }};   // Functions
        button[ButtonType::New]             = {{ 28.0f, 28.0f }, { 16.0f, 16.0f }};   // New
        button[ButtonType::Edit]            = {{ 10.0f, 10.0f }, { 10.0f, 10.0f }};   // Edit
        button[ButtonType::FunctionButton]  = {{ 56.0f, 63.0f }, { 24.0f, 24.0f }};   // Functions
        button[ButtonType::Jog]             = {{ 19.0f, 19.0f }, { 16.0f, 16.0f }};   // Jog
    }
    
    ImGuiWindowFlags general_window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    
    ButtonDimension button[7];       //      Button Size,      Image Size
    float functionButtonTextOffset  = 52.0f;
    float functionButtonImageOffset = 8.0f;

    float dockPadding               =   20.0f;
    float toolbarHeight             =   164.0f;
    float toolbarTableHeight        =   102.0f; // toolbarHeight - 62.0f
    float toolbarTableScrollbarSize =   12.0f;
    float toolbarSpacer             =   22.0f;
    float toolbarItemHeight         =   65.0f;
    float toolbarComboBoxWidth      =   150.0f;
        
    float inputBoxWidth             =   140.0f;
        
    float widgetTextWidth           =   80.0f;
        
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
    ImageTexture img_Restart;
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
    ImageTexture img_Sketch_Line;
    ImageTexture img_Sketch_Arc;
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
    None            = 0,
    Clear           = 1 << 0,
    ActiveDrawing   = 1 << 1,
    ActiveFunction  = 1 << 2,
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

