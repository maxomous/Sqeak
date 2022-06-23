/*
 * grblcodes.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */
#pragma once

namespace Sqeak { 
    
extern int getErrMsg(int num, std::string* name, std::string* desc);
extern int getSettingsMsg(int num, std::string* name, std::string* units, std::string* desc);
extern int getAlarmMsg(int num, std::string* name, std::string* desc);

} // end namespace Sqeak
