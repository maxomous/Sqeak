/*
 * file.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */
#include "common.hpp"
using namespace std;


string getWorkingDir(char **argv) {
    string loc(argv[0]);
    //remove the filename from the string
    size_t c = 0;
    c = loc.find_last_of("/");
    if(c == string::npos) {
	    cout << "ERROR: Cannot get current working directory" << endl;
	    return "/";
    }
    else {
	loc.erase(++c, loc.length()-c);
	return loc;
    }
}

/*	Usage:
    auto lambda = [](string& str) {
	cout << str << endl;
    }; 
    if(readFile("/home/pi/Desktop/New.nc", lambda)) {
	cout << "Error: Could not open file" << endl;
    }
*/

// reads a line of a file and invokes the callback function with a pointer to the string 
// returns 0 on success / 1 if unsuccessful
int readFile(const std::string& filename, const std::function<int(std::string&)>& func) {
    ifstream openFile(filename);
    if(!openFile) {
	    cout << "Error: Couldn't open file" << endl;
        return -1;
    }
    string output;
    while(getline(openFile, output)) {
	if(func(output)) {
	    cout << "Error: Cannot execute line of file" << endl;
	    return -1;
	}
    }
    openFile.close();
    return 0;
}


// combines dir & name to give a file location
string getFilePath(const string& dir, const string& name) {
    
    string str = dir;
    if(str.back() != '/')
	str += '/';
    str += name;
    return str;
}

// take a directory location and returns the files and directories within it in the vector files
// if extensions is not an empty string, it will only return files with given extensions (can be seperated by ',' e.g. "exe,ini")
// returns 1 on failure
int getFilesInDir(const string& location, const string& extensions, vector<filedesc_t>& files)
{
    auto buildExtensionsList = [](const string& ext) 
    {
	istringstream stream(ext);
	vector<string> list;
	// build vector of extensions
	for(string s; getline(stream, s, ','); ) {
	    // strip out whitespace & add to vector
	    s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
	    list.push_back(s);
	}
	return list;
    };
    
    auto isValidExtension = [](const string& ext, vector<string> list)
    {	// returns true if ext is part of list
	for(string s : list) {
	    if (ext == s)
		return true;
	}
	return false;
    };
    
    files.clear();
    vector<string> permittedExt = buildExtensionsList(extensions);
    
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir (location.c_str())) != NULL) {
	while ((ent = readdir (dir)) != NULL) {
	    // ignore . hidden files & ..
	    if(strncmp(ent->d_name, ".", 1) && strncmp(ent->d_name, "..", 2)) 
	    {
		static int id = 0;
		string str = ent->d_name;
		string name, ext;
		int type = 0;
		// if directory
		if(ent->d_type == DT_DIR) {
		    name = str;
		    ext = "";
		    type = FOLDER_TYPE;
		} else {
		    // split name and file extension
		    size_t dotPos = str.find_last_of(".");
		    name = (dotPos != string::npos) ? str.substr(0, dotPos) : str;
		    ext = (dotPos != string::npos) ? str.substr(dotPos+1) : "";
		    type = FILE_TYPE;
		}
		if(type == FOLDER_TYPE || extensions == "" || isValidExtension(ext, permittedExt))
		{
		    struct stat attrib;
		    tm modtimeLocal;
		    char dateStr[32];
		    string filepath = getFilePath(location, string(ent->d_name));
		    // get modified date & time
		    if(!stat(filepath.c_str(), &attrib)) {
			time_t modtime = attrib.st_mtime;
			modtimeLocal = *localtime(&modtime);
			strftime(dateStr, 32, "%d-%b-%y  %H:%M", localtime(&modtime));
		    }
		    files.push_back((filedesc_t){.id = id++, .name = name, .ext = ext, .lastModified = modtimeLocal, .lastModifiedStr = dateStr, .type = type});
		}
	    }
	}
	closedir (dir);
    } else {
	/* could not open directory */
	perror ("");
	return 1;
    }
    return 0;
}

