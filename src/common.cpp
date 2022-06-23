
#include "common.h"

using namespace std; 
 
namespace Sqeak { 
    
Time::Time(uint seconds) 
{ 
    m_hr = seconds / 3600;
    seconds %= 3600;
    m_min = seconds / 60;
    seconds %= 60;
    m_sec = seconds; 
}
    

bool trigger(bool& input)
{
    if(!input) {        
        return false;
    }
    input = false;
    return true;
}
 // This takes a std::string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
glm::vec2 stoVec2(const std::string& msg) 
{
    std::stringstream stream(msg);
    std::string segment;
    glm::vec2 p;
    
    for (int i = 0; i < 2; i++) {
        getline(stream, segment, ',');
        p[i] = stof(segment);
    }
    return p;
}
 
// This takes a std::string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
glm::vec3 stoVec3(const std::string& msg) 
{
    std::stringstream stream(msg);
    std::string segment;
    glm::vec3 p;

    for (int i = 0; i < 3; i++) {
        getline(stream, segment, ',');
        p[i] = stof(segment);
    }
    return p;
}
  

} // end namespace Sqeak
