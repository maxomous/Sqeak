/*
 * file.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */
#ifndef __FILE_HPP
#define __FILE_HPP

#define FOLDER_TYPE	1
#define FILE_TYPE	2

typedef struct {
	int id;	// to allow us to sort and find the right item
	std::string name;
	std::string ext;
	tm lastModified;
	std::string lastModifiedStr;
	int type; // 1 - FILE_TYPE / 2 - FOLDER_TYPE
} filedesc_t;
    
extern std::string 	getWorkingDir(char **argv);
extern int readFile(const std::string& filename, const std::function<int(std::string&)>& func);
// combines dir & name to give a file location
extern std::string getFilePath(const std::string& dir, const std::string& name);
extern int getFilesInDir(const std::string& location, const std::string& extensions, std::vector<filedesc_t>& files);
#endif 
