#include "common.h" 
using namespace std;
using namespace MaxLib;

namespace Sqeak { 
// *************************************************************************************************************
// ********************************************* USER SETTINGS *************************************************

// used to replace '\n' with for multi line settings strings
#define NEWLINE_STRING     std::string("{NEWLINE}")
    
void Settings::AddSettings()
{       
    // Static Settings   
    Setting s = Setting("System", 0);
    s.AddParameter("SerialDevice", &p.system.serialDevice);
    s.AddParameter("SerialBaudrate", &p.system.serialBaudrate); 
    s.AddParameter("CurrentDirectory", &p.system.curDir); 
    s.AddParameter("SaveFileDirectory", &p.system.saveFileDirectory);
    m_SettingsList.push_back(s); 
                
    s = Setting("Viewer", 0); 
    s.AddParameter("ToolpathFeedColour", &p.viewer.toolpath.Colour_Feed);
    s.AddParameter("ToolpathFeedZColour", &p.viewer.toolpath.Colour_FeedZ);
    s.AddParameter("ToolpathRapidColour", &p.viewer.toolpath.Colour_Rapid);
    s.AddParameter("ToolpathHomeColour", &p.viewer.toolpath.Colour_Home);
    s.AddParameter("BackgroundColour", &p.viewer.general.BackgroundColour);
    m_SettingsList.push_back(s);
    
    s = Setting("Viewer-Elements", 0);
    s.AddParameter("PointSize",          &p.sketch.point.size);
    s.AddParameter("PointColour",        &p.sketch.point.colour);
    s.AddParameter("PointColourActive",        &p.sketch.point.colourActive);
    s.AddParameter("LineColour",         &p.sketch.line.colour);
    s.AddParameter("LineColourDisabled", &p.sketch.line.colourDisabled);
    m_SettingsList.push_back(s);

    s = Setting("Viewer-Axis", 0);
    s.AddParameter("Size", &p.viewer.axis.Size);
    m_SettingsList.push_back(s);
     
    s = Setting("Viewer-Grid", 0); 
    s.AddParameter("Position", &p.viewer.grid.Position);
    s.AddParameter("Size", &p.viewer.grid.Size);
    s.AddParameter("Spacing", &p.viewer.grid.Spacing);
    s.AddParameter("Colour", &p.viewer.grid.Colour);
    m_SettingsList.push_back(s);
      
    s = Setting("Viewer-Spindle", 0);
    s.AddParameter("ToolColour", &p.viewer.spindle.colours.tool);
    s.AddParameter("ToolHolderColour", &p.viewer.spindle.colours.toolHolder);
    s.AddParameter("ToolColourOutline", &p.viewer.spindle.colours.toolOutline);
    m_SettingsList.push_back(s);   
       
    s = Setting("PathCutter", 0);   
    s.AddParameter("CutTabs", &p.pathCutter.CutTabs);
    s.AddParameter("TabSpacing", &p.pathCutter.TabSpacing);  
    s.AddParameter("TabHeight", &p.pathCutter.TabHeight);
    s.AddParameter("TabWidth", &p.pathCutter.TabWidth);
    s.AddParameter("CutOverlap", &p.pathCutter.CutOverlap);
    s.AddParameter("PartialRetractDistance", &p.pathCutter.PartialRetractDistance);
    s.AddParameter("QuadrantSegments", &p.pathCutter.geosParameters.QuadrantSegments);
    m_SettingsList.push_back(s);
} 

void Settings::AddDynamicSettings()
{      
/*   
    char*           str = (*(const std::vector<Tool>*)data)[idx].Name.c_str(); 
    string           s = (*(const std::vector<Tool>*)data)[idx].Name;
    Tool             t = (*(const std::vector<Tool>*)data)[idx]
    vector<Tool>     v = (*(const std::vector<Tool>*)data) 
    *vector<Tool>     vptr = (const std::vector<Tool>*)data
 */  
    DynamicSetting d = DynamicSetting("CustomGCode", (void*)&p.customGCodes, 
        // get size   
        [](void* data) { 
            return (*(const std::vector<ParametersList::CustomGCode>*)data).size();
        },      
        // add parameters
        [](Setting& setting, void* data, uint id) { 
            ParametersList::CustomGCode& d = (*(std::vector<ParametersList::CustomGCode>*)data)[id];
            setting.AddParameter(std::string("Name"),   &(d.name));
            setting.AddParameter(std::string("GCode"),  &(d.gcode));
        },
        // check if vector is large enough 
        [&](void* data, std::string& name, uint id, std::string& paramName) { 
            (void)name; (void)paramName;
            std::vector<ParametersList::CustomGCode>& v = (*(std::vector<ParametersList::CustomGCode>*)data); 
            while(v.size() < id+1) {
                v.push_back(ParametersList::CustomGCode({"",""}));
            }
        });
    m_VectorList.push_back(d);          
    /*
     *  TODO: Make this accept Vector_Ptrs
     * 
    static const std::string ToolListStr = "ToolList";
    d = DynamicSetting(ToolListStr, (void*)p.tools.toolList.DataPtr(), 
        // get size
        [](void* data) { 
            return (*(const std::vector<ParametersList::Tools::Tool>*)data).size();
        }, 
        // add parameters
        [](Setting& setting, void* data, uint id) { 
            ParametersList::Tools::Tool& d = (*(std::vector<ParametersList::Tools::Tool>*)data)[id];
            setting.AddParameter("Name",        &d.Name);
            setting.AddParameter("Diameter",    &d.Diameter);
            setting.AddParameter("Length",      &d.Length);
                    
            for (uint i = 0; i < d.Data.Size(); i++) {
                ParametersList::Tools::Tool::ToolData* toolData = d.Data.ItemPtr(i);
                setting.AddParameterWithPrefix("Material",        i, &(toolData->material)); // <1>Material
                setting.AddParameterWithPrefix("SpindleSpeed",    i, &(toolData->speed));
                setting.AddParameterWithPrefix("CuttingFeedRate", i, &(toolData->feedCutting));
                setting.AddParameterWithPrefix("PlungeFeedRate",  i, &(toolData->feedPlunge));
                setting.AddParameterWithPrefix("CutDepth",        i, &(toolData->cutDepth));
            }
        },  
        // check if vector is large enough 
        [&](void* data, std::string& name, uint id, std::string& paramName) { 
            (void)name; (void)id;
            std::vector<ParametersList::Tools::Tool>& v = (*(std::vector<ParametersList::Tools::Tool>*)data); 
            
            // vector parameter: <0>SpindleSpeed=10000.000000
            if(paramName.substr(0,1) == std::string("<"))
            {
                size_t b = paramName.find(">");
                // error if b doesn't exist or there is no number between < & >
                if(b == std::string::npos || b < 2) {
                    Log::Error("%s in Config file looks incorrect", paramName.c_str());
                    return; 
                }
                int prefixId                = stoi(paramName.substr(1, b-1));                
                while(v[id].Data.Size() < (uint)prefixId+1) {
                    v[id].Data.Add(ParametersList::Tools::Tool::ToolData());
                } 
            } else { // main parameter Length=20.000000 
                while(v.size() < id+1) {
                    v.push_back(ParametersList::Tools::Tool()); 
                }
            } 
        });
    m_VectorList.push_back(d); 
    */
}    
   
// ****************************************** END OF USER SETTINGS *********************************************
// *************************************************************************************************************

bool ParametersList::Tools::IsToolAndMaterialSelected() 
{        
    if(!toolList.HasItemSelected()) {
        Log::Error("No Tool Selected");
        return true;
    }
    if(!toolList.CurrentItem().Data.HasItemSelected()) {
        Log::Error("No Material Selected");
        return true; 
    }
    return false;
}  
glm::vec3 ParametersList::Tools::GetToolScale()
{
    if(toolList.HasItemSelected()) {
        ParametersList::Tools::Tool& tool = toolList.CurrentItem();
        return glm::vec3(tool.Diameter, tool.Diameter, tool.Length);
    } // else get default tool dimensions
    ParametersList::Tools::Tool defaultTool("default tool");
    return glm::vec3(defaultTool.Diameter, defaultTool.Diameter, defaultTool.Length);
}   

std::string Setting::GetParamName(size_t i) { 
    return std::string(data[i].first); 
}
auto Setting::GetDataLocation(size_t i) { 
    return data[i].second; 
}
    
DynamicSetting::DynamicSetting(const std::string& name, void* data,  
    std::function<size_t(void* data)> cb_GetSize,
    std::function<void(Setting& setting, void* data, uint id)> cb_AddParameters, 
    std::function<void(void* data, std::string& name, uint id, std::string& paramName)>  cb_UpdateVectorSize
) 
    : m_Name(name), m_Data(data), m_cb_GetSize(cb_GetSize), m_cb_AddParameters(cb_AddParameters), m_cb_UpdateVectorSize(cb_UpdateVectorSize) 
{ }

 
size_t DynamicSetting::GetSize() {  
    return m_cb_GetSize(m_Data); 
}
void DynamicSetting::AddParameters(Setting& setting, size_t index) { 
    m_cb_AddParameters(setting, m_Data, index); 
}
void DynamicSetting::UpdateVectorSize(std::string name, size_t index, std::string& paramName) { 
    m_cb_UpdateVectorSize(m_Data, name, index, paramName); 
}
std::string DynamicSetting::Name() {  
    return m_Name; 

}   

Settings::Settings(const std::string& filename) : m_Filename(filename) {
    
    Event<Event_SaveSettings>::RegisterHandler([&](Event_SaveSettings data) {
        (void)data;
        SaveToFile();
    });
    Event<Event_UpdateSettingsFromFile>::RegisterHandler([&](Event_UpdateSettingsFromFile data) {
        (void)data; 
        UpdateFromFile();
    }); 
      
    AddSettings();  
    AddDynamicSettings();
    UpdateFromFile(); 
} 

void Settings::SaveToFile() 
{  
    UpdateDynamicSettings();
    std::ostringstream stream;
    // go through each setting
    for(auto setting : m_SettingsList) {
        stream << "[" << setting.name << "][" << setting.id << "]" << std::endl;
        // go through each parameter
        for (size_t i = 0; i < setting.data.size(); i++)
        {
            std::string valueStr = GetSettingString(setting, i);
            stream << setting.GetParamName(i) << "=" << valueStr << std::endl;
        }
        stream << std::endl;
    }
    std::string filename = m_Filename;
    File::Write(filename, stream.str()); 
} 

 
// builds vector of class Settings from config code ("[name][id]" ...)
int Settings::UpdateFromFile()
{
    std::string name;
    int id = -1;
    std::vector<Setting> settingsData;
    
    auto executeLine = [&](std::string &str) {
        if (str == "")
            return 0;
        // header       [Viewer-Grid][0]
        if(!str.compare(0, 1, "[")) {
            
            size_t a = str.find("]", 1);
            // checks to prevent errors
            if((a == std::string::npos) || (a <= 0) || str.length() <= a) {
                Log::Error("Problem finding ']' in %s config file", str.c_str());
                return -1;
            }
            std::string name_str = str.substr(1, a-1);
            std::string id_str = str.substr(a+2, str.length() - a - 3);
            if(id_str == "") {
                Log::Error("Problem finding ID in config file");
                return -1;
            }            
            name = name_str;
            id = std::stoi(id_str);
            //std::cout << name << "(name):" << id << "(id)" << std::endl;
            
        } // data       Position=0,0,0
          //            Size=1200,600
        else {
            if(name == "" || id <= -1 ) {
                Log::Error("Name or id not set in config file");
                return -1;
            }     
            size_t a = str.find("=", 1);
            // checks to prevent errors
            if((a == std::string::npos) || (a <= 0) || str.length() <= a-1) {
                Log::Error("Problem finding ']' in %s config file", str.c_str());
                return -1;
            } 
            std::string paramName = str.substr(0, a);
            std::string dataString = str.substr(a+1);
              
            SetParameter(name, (uint)id, paramName, dataString);
        }
        return 0;
    };

    std::string filename = m_Filename;
    if (File::Read(filename, executeLine)) {
        Log::Error("Can't update data, no config file found");
        return -1;
    }
    return 0;
}

int Settings::GetVectorIndex(std::string name)
{
    for (size_t i = 0; i < m_VectorList.size(); i++) {
        if(name == m_VectorList[i].Name()) {
            return i;
        }
    }
    return -1;
}

void Settings::CheckVectorIsBigEnough(std::string& name, uint id, std::string& paramName)
{
    // First, check if it's a vector type, then ensure the vector is big enough and update m_SettingsList 
    int vectorIndex = GetVectorIndex(name);
    if(vectorIndex >= 0) { 
        DynamicSetting& dSetting = m_VectorList[vectorIndex]; 
        dSetting.UpdateVectorSize(name, id, paramName);
        UpdateDynamicSetting(dSetting);
    } 
} 
int Settings::SetParameter(std::string& name, uint id, std::string& paramName, std::string& dataString)
{
    CheckVectorIsBigEnough(name, id, paramName);
    
    // go through our list of settings and find the one with the same name/id/parameter
    for(Setting& setting : m_SettingsList) 
    {
        //std::cout << "name: " << setting.name << " id: " << setting.id << std::endl;
        if(setting.name == name && setting.id == id) 
        {
            for(size_t i = 0; i < setting.data.size(); i++)
            {
                if(setting.GetParamName(i) == paramName) {
                    // we've found the correct parameter inside m_SettingsList
                    // now determine the type of data and copy the values across
                    SetSettingFromString(setting, i, dataString);
                    return 0;
                } 
            }
            Log::Error("Setting in file found, but not the parameter. Name = %s, id = %u, ParamName = %ss", name.c_str(), id, paramName.c_str());
            return -1;
        }
    }
    Log::Error("Setting in file not found on system. Name = %s, id = %u, ParamName = %s", name.c_str(), id, paramName.c_str());
    return -1;
}

    
void Settings::UpdateDynamicSettings()
{
    for(DynamicSetting& setting : m_VectorList) {
        UpdateDynamicSetting(setting);
    }
}
// removes old settings from and updates m_SettingsList
void Settings::UpdateDynamicSetting(DynamicSetting& dSetting)
{
    // remove old settings with same name
    m_SettingsList.erase(std::remove_if(m_SettingsList.begin(), m_SettingsList.end(), [&](Setting& setting) { 
        return setting.name == dSetting.Name();
    }), m_SettingsList.end());
    // add existing settings to list
    for (size_t i = 0; i < dSetting.GetSize(); i++) {
        Setting item(dSetting.Name(), i);
        dSetting.AddParameters(item, i);
        m_SettingsList.push_back(item);
    }
}

// takes a variable and converts its value to a string
std::string Settings::GetSettingString(Setting& setting, size_t paramIndex)
{
    auto refToData = setting.GetDataLocation(paramIndex);
    if(auto dataLocation = std::get_if<bool*>(&refToData)) {
        bool* v = *dataLocation;
        return std::to_string(*v);
    } 
    else if(auto dataLocation = std::get_if<char*>(&refToData)) {
        char* v = *dataLocation;
        return std::to_string(*v);
    } 
    else if(auto dataLocation = std::get_if<int*>(&refToData)) {
        int* v = *dataLocation;
        return std::to_string(*v);
    } 
    else if(auto dataLocation = std::get_if<float*>(&refToData)) {
        float* v = *dataLocation;
        return std::to_string(*v);
    } 
    else if(auto dataLocation = std::get_if<double*>(&refToData)) {
        double* v = *dataLocation;
        return std::to_string(*v); 
    } 
    else if(auto dataLocation = std::get_if<std::string*>(&refToData)) {
        std::string str = *(*dataLocation);
        // special case for strings so we can accept multiple lines
        while (1) {
            size_t a = str.find("\n");
            if(a == std::string::npos) break;
            str.erase(a, 1);
            str.insert(a, NEWLINE_STRING);
        }
        return str;
    } 
    else if(auto dataLocation = std::get_if<glm::vec2*>(&refToData)) {
        glm::vec2* p = *dataLocation;
        std::ostringstream stream;
        stream << p->x << "," << p->y;
        return stream.str();
    } 
    else if(auto dataLocation = std::get_if<glm::vec3*>(&refToData)) {
        glm::vec3* p = *dataLocation;
        std::ostringstream stream;
        stream << p->x << "," << p->y << "," << p->z;
        return stream.str();
    } 
    
    Log::Critical("Unknown type in settings file");
    // never reaches 
    return "";
}
    
void Settings::SetSettingFromString(Setting& setting, size_t paramIndex, std::string& dataString)
{
    auto refToData = setting.GetDataLocation(paramIndex);
    
    if(auto dataLocation = std::get_if<bool*>(&refToData)) {
        *(*dataLocation) = (dataString == "1");
    }  
    else if(auto dataLocation = std::get_if<char*>(&refToData)) {
        *(*dataLocation) = dataString[0];
    } 
    else if(auto dataLocation = std::get_if<int*>(&refToData)) {
        *(*dataLocation) = std::stoi(dataString);
    } 
    else if(auto dataLocation = std::get_if<float*>(&refToData)) {
        *(*dataLocation) = std::stof(dataString);
    } 
    else if(auto dataLocation = std::get_if<double*>(&refToData)) {
        *(*dataLocation) = std::stod(dataString);
    } 
    else if(auto dataLocation = std::get_if<std::string*>(&refToData)) {
        // special case for strings so we can accept multiple lines
        std::string str = std::string(dataString);
        while (1) {
            size_t a = str.find(NEWLINE_STRING);
            if(a == std::string::npos) break;
            str.erase(a, NEWLINE_STRING.length());
            str.insert(a, 1, '\n');
        }
        *(*dataLocation) = str;
    } 
    else if(auto dataLocation = std::get_if<glm::vec2*>(&refToData)) {
        glm::vec2 p = stoVec2(dataString);
        *(*dataLocation) = p;
    } 
    else if(auto dataLocation = std::get_if<glm::vec3*>(&refToData)) {
        glm::vec3 p = stoVec3(dataString);
        *(*dataLocation) = p;
    } 
    else {
        Log::Error("Unknown type in settings file Name = ", setting.name);
    }
}
    
    
     
ImVec2 GUISettings::FramePosition_UnderToolbar()  
{ 
    return { dockPadding, toolbarHeight + dockPadding * 2.0f }; 
}


} // end namespace Sqeak
