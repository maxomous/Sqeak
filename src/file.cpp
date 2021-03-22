/*
 * file.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */
#include "common.hpp"
#include <fstream>
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

int runFile(GCList* gcList, const string& filename) {
    ifstream openFile(filename);
    if(!openFile) {
		cout << "Error: Couldn't open file" << endl;
        return ERR_FAIL;
    }
    string output;
    while(getline(openFile, output)) {
        gcList->add(output);
    }
    openFile.close();
    return ERR_NONE;
}

