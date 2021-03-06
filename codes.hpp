/*
 * codes.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#ifndef CODES_HPP
#define CODES_HPP

extern int getErrMsg(int num, std::string* name, std::string* desc);
extern int getSettingsMsg(int num, std::string* name, std::string* units, std::string* desc);
extern int getAlarmMsg(int num, std::string* name, std::string* desc);
#endif
