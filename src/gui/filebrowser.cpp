#include "filebrowser.h"
using namespace std;
using namespace MaxLib;

namespace Sqeak { 


#define SORT_BY_NAME 1
#define SORT_BY_DATE 2

#define MAX_DISPLAY_FOLDERS 5 // max no. folders to display

FileBrowser::FileBrowser(std::string* currentDirectory) 
    : m_CurrentDirectory(currentDirectory), m_Popup("FileBrowserPopup")
{
    // initialise the images
    img_File.Init(File::ThisDir("img/img_file.png").c_str());
    img_Folder.Init(File::ThisDir("img/img_folder.png").c_str());
    // initialise filetype to show in browser
    assert(m_FileTypes.Size() > 0 && "File browser requires at least one filetype");
    m_FileTypes.SetCurrentIndex(0);
    updateDirAndFiles(*m_CurrentDirectory); 
}

 



void FileBrowser::Open()
{
    m_Popup.Open();
}

void FileBrowser::DrawPopup()
{
    // Always center the fileviewer window
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    m_Popup.DrawModal([&]() {
        // close popup on error or file has been selected
        if(DrawWidgets()) {
            ImGui::CloseCurrentPopup();
        }
    }, ImGuiWindowFlags_AlwaysAutoResize);
}

bool FileBrowser::DrawWidgets()
{
    bool needsClosing = false;
    ImGui::TextUnformatted("Select the file you wish to open:n");
    ImGui::Separator();
    DrawFolders();
    needsClosing |= DrawFiles();

    static std::function<std::string(FileType& item)> callback = [](FileType& item) { return item.display; };
    if(ImGuiModules::ComboBox("Type", m_FileTypes, callback)) {
        updateDirAndFiles(*m_CurrentDirectory);
    }
    if (ImGui::Button("OK", ImVec2(120, 0))) {
        needsClosing |= openSelectedFile();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        needsClosing = true;
    }
    return needsClosing;
}

 
void FileBrowser::SetCurrentDirectory(const string& directory) 
{
    // empty array if already full
    m_CurrentDirFolders.clear();
    // set CurrentDirectory string for easy access
    *m_CurrentDirectory = directory;
    // split directory into folders
    istringstream stream(directory);
    string segment;

    for (string folder; getline(stream, folder, '/');) {
        if (folder != "") {
            m_CurrentDirFolders.push_back(folder);
        }
    } 
    nodeSelected = m_CurrentDirFolders.size() - 1;
}

// update list of files and directories from within curDir
void FileBrowser::updateFiles() 
{
    assert(m_FileTypes.HasItemSelected() && "File browser has no item selected");
    if (File::GetFilesInDir(*m_CurrentDirectory, m_FileTypes.CurrentItem().extension, m_Files)) {
        Log::Error("Can't access directory: %s", m_CurrentDirectory->c_str());
        return;
    }
    m_ResortFiles = true;
}
// full update of directory & files
void FileBrowser::updateDirAndFiles(const string &dir) 
{
    SetCurrentDirectory(dir);
    updateFiles();
    selectedFileID = -1;
}

// returns filename and type of file selected in browser
int FileBrowser::getSelectedFile(string &filename, int &filetype) 
{
    if (selectedFileID == -1)
        return -1;

    int i = 0;
    for (File::FileDesc f : m_Files) {
        if (f.id == selectedFileID) {
            filetype = f.type;
            filename = f.name;
            // add on extension
            if (filetype == File::FileDesc::Type::File && f.ext != "")
                filename += "." + f.ext;
            return 0;
        }
        i++;
    }
    // didn't find it - shouldn't ever reach
    Log::Error(
        "File not found in list - We should never have reached this...");
    filetype = -1;
    return -1;
}

bool FileBrowser::openSelectedFile() 
{
    bool needsClosing = false;
    int filetype; 
    string filename;

    if (getSelectedFile(filename, filetype))
        return true; // error, couldn't find it... shouldnt
                                    // reach

    if (filetype == File::FileDesc::Type::File) {
        m_CurrentFile = filename;
        m_Filepath = File::CombineDirPath(*m_CurrentDirectory, m_CurrentFile);
        Log::Info(string("Opening file: ") + m_Filepath);
        
        Event_Update3DModelFromFile data = { m_Filepath };
        Event<Event_Update3DModelFromFile>::Dispatch(data); 
        
        needsClosing = true;
    } else { // folder
        updateDirAndFiles(File::CombineDirPath(*m_CurrentDirectory, filename));
    }
    return needsClosing;
}

void FileBrowser::sortFiles(const ImGuiTableColumnSortSpecs *sort_spec) {
    auto compare = [&](const File::FileDesc &a, const File::FileDesc &b) {
        string strA, strB;

        switch (sort_spec->ColumnIndex) { // 0 is file/folder icon
        case 1:                           // name
            strA = a.name;
            strB = b.name;
            MaxLib::String::LowerCase(strA);
            MaxLib::String::LowerCase(strB);
            if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                return (strA < strB);
            else
                return (strA > strB);
            break;

        case 2: // Type
            strA = a.ext;
            strB = b.ext;
            MaxLib::String::LowerCase(strA);
            MaxLib::String::LowerCase(strB);
            if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                return (strA < strB);
            else
                return (strA > strB);
            break;

        case 3: // Date Modifed
            char dateStr_a[32], dateStr_b[32];
            strftime(dateStr_a, 32, "%y%j%H%M%S", &(a.lastModified));
            strftime(dateStr_b, 32, "%y%j%H%M%S", &(b.lastModified));
            int64_t dateVal_a, dateVal_b;
            dateVal_a = strtoll(dateStr_a, NULL, 10);
            dateVal_b = strtoll(dateStr_b, NULL, 10);

            if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                return (dateVal_a < dateVal_b);
            else
                return (dateVal_a > dateVal_b);
            break;

        default:
            Log::Error("Sorting column unknown");
            return false;
        }
        return false;
    };

    auto compareType = [&](const File::FileDesc &a, const File::FileDesc &b) {
        return a.type < b.type;
    };

    sort(m_Files.begin(), m_Files.end(), compare);
    sort(m_Files.begin(), m_Files.end(), compareType);
}

void FileBrowser::DrawFolder(int i) {

    ImGuiTreeNodeFlags node_flags =
        ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_FramePadding; // ImGuiTreeNodeFlags_OpenOnArrow |
                                         // ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (i == nodeSelected)
        node_flags |= ImGuiTreeNodeFlags_Selected;

    if (ImGui::TreeNodeEx(m_CurrentDirFolders[i].c_str(), node_flags)) {
        if (ImGui::IsItemClicked())
            nodeClicked = i;
        i++;
        if (i < (int)m_CurrentDirFolders.size())
            DrawFolder(i);

        ImGui::TreePop();
    }
}
void FileBrowser::DrawFolders() 
{
    int s = m_CurrentDirFolders.size();
    // recursively draw all the folders in m_CurrentDirFolders (which are the
    // individual folders of curDir)
    int startAt = (s > MAX_DISPLAY_FOLDERS) ? s - MAX_DISPLAY_FOLDERS : 0;
    DrawFolder(startAt);
    // draw blanks if less than number displayable
    if (s < MAX_DISPLAY_FOLDERS) {
        for (int i = 0; i < MAX_DISPLAY_FOLDERS - s; i++)
            ImGui::Dummy(ImVec2(0, ImGui::GetFrameHeight()));
    }
    // if a folder is selected, mark it, and update curDir, m_CurrentDirFolders & m_CurrentFiles
    if (nodeClicked != -1) {
        nodeSelected = nodeClicked;
        nodeClicked = -1;

        string newDir = "/";
        for (int j = 0; j < nodeSelected + 1; j++) {
            if (j < (int)m_CurrentDirFolders
                        .size()) { // just in case to prevent overflow
                newDir += m_CurrentDirFolders[j];
                newDir += '/';
            }
        }
        updateDirAndFiles(newDir);
    }
}
bool FileBrowser::DrawFiles() 
{
    bool err = false;
    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable( "table_sorting", 4, flags, ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing() * 15), 0.0f)) 
    {
        ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoResize, 0.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort, 0.0f);
        ImGui::TableSetupColumn("Extension", ImGuiTableColumnFlags_None, 0.0f);
        ImGui::TableSetupColumn("Date Modified", ImGuiTableColumnFlags_None, 0.0f);
        // Make top row always visible
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Resort data if sort specs has been changed
        ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs();
        if (sorts_specs->SpecsDirty | m_ResortFiles) {
            if (m_Files.size() > 1) {
                sortFiles(&sorts_specs->Specs[0]);
            }
            sorts_specs->SpecsDirty = false;
            m_ResortFiles = false;
        }

        ImGuiListClipper clipper;
        clipper.Begin(m_Files.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart;
                 row_n < clipper.DisplayEnd; row_n++) {
                // Display a data item
                File::FileDesc *item = &m_Files[row_n];
                const bool item_is_selected = (item->id == selectedFileID);

                ImGui::PushID(item->id);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (item->type == File::FileDesc::Type::Folder)
                    ImGui::Image(img_Folder, ImVec2(16, 16));
                else
                    ImGui::Image(img_File, ImVec2(16, 16));

                ImGui::TableNextColumn();
                if (ImGui::Selectable(
                        item->name.c_str(), item_is_selected,
                        ImGuiSelectableFlags_SpanAllColumns |
                            ImGuiSelectableFlags_AllowItemOverlap |
                            ImGuiSelectableFlags_AllowDoubleClick,
                        ImVec2(0, 0))) 
                {
                    selectedFileID = item->id;

                    if (ImGui::IsMouseDoubleClicked(0))
                        openSelectedFile();
                    else { // single click
                        int filetype;
                        string filename;

                        if (getSelectedFile(filename, filetype))
                            err = true; // error, couldn't find it... shouldnt reach

                        if (filetype == File::FileDesc::Type::Folder)
                            updateDirAndFiles(File::CombineDirPath(*m_CurrentDirectory, filename));
                    }
                }
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(item->ext.c_str());
                ImGui::TableNextColumn();

                ImGui::TextUnformatted(item->lastModifiedAsText.c_str());
                ImGui::PopID();
            }
        ImGui::EndTable();
    }
    return err;
}


} // end namespace Sqeak
