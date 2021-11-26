#pragma once
 
// a vector wrapper which allows a specific item to be currently selected
template<typename T>
class  VectorSelectable
{
public:
    void Add(const T& v)                { m_Items.push_back(v); m_CurrentIndex = m_Items.size()-1; }
    void Remove(size_t index) { 
        assert(index < m_Items.size()); 
        m_Items.erase(m_Items.begin() + index);
        if(m_CurrentIndex != -1 && (int)index <= m_CurrentIndex) 
            m_CurrentIndex--; 
    }
    void RemoveCurrent()                { assert(m_CurrentIndex > -1 && m_CurrentIndex < (int)m_Items.size()); Remove(m_CurrentIndex); }
    T& CurrentItem()                    { assert(m_CurrentIndex > -1 && m_CurrentIndex < (int)m_Items.size()); return m_Items[m_CurrentIndex]; }
    T& Item(size_t index)               { assert(index < m_Items.size()); return m_Items[index]; }
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
        std::string     serialDevice    = "/dev/ttyS0"; // "/dev/ttyAMA0"
        std::string     serialBaudrate  = "115200";
        std::string     curDir          = File::ThisDir();
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
            
            Tool(std::string name, float diameter = 6.0f, float length = 20.0f) : Name(name), Diameter(diameter), Length(length) {}
            
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
        
        glm::vec3 BackgroundColour = { 0.45f, 0.55f, 0.60f };
        glm::vec3 ToolpathColour = { 0.851f, 0.697f, 0.086f };
        
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
        float TabSpacing = 50.0f;
        float TabHeight = 4.0f;
        float TabWidth = 8.0f;
        // shape offset
        glm::vec3 ShapeColour       = { 0.0f, 1.0f, 0.0f };
        glm::vec3 ShapeOffsetColour = { 1.0f, 0.0f, 0.0f };
        int QuadrantSegments        = 30;
    } pathCutter;
    
    struct CustomGCode {
        std::string name;
        std::string gcode;
    };
    std::vector<CustomGCode> customGCodes;
};

// internal settings (not added to user settings ini file)
struct GUISettings 
{
    
    ImVec2 buttonSize[2]        =  {{ 100.0f, 40.0f },
                                    { 70.0f,  30.0f }};

    ImVec2 buttonImageSize[2]   =  {{ 18.0f,  18.0f },
                                    { 18.0f,  18.0f }};
    
    float dockPadding           =   9.0f;
    float toolbarHeight         =   150.0f;
    float toolbarSpacer         =   10.0f;
    float toolbarComboBoxWidth  =   150.0f;
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
    void AddParameterWithPrefix(const std::string name, const std::string& prefix, uint index, T dataLocation) {    
        AddParameter(PrefixName(prefix, index, name), dataLocation);
    }
    std::string PrefixName(const std::string& prefix, uint index, const std::string& name) {
        std::stringstream stream;
        stream << "<" << prefix << "#" << index << ">"  << name;
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
    
// main class - Contains all settings / dynamic settings
class Settings 
{    
public:
    Settings(const std::string& filename);
    
    void SaveToFile();
    int UpdateFromFile();

    GRBLVals grblVals;
    ParametersList p;
    
private:
// *************************************************************************************************************
// ********************************************* USER SETTINGS *************************************************
    void AddSettings();
    void AddDynamicSettings();

// ****************************************** END OF USER SETTINGS *********************************************
// *************************************************************************************************************
        
private:
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

public:
    GUISettings guiSettings;
};

