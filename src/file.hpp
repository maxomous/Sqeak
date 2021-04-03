/*
 * file.hpp
 */


#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include "dirent.h"	// viewing directory
#include <sys/stat.h> // file stats
// for getting current filepath
#include <limits.h>
#include <unistd.h>

#define FOLDER_TYPE	1
#define FILE_TYPE	2

typedef struct {
    int id;				// ID to allow us to sort a list of filedesc_t's and find the right item
    std::string name;			// filename / foldername
    std::string ext;			// file extension / "" for folder
    tm lastModified;			// last modification
    std::string lastModifiedStr;	// last modified as a readable string
    int type; 				// file or folder (FILE_TYPE or FOLDER_TYPE)
} filedesc_t;
    
    
class File
{
    public:
	// deleted copy constructor
	File(const File&) = delete;
	File& operator=(const File&) = delete;
	
	// returns instance of File / creates if does already exist
	static File& Get() {
	    static File instance;
	    return instance;
	}
	// Returns the directory of this executable from argv
	static std::string GetWorkingDir() { return Get().IGetWorkingDir(); }
	// Returns string of "directory/location"
	static std::string GetWorkingDir(const std::string& location) { return Get().IGetWorkingDir(location); }
	// Combines dir & name to give a file location
	static std::string CombineDirPath(const std::string& dir, const std::string& name) { return Get().ICombineDirPath(dir, name); };
	// Writes to a new file or overwrites existing file
	static void Write(const std::string& filename, const std::string& str) { return Get().IWrite(filename, str); };
	// Writes to a new file or appends to the end of an existing file
	static void Append(const std::string& filename, const std::string& str) { return Get().IAppend(filename, str); };
	// Reads contents of file and calls func callback
	// returns 0 on success / 1 if unsuccessful
	    
//	auto executeLine = [](string& str) {
//	    cout << str << endl;
//	    return 0;
//	}; 
//	if(File::Read("/home/pi/Desktop/New.nc", executeLine)) {
//		cout << "Error: Could not open file" << endl;
//	}
	
	static int Read(const std::string& filename, const std::function<int(std::string&)>& func) { return Get().IRead(filename, func); };
	// retrieves information about the files within a given directory
	// if extensions is not an empty string, it will only return files with given extensions (can be seperated by ',' e.g. "exe,ini")
	// returns 1 on failure
	static int GetFilesInDir(const std::string& location, const std::string& extensions, std::vector<filedesc_t>& files) { return Get().IGetFilesInDir(location, extensions, files); };
    
    private:
	// internal functions
	std::string IGetWorkingDir();
	std::string IGetWorkingDir(const std::string& location);
	std::string ICombineDirPath(const std::string& dir, const std::string& name);
	void IWrite(const std::string& filename, const std::string& str);
	void IAppend(const std::string& filename, const std::string& str);
	int IRead(const std::string& filename, const std::function<int(std::string&)>& func);
	int IGetFilesInDir(const std::string& location, const std::string& extensions, std::vector<filedesc_t>& files);

	// prevent class from being instatiated
	File() {}	
};

#endif /* FILE_HPP */ 
