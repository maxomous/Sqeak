#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <MaxLib.h>
// OpenGL / ImGui
#include "../glcore/glcore.h"

//#include "../common.h"
namespace Sqeak { 
    
using namespace MaxLib;
using namespace MaxLib::Vector;

class FileBrowser 
{
public:
    struct FileType { 
        std::string extension; 
        std::string display; 
    };
    // current working directory is sent as parameter so that the location can be remembered each run of program.
    FileBrowser(std::string* currentDirectory);
    // opens the filebrowser (should only be called once)
    void Open();
    // draws the filebroswer popup + widgets
    void DrawPopup();
    // draws just the filebroswer widgets
    bool DrawWidgets();
    // clear tje current file
    void ClearCurrentFile()         { m_CurrentFile = ""; m_Filepath = ""; }
    // returns current file e.g. file.nc
    std::string CurrentFile()       { return m_CurrentFile; }
    // returns current filepath e.g. directory/file.nc
    std::string CurrentFilePath()   { return m_Filepath; }
    
private:
    Vector_SelectablePtrs<FileType> m_FileTypes = { {"nc", "*.nc"}, {"txt", "*.txt"}, {"", "All Files"} };
    
    ImageTexture img_File;
    ImageTexture img_Folder;
    
    std::string* m_CurrentDirectory;
    std::string m_CurrentFile;
    std::string m_Filepath;
    
    std::vector<std::string> m_CurrentDirFolders; // list of all folders inside curDir
    std::vector<File::FileDesc> m_Files;
    
    int selectedFileID = -1;
    int sortByName = true; // else sort by date
    int nodeSelected = -1;
    int nodeClicked = -1;
    bool m_ResortFiles = false; // used when manually requiring resort

    ImGuiModules::ImGuiPopup m_Popup;

    void SetCurrentDirectory(const std::string& dir);

    // update list of files and directories from within curDir
    void updateFiles();
    // full update of directory & files
    void updateDirAndFiles(const std::string& dir);

    // returns filename and type of file selected in browser
    int getSelectedFile(std::string& filename, int& filetype);

    bool openSelectedFile();

    void sortFiles(const ImGuiTableColumnSortSpecs *sort_spec);

    void DrawFolder(int i);
    void DrawFolders();
    bool DrawFiles();
};


} // end namespace Sqeak
