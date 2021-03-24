/*
 * file.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */
#ifndef __FILE_HPP
#define __FILE_HPP

extern std::string getWorkingDir(char **argv);
extern int readFile(GRBL* Grbl, const std::string& filename);
#endif 
