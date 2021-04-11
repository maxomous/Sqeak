/*
 * frames.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "../common.h"
using namespace std;

// unneeded with SetNextWindowSize(ImVec2(0, 200), ImGuiCond_Always)
//static ImGuiWindowFlags windowFlags = 0;//ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

/*
 * io.configwindowsmovefromtitlebaronly
 * noclose
 * noresize? / noscrollbar
 * windowtitlealign 0.5
    frameRounding 3
    scrollbarRounding 
    GrabRounding 

 aligning text  
ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 1.0f));

ImGui::PopStyleVar();
  */
  
#define MAX_CUSTOM_GCODES 12 // should be divisible by 3
#define MAX_HISTORY 100 

//*******************************************************//
//*******************CUSTOM WIDGETS**********************//

// Disable all widgets when not connected to GRBL
void BeginDisableWidgets(GRBL* Grbl)
{
    if(!Grbl->IsConnected()) {
	// disable all widgets
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
}
void EndDisableWidgets(GRBL* Grbl)
{
    if(!Grbl->IsConnected()) {
	ImGui::PopStyleVar();
	ImGui::PopItemFlag();
    }
}
  

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}



template<typename T>
int Incrementer(const char* id, const char* str, T increment, T& modifyValue, bool allowNegative = true) 
{
    int buttonPress = 0;
    ImGui::PushID(id);
    if(ImGui::SmallButton("-")) {
	modifyValue = (allowNegative || modifyValue - increment > 0) ? modifyValue - increment : 0;
	buttonPress = -1;
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(str);
    ImGui::SameLine();
    if(ImGui::SmallButton("+")) {
	modifyValue += increment;
	buttonPress = 1;
    }
    ImGui::PopID();
    return buttonPress;
};

struct Console
{	
    bool log_Tab_Open = true;
    bool scroll_To_Bottom = false;
    bool reclaim_focus;
    vector<string> history;
    int historyCount = -1;
    int historyPos = 0;
    
    void DrawLogTab(GRBL* Grbl) 
    {
      if (ImGui::BeginTabItem("Log"))
	{	
	    log_Tab_Open = true;
	    // Console    
	    ImGui::BeginChild("Log", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 3 - 16), false, ImGuiWindowFlags_HorizontalScrollbar);
	    // Clip only visible lines
	    ImGuiListClipper clipper;
	    clipper.Begin(Grbl->consoleLog->size());
	    while (clipper.Step()) {
		for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
		    ImGui::TextUnformatted((*(Grbl->consoleLog))[line_no].c_str());
		}
	    }
	    clipper.End();
	    
	    if (scroll_To_Bottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		ImGui::SetScrollHereY(1.0f); // set to bottom of last item
    
	    ImGui::EndChild();
	    ImGui::EndTabItem();
	}
    }
    
    void DrawCommandsTab(GRBL* Grbl) 
    {
	if (ImGui::BeginTabItem("Commands"))
	{	
	    log_Tab_Open = false;
	    // Commands    
	    ImGui::BeginChild("Commands", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 3 - 16));
    
	    if(ImGui::BeginTable("Commands", 3, ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV, ImVec2(ImGui::GetWindowWidth(), 0)))
	    {	// Make top row always visible
		ImGui::TableSetupScrollFreeze(0, 1); 
		// Set up headers
		ImGui::TableSetupColumn("Sent", ImGuiTableColumnFlags_None, 165.0f);
		ImGui::TableSetupColumn("Reponse", ImGuiTableColumnFlags_None, 55.0f);
		ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_None, 600.0f);
		ImGui::TableHeadersRow();
		
		ImGuiListClipper clipper;
		clipper.Begin(Grbl->gcList.GetSize());
		while (clipper.Step()) 
		{
		    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
		    {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			
			GCItem_t gcItem = Grbl->gcList.GetItem(row_n);
			ImGui::TextUnformatted(gcItem.str.c_str());
			ImGui::TableNextColumn();
			switch (gcItem.status)
			{
			    case STATUS_UNSENT:
				ImGui::TextUnformatted("");
				break;
			    case STATUS_PENDING:
				ImGui::TextUnformatted("Pending...");
				break;
			    case STATUS_OK:
				ImGui::TextUnformatted("Ok");
				break;
			    default: // error
				string errName, errDesc;
				if(getErrMsg(gcItem.status, &errName, &errDesc)) {	
				    cout << "Error: Can't find error code!" << endl;
				    exit(1);
				}
				ImGui::Text("Error %d", gcItem.status);
				ImGui::TableNextColumn();
				ImGui::Text("%s: %s", errName.c_str(), errDesc.c_str());
			}
		    }
		}
		if (scroll_To_Bottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		    ImGui::SetScrollHereY(1.0f);
		
		ImGui::EndTable();
	    }
	    ImGui::EndChild();
	    ImGui::EndTabItem();
	}
    }
    
    void Draw(GRBL* Grbl)
    {
	// initialise
	ImGui::SetNextWindowSize(ImVec2(570,270), ImGuiCond_Appearing);
	if (!ImGui::Begin("Console", NULL)) {
	    ImGui::End();
	    return;
	}
	// Disable all widgets when not connected to GRBL
	BeginDisableWidgets(Grbl);
	
	if (ImGui::BeginTabBar("Console", ImGuiTabBarFlags_None))
	{
	    DrawLogTab(Grbl);
	    DrawCommandsTab(Grbl);
	    
	    scroll_To_Bottom = false;
	    reclaim_focus = false;
	    
	    ImGui::EndTabBar();
	}
	
	ImGui::Separator();
	
	static char strConsole[128] = "";
	static bool send = false;
	
	send |= ImGui::Button("Send");
		
	ImGui::SameLine();
	 
	ImGui::PushItemWidth(200); // use -ve for distance from right size
	// Console text input
	send |= ImGui::InputText("Input GCode Here", strConsole, IM_ARRAYSIZE(strConsole), 
	    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this);
	
	ImGui::PopItemWidth();
	
	if(send) {
	    sendToConsole(Grbl, strConsole);
	    send = false;
	}
	
	// Auto-focus
	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
	    ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	
	ImGui::SameLine();
	
	if(log_Tab_Open) {
	    if (ImGui::Button("Clear Log"))
		Grbl->consoleLog->clear(); 
	}
	else {
	    if (ImGui::Button("Clear Commands")) 
		Grbl->gcList.ClearCompleted(); 
	}
	
	// Disable all widgets when not connected to GRBL
	EndDisableWidgets(Grbl);
	ImGui::End();
    }
    
    void sendToConsole(GRBL* Grbl, char* str) 
    {
	if(str[0]) {
	    Grbl->Send(str);
	    cout << "Sending: " << str << endl;
	    history.push_back(str);
	    historyCount++;
	    if(history.size() > MAX_HISTORY) {
		history.erase(history.begin());
		historyCount = MAX_HISTORY-1;
	    }
	    historyPos = historyCount + 1;
	    strncpy(str, "", 128);
	}
	reclaim_focus = true;
	scroll_To_Bottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        Console* console = (Console*)data->UserData;
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData* data)
    {
	// reset history position if user types anything
	if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) 
	    historyPos = historyCount + 1;
        // select through history of previous sent commands
        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) 
	{
	    if(historyCount == -1)
		return 0;
		    
	    if (data->EventKey == ImGuiKey_UpArrow)
	    {
		if(historyPos > 0)
		    historyPos--;
		else
		    return 0;
	    }
	    else if (data->EventKey == ImGuiKey_DownArrow)
	    {
		if(historyPos < historyCount)
		    historyPos++;
		else
		    return 0;
	    }
	    const char* history_str = (history.size() > 0) ? history[historyPos].c_str() : "";
	    data->DeleteChars(0, data->BufTextLen);
	    data->InsertChars(0, history_str);
        }
        return 0;
    }
};
  
#define SORT_BY_NAME 1
#define SORT_BY_DATE 2

#define MAX_DISPLAY_STRING 	30	// max no. characters to display in open file string
#define MAX_DISPLAY_FOLDERS	5 	// max no. folders to display

static const char* fileTypesDisplay[] 	= { "*.nc", "*.txt", "All Files" };
static const char* fileTypesRaw[] 	= {   "nc",   "txt",	      "" };

struct FileBrowser {



    ImageTexture img_File, img_Folder;
    uint timeStart = 0;
    uint timeCur = 0;
	
    string curFile;
    string curDir;
    string curFileFull;
    vector<string> curDirFolders;	// list of all folders inside curDir
    int selectedFileID = -1;
    int sortByName = true; // else sort by date
    int nodeSelected = -1;
    int nodeClicked = -1;
    bool resort = false; // used when manually requiring resort
    vector<filedesc_t> files;
    string showFileTypes = fileTypesRaw[0]; // filetypes to show in browser
	
    FileBrowser() 
    {
	img_File.Init(File::GetWorkingDir("img/img_file.png").c_str());
	img_Folder.Init(File::GetWorkingDir("img/img_folder.png").c_str());
    }
    
    void setCurrentDirectory(const string& dir) {
	// empty array if already full
	curDirFolders.clear();
	// set curDir string for easy access
	curDir = dir;
	// split dir into folders onto curDir
	istringstream stream(dir);
	string segment;
	
	for (string segment; getline(stream, segment, '/'); ) {
	    if(segment != "")
		curDirFolders.push_back(segment);
	}
	nodeSelected = curDirFolders.size()-1;
    }
    
    // update list of files and directories from within curDir
    void updateFiles()
    {	
	if(File::GetFilesInDir(curDir, showFileTypes, files)) {
	    cout << "Error: Can't access directory: " << curDir << endl;
	    return;
	}
    }
    // full update of directory & files
    void updateDirAndFiles(const string& dir) {
	setCurrentDirectory(dir);
	updateFiles();
	resort = true;
	selectedFileID = -1;
    }
    // initialisation
    void Init() {
	updateDirAndFiles(File::GetWorkingDir());
    }
    // shortens file path to "...path/at/place.gc" if length > MAX_DISPLAY_STRING
    string shortenFilePath(const string& str) {
	string s = str;
	if(s.length() > MAX_DISPLAY_STRING) {
	    s = s.substr(s.length()-MAX_DISPLAY_STRING);
	    return "..." + s;
	}
	return s;
    }
    
    // returns filename and type of file selected in browser
    int getSelectedFile(string& filename, int& filetype) 
    {
	if(selectedFileID == -1)
	    return -1;
	
	int i = 0;
	for(filedesc_t f : files) {
	    if(f.id == selectedFileID) {
		filetype = f.type;
		filename = f.name;
		// add on extension
		if(filetype == FILE_TYPE && f.ext != "")
		    filename += "." + f.ext;
		return 0;
	    }
	    i++;
	}
	// didn't find it - shouldn't ever reach
	cout << "Error: File not found in list?" << endl;
	cout << "selectedFileID: " << selectedFileID << endl;
	for (filedesc_t f : files)
	    cout << "name: " << f.name << "id: " << f.id << endl;
		
	filetype = -1;
	return -1;
    }
    
    void openSelectedFile()
    {
	int filetype;
	string filename;
	
	if (getSelectedFile(filename, filetype))
	    ImGui::CloseCurrentPopup(); // error, couldn't find it... shouldnt reach
	
	if(filetype == FILE_TYPE) {
	    curFile = filename;
	    curFileFull = File::CombineDirPath(curDir, curFile);
	    cout << "Opening file: " << curFileFull << endl;
	    curFileFull = shortenFilePath(curFileFull);
	    ImGui::CloseCurrentPopup();
	} else { //folder
	    updateDirAndFiles(File::CombineDirPath(curDir, filename));
	}
	
    }
    
    void sortFiles(const ImGuiTableColumnSortSpecs* sort_spec) 
    {
	auto compare = [&](const filedesc_t& a, const filedesc_t& b) {
	    
	    string strA, strB;
	    
	    switch (sort_spec->ColumnIndex)
	    {	// 0 is file/folder icon
		case 1: // name
		    strA = a.name;    strB = b.name;
		    lowerCase(strA);  lowerCase(strB);
		    if(sort_spec->SortDirection == ImGuiSortDirection_Ascending)
			return (strA < strB);
		    else
			return (strA > strB);
		    break;
		    
		case 2: // Type
		    strA = a.ext;    strB = b.ext;
		    lowerCase(strA); lowerCase(strB);
		    if(sort_spec->SortDirection == ImGuiSortDirection_Ascending)
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
		    
		    if(sort_spec->SortDirection == ImGuiSortDirection_Ascending)
			return (dateVal_a < dateVal_b);
		    else
			return (dateVal_a > dateVal_b);
		    break;
		    
		default:
		    exitf("Error: Column unknown\n");
	    }
	    return false;
	};
	
	auto compareType = [&](const filedesc_t& a, const filedesc_t& b) {
	    return a.type < b.type;
	};
	
	sort(files.begin(), files.end(), compare);
	sort(files.begin(), files.end(), compareType);
    }
    
    void DrawFolder(int i) {
	
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf  | ImGuiTreeNodeFlags_FramePadding;//ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if(i == nodeSelected)
	    node_flags |= ImGuiTreeNodeFlags_Selected;
	    
	if (ImGui::TreeNodeEx(curDirFolders[i].c_str(), node_flags))
	{
	    if (ImGui::IsItemClicked())
		nodeClicked = i;
	    i++;
	    if(i < (int)curDirFolders.size())
		DrawFolder(i);
	    
	    ImGui::TreePop();
	}
    }
    void DrawFolders() 
    {
	int s = curDirFolders.size();
	// recursively draw all the folders in curDirFolders (which are the individual folders of curDir)
	int startAt = (s > MAX_DISPLAY_FOLDERS) ? s - MAX_DISPLAY_FOLDERS : 0;
        DrawFolder(startAt);
	// draw blanks if less than number displayable
	if(s < MAX_DISPLAY_FOLDERS) {
	    for (int i = 0; i < MAX_DISPLAY_FOLDERS - s; i++)	    
		ImGui::Dummy(ImVec2(0, ImGui::GetFrameHeight()));
	}
	// if a folder is selected, mark it, and update curDir, curDirFolders & curFiles
	if(nodeClicked != -1) {
	    nodeSelected = nodeClicked;
	    nodeClicked = -1;
	    
	    string newDir = "/";
	    for (int j = 0; j < nodeSelected+1; j++) {
		if(j < (int)curDirFolders.size()) {	// just in case to prevent overflow
		    newDir += curDirFolders[j];
		    newDir += '/';
		}
	    }
	    updateDirAndFiles(newDir);
	}
    }
    void DrawFiles() 
    {
	static ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable 
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollY;
	

        if (ImGui::BeginTable("table_sorting", 4, flags, ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing() * 15), 0.0f)) { // ImVec2(0.0f, TEXT_BASE_HEIGHT * 15)
	    // Setup columns 	
	    // (add index numbers as next parameter to this if we want a more specific selection 
	    // i.e. ImGuiID filesColID_Name  and select using sort_spec->ColumnUserID)
	    ImGui::TableSetupColumn(" ", 		ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoResize,	0.0f);
            ImGui::TableSetupColumn("Name", 		ImGuiTableColumnFlags_DefaultSort, 	0.0f);		//ImGuiTableColumnFlags_WidthFixed / ImGuiTableColumnFlags_WidthStretch
            ImGui::TableSetupColumn("Extension", 	ImGuiTableColumnFlags_None, 		0.0f);
            ImGui::TableSetupColumn("Date Modified", 	ImGuiTableColumnFlags_None, 		0.0f);
	    // Make top row always visible
            ImGui::TableSetupScrollFreeze(0, 1); 
            ImGui::TableHeadersRow();

            // Resort data if sort specs has been changed
            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
                if (sorts_specs->SpecsDirty | resort) {
		    if (files.size() > 1) {
			sortFiles(&sorts_specs->Specs[0]);
		    }
                    sorts_specs->SpecsDirty = false;
		    resort = false;
                }
	    }
	    
            ImGuiListClipper clipper;
            clipper.Begin(files.size());
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    filedesc_t* item = &files[row_n];
                    const bool item_is_selected = (item->id == selectedFileID);
		    
                    ImGui::PushID(item->id);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
		    if(item->type == FOLDER_TYPE)
			ImGui::Image(img_Folder, ImVec2(16, 16));
		    else
			ImGui::Image(img_File, ImVec2(16, 16));
		    
                    ImGui::TableNextColumn();
		    if (ImGui::Selectable(item->name.c_str(), item_is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0))) 
		    {
			selectedFileID = item->id;
			  
			if (ImGui::IsMouseDoubleClicked(0))
			    openSelectedFile();
			else { // single click
			    int filetype;
			    string filename;
			    
			    if (getSelectedFile(filename, filetype))
				ImGui::CloseCurrentPopup(); // error, couldn't find it... shouldnt reach
			    
			    if(filetype == FOLDER_TYPE) 
				updateDirAndFiles(File::CombineDirPath(curDir, filename));
			}
		    }
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(item->ext.c_str());
                    ImGui::TableNextColumn();
		    
                    ImGui::TextUnformatted(item->lastModifiedStr.c_str());
		    ImGui::PopID();		   
                }
            ImGui::EndTable();
        }
    }
    
    void Draw()
    {	
	ImGui::TextUnformatted("Select the file you wish to open\n\n");
	ImGui::Separator();
            
	DrawFolders();
	DrawFiles();
	
	static int selectedFileType = 0;
	if (ImGui::Combo("Type", &selectedFileType, fileTypesDisplay, IM_ARRAYSIZE(fileTypesDisplay))) {
	    showFileTypes = fileTypesRaw[selectedFileType];
	    updateDirAndFiles(curDir);
	}
	 
	if (ImGui::Button("OK", ImVec2(120, 0))) 
	    openSelectedFile();
	    
	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	    ImGui::CloseCurrentPopup();
    }
};

struct Controller {
    
    FileBrowser fileBrowser;
    ImageTexture img_Play, img_Pause;
    uint timeStart = 0;
    uint timeCur = 0;
    
    Controller() {
	fileBrowser.Init(); 
	img_Play.Init(File::GetWorkingDir("img/img_restart.png").c_str());
	img_Pause.Init(File::GetWorkingDir("img/img_pause.png").c_str());
    }
    
    void Run(GRBL* Grbl) 
    {
	// check we have selected a file 
	if(fileBrowser.curFile == "") {
	    Grbl->consoleLog->push_back("Error: No file has been selected");
	    return;
	}
	// Check we haven't already got a file running
	if(Grbl->IsFileRunning()) {
	    Grbl->consoleLog->push_back("Error: File is already running");
	    return;
	}
	// check grbl is idle - not neccessarily needed but a good sanity check
	if(Grbl->WaitForIdle())
	    return;
	// get filepath of file
	string filepath = File::CombineDirPath(fileBrowser.curDir, fileBrowser.curFile);
	// add to log
	Grbl->consoleLog->push_back((string)"Sending File: " + filepath);
	// start time
	timeStart = millis();
	// send file to grbl
	if(Grbl->SendFile(filepath)) {
	    //couldn't open file
	    Grbl->consoleLog->push_back("Error: Couldn't send file to GRBL");
	}
	
    }
    
    void Draw(GRBL* Grbl) 
    {
        // initialise
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);	//ImVec2(520, 100)
	if (!ImGui::Begin("Run From File", NULL)) {
	    ImGui::End();
	    return;
	}
	// Disable all widgets when not connected to GRBL
	BeginDisableWidgets(Grbl);
	
	// button dimensions
	const int w = 70;
	const int h = 30; 
	
        if (ImGui::Button("Open..", ImVec2(w, h))) {
            ImGui::OpenPopup("Open File");
	}
	ImGui::SameLine();
	ImGui::TextUnformatted("Current File:");
	ImGui::SameLine();
	ImGui::TextUnformatted(fileBrowser.curFileFull.c_str());
	    
        if (ImGui::Button("Run", ImVec2(w, h))) 
	    Run(Grbl);
	    
	ImGui::SameLine();
	
	if (ImGui::Button("Cancel", ImVec2(w, h))) {	
	    if(Grbl->IsFileRunning()) 
	    {  
		Grbl->consoleLog->push_back("Cancelling... Note: Any commands remaining in GRBL's buffer will still execute.");
		Grbl->Cancel();
	    }
	}
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(50, 0));
	ImGui::SameLine();
	
	// dimensions of play/pause image
	int w2 = 18; 	int h2 = 18;
	// padding around image (i.e. button width = width of image + 2*padding)
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2((w-w2)/2, (h-h2)/2));
	    
	    ImGui::SameLine();
	    if (ImGui::ImageButton(img_Pause, ImVec2(w2, h2))) {
		Grbl->consoleLog->push_back("Pausing...");
		Grbl->SendRT(GRBL_RT_HOLD);
	    }
	    
	    ImGui::SameLine();
	    if (ImGui::ImageButton(img_Play, ImVec2(w2, h2))) {
		Grbl->consoleLog->push_back("Resuming...");
		Grbl->SendRT(GRBL_RT_RESUME);
	    }
	ImGui::PopStyleVar();
	
	
	
 	if(Grbl->gcList.IsFileRunning()) {
	    
	    // how far throuhg file we are
	    int pos = Grbl->gcList.GetFilePos();
	    int lines = Grbl->gcList.GetFileLines();
	    float percComplete = (float)pos / (float)lines;
		
	    if (ImGui::BeginTable("FileRunning", 2, ImGuiTableFlags_NoSavedSettings))
	    {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		    
		//ImGui::Text("%d/%d (%.2f%)", pos, lines, 100 * percComplete);
		
		// elapsed time		
		timeCur = millis();
		uint elapsed = (timeCur - timeStart) / 1000;
		uint hr = 0, min = 0, sec = 0;
		normaliseSecs(elapsed, hr, min, sec);
		ImGui::Text("Elapsed: %u:%.2u:%.2u", hr,min,sec);
				
		ImGui::TableSetColumnIndex(1);
		//estimate time remaining
		uint expected = elapsed / percComplete;
		uint remaining = expected - elapsed;
		hr = min = sec = 0;
		normaliseSecs(remaining, hr, min, sec);
		ImGui::Text("Estimated Remaining: %u:%.2u:%.2u", hr,min,sec);
		
		ImGui::EndTable();
		
	    }
	    
	    ImGui::PushItemWidth(-FLT_MIN);
	    // progress bar
	    char buf[64];
	    snprintf(buf, 64, "%d/%d (%.2f%%)", pos, lines, 100 * percComplete);
	    ImGui::ProgressBar(percComplete, ImVec2(0.0f, 0.0f), buf);
	    ImGui::PopItemWidth();
	} 
	
        // Always center the fileviewer windoe
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Open File", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
	    fileBrowser.Draw();
	    ImGui::EndPopup();
	}
	// Disable all widgets when not connected to GRBL
	EndDisableWidgets(Grbl);
	ImGui::End();
    }	
};
  
  

/*
x   std::string state;
	// either of these are given 
	// WPos = MPos - WCO
	point3D MPos;
x	point3D WPos;
	// this is given every 10 or 30 status messages
	point3D WCO;
	int lineNum;
	float feedRate;	// mm/min or inches/min
	int spindleSpeed; // rpm
	
	bool inputPin_LimX;
	bool inputPin_LimY;
	bool inputPin_LimZ;
	bool inputPin_Probe;
	bool inputPin_Door;
	bool inputPin_Hold;
	bool inputPin_SoftReset;
	bool inputPin_CycleStart;
	
	int override_Feedrate;
	int override_RapidFeed;
	int override_SpindleSpeed;
	
	int accessory_SpindleDir;
	int accessory_FloodCoolant;
	int accessory_MistCoolant;
	
*/
struct Stats 
{
    typedef struct {
	string name;
	string gcode; // may be seperated by ';'
    } customGC_t;

    vector<customGC_t> customGC;
    // sizes of buttons
    ImVec2 sizeL = { 80.0f, 60.0f };
    ImVec2 sizeS = { 55.0f, 30.0f };
    ImVec2 posS = (sizeL - sizeS) / 2;
    
    ImVec2 sizeCust= { 80.0, 35.0f };
    ImVec2 sizeCustNew = { 25.0, 25.0f };
    ImVec2 posCustNew = (sizeCust - sizeCustNew) / 2;

    Stats() 
    {
	importCustomGCs();
    }
    
#define IMPORT_TYPE_NONE 	0
#define IMPORT_TYPE_CUSTOMGC 	1

    void importCustomGCs()
    {
	vector<string> fileBuf;
		
	auto executeLine = [&fileBuf](string& str) {
	    if(str != "")
		fileBuf.push_back(str);
	    return 0;
	}; 
	
	string filename = File::GetWorkingDir("config.ini");
	if(File::Read(filename, executeLine)) {
	    cout << "Error: Could not open file " << filename << endl;
	    return;
	}
	
	// clear existing values
	customGC.clear();
	
	// initialise type of import
	int importType = IMPORT_TYPE_NONE;
	    
	// create buffer
	customGC_t gcBuf;
	
	for(string line : fileBuf)
	{
	    // If heading, set import type
	    if(line.substr(0, 13) == "[CustomGCode]") {
		importType = IMPORT_TYPE_CUSTOMGC;
		continue;
	    }
	    
	    // If value...
	    switch (importType)
	    {
		case IMPORT_TYPE_CUSTOMGC:
		    if(line.substr(0, 5) == "Name=")
			gcBuf.name = line.substr(5);
		    else if(line.substr(0, 6) == "GCode=") {
			gcBuf.gcode = line.substr(6);
			customGC.push_back(gcBuf);
			// clear
			gcBuf.name = gcBuf.gcode = "";
		    }
		    break;
		default:
		    cout << "Error: Unknown import type";
		    break;
	    }
	}
    }
    
    void exportCustomsGCs() 
    {
	ostringstream stream;
	for (size_t i = 0; i < customGC.size(); i++) {
	     stream << "[CustomGCode][" << i << "]" << endl;
	     stream << "Name=" << customGC[i].name << endl;
	     stream << "GCode=" << customGC[i].gcode << endl << endl;
	}
	string filename = File::GetWorkingDir("config.ini");
	File::Write(filename, stream.str());
    }
    
    void DrawConnect(GRBL* Grbl)
    {
	int buttonWidth = 100;
	// right align
	//ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - ImGui::GetStyle().WindowPadding.x);
	// centre align
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth)/2);
	if(!Grbl->IsConnected()) {
	    if(ImGui::Button("Connect", ImVec2(buttonWidth, 0))) {
		Grbl->Connect();
	    }
	}
	else if(Grbl->IsConnected()) {
	    if(ImGui::Button("Disconnect", ImVec2(buttonWidth, 0))) {
		Grbl->SoftReset();
		Grbl->Disconnect();
	    }
	}
    }
    
    void DrawStatus(GRBL* Grbl) 
    {
	grblStatus_t& status = Grbl->Param.status; 
	// current state colour
	ImVec4 colour;
	if(status.stateColour() == GRBL_STATE_COLOUR_IDLE)		// idle
	    colour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	else if(status.stateColour() == GRBL_STATE_COLOUR_MOTION)	// motion
	    colour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	else //if(status->stateColour = GRBL_STATE_COLOUR_ALERT)	// alert
	    colour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	// current state
	ImGui::TextColored(colour, status.state().c_str());
    }
    
    void DrawPosition(GRBL* Grbl) 
    {
	MainSettings& settings = Grbl->Param.settings;
	grblStatus_t& status = Grbl->Param.status;
	
	static bool showMachineCoords = false;
	
	ImGui::TextUnformatted(showMachineCoords ? "MPos" : "WPos");
	
	if (ImGui::BeginTable("Position", 3, ImGuiTableFlags_NoSavedSettings))
	{
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	    ImGui::TextUnformatted("X");
	    ImGui::TableSetColumnIndex(1);
	    ImGui::TextUnformatted("Y");
	    ImGui::TableSetColumnIndex(2);
	    ImGui::TextUnformatted("Z");
			
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	    ImGui::Text("%.3f", showMachineCoords ? status.MPos().x : status.WPos().x);
	    ImGui::TableSetColumnIndex(1);
	    ImGui::Text("%.3f", showMachineCoords ? status.MPos().y : status.WPos().y);
	    ImGui::TableSetColumnIndex(2);
	    ImGui::Text("%.3f", showMachineCoords ? status.MPos().z : status.WPos().z);
			
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	    ImGui::TextUnformatted(settings.units_Distance().c_str());	// mm
	    ImGui::TableSetColumnIndex(1);
	    ImGui::TextUnformatted(settings.units_Distance().c_str());
	    ImGui::TableSetColumnIndex(2);
	    ImGui::TextUnformatted(settings.units_Distance().c_str());
			
	    ImGui::EndTable();
	}
	showMachineCoords = (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) ? true : false;
    }
    void DrawMotion(GRBL* Grbl) 
    { 
	grblStatus_t& status = Grbl->Param.status;
	MainSettings& settings = Grbl->Param.settings;
		
	static float f[] = { 0.0f };
        static float s[] = { 0.0f };
			
	if (ImGui::BeginTable("Motion", 4, ImGuiTableFlags_NoSavedSettings))
	{
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	   // ImGui::Dummy(ImVec2(10.0f,0));
	    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
	    f[0] = status.feedRate();
	    ImGui::PlotHistogram("", f, IM_ARRAYSIZE(f), 0, NULL, 0.0f, settings.max_FeedRate(), ImVec2(20.0f, ImGui::GetFrameHeightWithSpacing() * 3));
	    
	    ImGui::TableSetColumnIndex(1);
	    ImGui::TextUnformatted("Feed Rate");
	    ImGui::Text("%.0f", status.feedRate());
	    ImGui::TextUnformatted(settings.units_Feed().c_str());
	    
	    ImGui::TableSetColumnIndex(2);
	    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
	    s[0] = (float)status.spindleSpeed();
	    ImGui::PlotHistogram("", s, IM_ARRAYSIZE(s), 0, NULL, (float)settings.min_SpindleSpeed(), (float)settings.max_SpindleSpeed(), ImVec2(20.0f, ImGui::GetFrameHeightWithSpacing() * 3));
	    
	    ImGui::TableSetColumnIndex(3);
	    ImGui::TextUnformatted("Spindle");
	    ImGui::Text("%.0d", status.spindleSpeed());
	    ImGui::TextUnformatted("RPM");
	
	    ImGui::EndTable();
	}
    }

    void DrawInputPins(GRBL* Grbl) {
	if (ImGui::BeginTable("InputPins", 4, ImGuiTableFlags_NoSavedSettings, ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 2)))
	{ 
	    grblStatus_t& s = Grbl->Param.status;
	    
	    // orange
	    ImVec4 colour = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);;
	
	    ImGui::TableNextRow();
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_LimX())
		ImGui::TextColored(colour, "Limit X");
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_LimY())
		ImGui::TextColored(colour, "Limit Y");
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_LimZ())
		ImGui::TextColored(colour, "Limit Z");
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_Probe())
		ImGui::TextColored(colour, "Probe");
	    ImGui::TableNextRow();
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_Door())
		ImGui::TextColored(colour, "Door");
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_Hold())
		ImGui::TextColored(colour, "Hold");
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_SoftReset())
		ImGui::TextColored(colour, "Reset");
	    ImGui::TableNextColumn();
	    
	    if(s.inputPin_CycleStart())
		ImGui::TextColored(colour, "Start");
	    
	    ImGui::EndTable();
	}
    }

    void DrawZeroing(GRBL* Grbl)
    {
	posS.y = 0; 
	
	if (ImGui::BeginTable("XYZ Zeroing", 3, ImGuiTableFlags_NoSavedSettings))
	{
	    ImGui::TableNextRow();
	    
	    ImGui::TableSetColumnIndex(0);
	    ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
	    if (ImGui::Button("Zero X", sizeS)) {
		Grbl->consoleLog->push_back("Setting current X position to 0 for this coord system");
		Grbl->Send("G10 L20 P0 X0");
	    }
	    ImGui::TableSetColumnIndex(1);
	    ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
	    if (ImGui::Button("Zero Y", sizeS)) {
		Grbl->consoleLog->push_back("Setting current Y position to 0 for this coord system");
		Grbl->Send("G10 L20 P0 Y0");
	    }
	    
	    ImGui::TableSetColumnIndex(2);
	    ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
	    if (ImGui::Button("Zero Z", sizeS)) {
		Grbl->consoleLog->push_back("Setting current Z position to 0 for this coord system");
		Grbl->Send("G10 L20 P0 Z0");
	    }
	    // position cursor to bottom corner of cell so it doesnt clip
	    //ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
	    
	    ImGui::EndTable();
	    
	}
    }
    
    void DrawCommands(GRBL* Grbl) 
    {
	
	if (ImGui::BeginTable("Commands", 3, ImGuiTableFlags_NoSavedSettings))
	{
	    ImGui::TableNextRow();

	    ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImVec4(1.0f, 0.8f, 0.8f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImVec4(1.0f, 0.6f, 0.6f, 0.6f)));	    
	    if(ImGui::Button("Reset", sizeL))
		Grbl->SoftReset();
            ImGui::PopStyleColor(3);
		    
	    ImGui::TableSetColumnIndex(1);
	    if(Grbl->Param.status.stateColour() == GRBL_STATE_COLOUR_ALERT) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImVec4(1.0f, 0.5f, 0.0f, 0.6f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImVec4(1.0f, 0.8f, 0.8f, 0.6f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImVec4(1.0f, 0.8f, 0.6f, 0.6f)));
	    }
	    if(ImGui::Button(" Clear\nAlarms", sizeL)) 
		Grbl->Send("$X");
	    if(Grbl->Param.status.stateColour() == GRBL_STATE_COLOUR_ALERT)
		ImGui::PopStyleColor(3);
		    
	    ImGui::TableSetColumnIndex(2);
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImVec4(0.6f, 0.8f, 0.6f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImVec4(0.8f, 0.9f, 0.8f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImVec4(0.6f, 0.8f, 0.6f, 0.6f)));
	    if(ImGui::Button("Home", sizeL)) 
		Grbl->Send("$H");
            ImGui::PopStyleColor(3);
	        
	    ImGui::TableNextRow();
	    
	    ImGui::TableSetColumnIndex(1);
	    if(ImGui::Button("Check\nMode", sizeL)) 
		Grbl->Send("$C");
	    
	    ImGui::EndTable();
	}	
    }	
    
    void DrawCustomGCodes(GRBL* Grbl) 
    {	// splits string using 
	auto sendCustomGCode = [Grbl](string gcodestr) {
	    istringstream s(gcodestr);
	    string segment;
	    for (string segment; getline(s, segment, ';'); ) {
		if(segment != "")
		    Grbl->Send(segment);
	    }
	};
	
	static int customGCIndex = 0;
	
	if (ImGui::BeginTable("Commands", 3, ImGuiTableFlags_NoSavedSettings))
	{
	    
	    for (size_t i = 0; i <= customGC.size(); i++) 
	    {
		if(i < MAX_CUSTOM_GCODES) 
		{
		    int rowIndex = i % 3;
		    // New row for every 3 custom gcodes
		    if(rowIndex == 0)
			ImGui::TableNextRow();
		    ImGui::TableSetColumnIndex(rowIndex);
		    //ImGui::SetCursorPos(posM);
		    // Add new button
		    if(i == customGC.size()) {
			// centre button in cell
			ImGui::SetCursorPos(ImGui::GetCursorPos() + posCustNew);
			if(ImGui::Button("+", sizeCustNew)) {
			    if(customGC.size() < MAX_CUSTOM_GCODES)
			    customGC.push_back((customGC_t){ "", "" });
			}
		    // Custom GCode button
		    } else {
			
			if(ImGui::Button(customGC[i].name.c_str(), sizeCust)) 
			    sendCustomGCode(customGC[i].gcode);
			
			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
			    customGCIndex = i;
			    ImGui::OpenPopup("Custom GCode");
			}
		    }
		}
	    }
	    static bool popupOpen = false;
	    if (ImGui::BeginPopup("Custom GCode"))
	    {
		popupOpen = true;
		ImGui::Text("Custom GCode #%d", customGCIndex+1);
		ImGui::SameLine();
		HelpMarker("GCode lines should be seperated with a semi-colon ("";"")");
		ImGui::SameLine();
		
		int delButtonW = 60;
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (delButtonW+10));
		if(ImGui::Button("Delete", ImVec2(delButtonW, 0))) {
		    customGC.erase(customGC.begin() + customGCIndex);
		    ImGui::CloseCurrentPopup();
		}
		ImGui::InputText("Name", &customGC[customGCIndex].name);
		ImGui::InputText("GCode", &customGC[customGCIndex].gcode);
		
		ImGui::EndPopup();
	    } else { // if popup has just been closed
		if(popupOpen == true) {
		    exportCustomsGCs();
		    popupOpen = false;
		}
	    }
	    ImGui::EndTable();
	}
    }
     
    void Draw(GRBL* Grbl)
    {
	// Initialise
	//ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
	if (!ImGui::Begin("Stats", NULL)) {
	    ImGui::End();
	    return;
	}
	// Connect / disconnect button
	DrawConnect(Grbl);
	// Disable all widgets when not connected to GRBL
	BeginDisableWidgets(Grbl);
	// GRBL State
	DrawStatus(Grbl);
	ImGui::Separator();
	// Current x y z location
	DrawPosition(Grbl);
	ImGui::Separator();
	// Current feedrate & spindle speed
	DrawMotion(Grbl); 
	ImGui::Separator();
	// Limit switches / probe
	DrawInputPins(Grbl);
	ImGui::Separator();
	// zeroing xyz
	DrawZeroing(Grbl);
	ImGui::Separator();
	
	DrawCommands(Grbl);
	ImGui::Separator();
	
	DrawCustomGCodes(Grbl);
	ImGui::Separator();
	
	ImGui::Checkbox("Status Report", &(Grbl->viewStatusReport));
	
	// Disable all widgets when not connected to GRBL
	EndDisableWidgets(Grbl);
	
	ImGui::End();
    }
};
  
struct JogController 
{
    bool allow_Keyb_Jogging = false;
    bool currently_Jogging = false;
    float jogDistance = 10;
    int feedRate;
	
    JogController(GRBL* Grbl) {
	feedRate = (int)Grbl->Param.settings.max_FeedRate(); // intialise to max feed
    }
    
    float calcuateJogDistance(float feedrate, float acc) 
    {
	    float v = feedrate / 60; // mm/s
	    int N = 15; // number blocks in planner buffer
	    
	    float dt = (v*v) / (2*acc*(N-1));
	    
	    cout << "@ " << feedrate << "mm/min" << endl;
	    cout << "dt = " << dt << endl;
	    
	    float smin = v*dt; // mm (smallest jog distance
	    
	    cout << "sMin = " << smin << endl << endl;
	    return smin;
    }

    void KeyboardJogging(GRBL* Grbl) 
    {
	// - This is a rather messy piece of code, but for now it solves the issue.
	// - When an arrow key is held, we want to repeatedly send jog commands to GRBL.
	// - The first issue is that we dont want to send more jog commands than the number of 
	// 	planner blocks (15) in GRBL, otherwise we have to remove any commands that haven't 
	//	been acknowledged. So we wait for an ok to be received before sending the 
	// 	next jog (this ensures a max. of 15 acknowledged + 1 pending jogs are sent)
	// - When we release an arrow key, we want to send a 'realtime jog cancel' cmd. When all jogs 
	//	have recieved an 'ok', this works fine, but if there is a pending jog in the queue,
	//	the cancel cmd executes first, clearing GRBL's buffer, which then allows the pending 
	//	jog to execute.
	//	- Option 1 was to wait for the 'ok' to arrive before sending the cancel command
	//		But this meant that we would have to wait for the last jog to execute which 
	//		could be a sizable distance.
	// 	- Option 2 was to repeatedly send cancel commands until we recieve and 'ok' for that 
	//		pending jog - not this most elegant fix but it seems to work for now.
	//	- Option 3 was to not allow too many jogs to be sent to GRBL to fill the planner 
	//		blocks, but the only way to know this information was from the status response 
	//		(Bf:15,128). This just seemed messy, as we are relying on a delayed response 
	//		from GRBL (or the status responses may not even be switched on)
	//	- Option 4 - send one long jog to end of table
	
	int KEY_LEFT = ImGui::GetKeyIndex(ImGuiKey_LeftArrow);
	int KEY_UP = ImGui::GetKeyIndex(ImGuiKey_UpArrow);
	int KEY_RIGHT = ImGui::GetKeyIndex(ImGuiKey_RightArrow);
	int KEY_DOWN = ImGui::GetKeyIndex(ImGuiKey_DownArrow);
	
	
	// option 4 - one long jog (must be > than the extents of the machine
	static int jogLongDistance = 10000;
	// on key release, send cancel
	if((ImGui::IsKeyReleased(KEY_LEFT) || ImGui::IsKeyReleased(KEY_RIGHT) || ImGui::IsKeyReleased(KEY_UP) || ImGui::IsKeyReleased(KEY_DOWN))
	&& (!ImGui::IsKeyPressed(KEY_LEFT) && !ImGui::IsKeyPressed(KEY_RIGHT) && !ImGui::IsKeyPressed(KEY_UP) && !ImGui::IsKeyPressed(KEY_DOWN))) 
	{
	    Grbl->SendRT(GRBL_RT_JOG_CANCEL);
	    currently_Jogging = false;
	} 
	else if(!currently_Jogging)
	{	// only allow combination of 2 buttons
	    if(ImGui::IsKeyPressed(KEY_LEFT) + ImGui::IsKeyPressed(KEY_UP) + ImGui::IsKeyPressed(KEY_RIGHT) + ImGui::IsKeyPressed(KEY_DOWN) <= 2)
	    {
		if(ImGui::IsKeyPressed(KEY_LEFT) && ImGui::IsKeyPressed(KEY_UP)) {
		    Grbl->SendJog(point3D(-jogLongDistance, jogLongDistance, 0), feedRate);
		    currently_Jogging = true;
		}
		else if(ImGui::IsKeyPressed(KEY_UP) && ImGui::IsKeyPressed(KEY_RIGHT)) {
		    Grbl->SendJog(point3D(jogLongDistance, jogLongDistance, 0), feedRate);
		    currently_Jogging = true;
		}
		else if(ImGui::IsKeyPressed(KEY_RIGHT) && ImGui::IsKeyPressed(KEY_DOWN)) {
		    Grbl->SendJog(point3D(jogLongDistance, -jogLongDistance, 0), feedRate);
		    currently_Jogging = true;
		}
		else if(ImGui::IsKeyPressed(KEY_DOWN) && ImGui::IsKeyPressed(KEY_LEFT)) {
		    Grbl->SendJog(point3D(-jogLongDistance, -jogLongDistance, 0), feedRate);
		    currently_Jogging = true;
		}
		
		else if(ImGui::IsKeyPressed(KEY_LEFT)) {
		    Grbl->SendJog(point3D(-jogLongDistance, 0, 0), feedRate);
		    currently_Jogging = true;
		}
		else if(ImGui::IsKeyPressed(KEY_RIGHT)) {
		    Grbl->SendJog(point3D(jogLongDistance, 0, 0), feedRate);
		    currently_Jogging = true;
		}
		else if(ImGui::IsKeyPressed(KEY_UP)) {
		    Grbl->SendJog(point3D(0, jogLongDistance, 0), feedRate);
		    currently_Jogging = true;
		}
		else if(ImGui::IsKeyPressed(KEY_DOWN)) {
		    Grbl->SendJog(point3D(0, -jogLongDistance, 0), feedRate);
		    currently_Jogging = true;
		}
	    }
	   
	}
	/*	option 1: wait to recieve ok before sending cancel
	// flag for when arrow key is released
	static bool send_Jog_Cancel = false;
	// have we recieved an 'ok' for every jog command we have sent?
	bool synced_With_Grbl = Grbl->gcList.status.size() == Grbl->gcList.read;
	// on key release, send cancel
	if((ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) || ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_RightArrow)) ||
		ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) || ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))) {
	    send_Jog_Cancel = true;
	} 
	else if(synced_With_Grbl && !send_Jog_Cancel)
	{
	    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
		Grbl->SendJog(X_AXIS, BACKWARD, jogDistance, feedRate);
	    else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
		Grbl->SendJog(X_AXIS, FORWARD, jogDistance, feedRate);
	    else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
		Grbl->SendJog(Y_AXIS, FORWARD, jogDistance, feedRate);
	    else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
		Grbl->SendJog(Y_AXIS, BACKWARD, jogDistance, feedRate);
	}
	// if synced, (last jog recieved 'ok') send cancel
	if(synced_With_Grbl && send_Jog_Cancel) {
	    Grbl->SendRT(GRBL_RT_JOG_CANCEL);
	    send_Jog_Cancel = false;
	}
	*/
	/* repeatedly send cancels
	// flag for when arrow key is released
	static bool send_Jog_Cancel = false;
	// have we recieved an 'ok' for every jog command we have sent?
	bool synced_With_Grbl = Grbl->gcList.status.size() == Grbl->gcList.read;
	// on key release, set flag to true
	if((ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) || ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_RightArrow)) ||
		ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) || ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))) {
	    send_Jog_Cancel = true;
	} // only send jog if an ok for the last jog is received & we are not waiting to cancel jog
	else if(synced_With_Grbl && !send_Jog_Cancel)
	{ 
	    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
		Grbl->SendJog(X_AXIS, BACKWARD, jogDistance, feedRate);
	    else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
		Grbl->SendJog(X_AXIS, FORWARD, jogDistance, feedRate);
	    else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
		Grbl->SendJog(Y_AXIS, FORWARD, jogDistance, feedRate);
	    else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
		Grbl->SendJog(Y_AXIS, BACKWARD, jogDistance, feedRate);
	}
	// repeatedly send cancels until the unacknowledged jog has been cancelled
	if(send_Jog_Cancel) {
	    Grbl->SendRT(GRBL_RT_JOG_CANCEL);
	    if (synced_With_Grbl)
		send_Jog_Cancel = false;
	}*/
    }

    void DrawJogController(GRBL* Grbl) 
    {
	ImGui::Checkbox("Control with arrow keys", &allow_Keyb_Jogging);
	if(allow_Keyb_Jogging)
	    KeyboardJogging(Grbl);
	
	int w = 40, h = 40;
	// Draw Jog XY
	ImGui::BeginGroup();
	
	    ImGui::Dummy(ImVec2(w, h));
	    ImGui::SameLine();
	    
	    if(ImGui::Button("Y+", ImVec2(w, h))) {
		if(!currently_Jogging)
		    Grbl->SendJog(point3D(0, jogDistance, 0), feedRate);
	    }
	       
	    if(ImGui::Button("X-", ImVec2(w, h))) {
		if(!currently_Jogging)
		    Grbl->SendJog(point3D(-jogDistance, 0, 0), feedRate);
	    }
	    ImGui::SameLine();	
	    ImGui::Dummy(ImVec2(w, h));
	    ImGui::SameLine();
	    
	    if(ImGui::Button("X+", ImVec2(w, h))) {
		if(!currently_Jogging)
		    Grbl->SendJog(point3D(jogDistance, 0, 0), feedRate);
	    }
	    ImGui::Dummy(ImVec2(w, h));
	    ImGui::SameLine();
	    
	    if(ImGui::Button("Y-", ImVec2(w, h))) {
		if(!currently_Jogging)
		    Grbl->SendJog(point3D(0, -jogDistance, 0), feedRate);
	    }
	   
	ImGui::EndGroup();
	
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 15);
	
	// Draw Jog Z
	ImGui::BeginGroup();
	    
	    if(ImGui::Button("Z+", ImVec2(w, h)))
		Grbl->SendJog(point3D(0, 0, jogDistance), feedRate);
		
	    ImGui::Dummy(ImVec2(w, h));
		
	    if(ImGui::Button("Z-", ImVec2(w, h)))
		Grbl->SendJog(point3D(0, 0, -jogDistance), feedRate);
			    
	ImGui::EndGroup();
    }
    void DrawJogSetting(GRBL* Grbl)
    {
	ImGui::PushItemWidth(100);
	ImGui::Indent();
	ImGui::InputFloat("Jog Distance", &jogDistance);
	ImGui::Unindent();
	
	Incrementer("Jog0.1", "0.1", 0.1f, jogDistance, false);
	ImGui::SameLine();
	Incrementer("Jog1", "1", 1.0f, jogDistance, false);
	ImGui::SameLine();
	Incrementer("Jog10", "10", 10.0f, jogDistance, false);
	
	ImGui::Separator();
	
	ImGui::Indent();
	ImGui::SliderInt("Feed Rate", &feedRate, 0, (int)Grbl->Param.settings.max_FeedRate());
	ImGui::Unindent();
	ImGui::PopItemWidth();
    }
	
    void Draw(GRBL* Grbl)
    {	// initialise
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
	if (!ImGui::Begin("Jog Controller", NULL)) {
		ImGui::End();
		return;
	}
	// Disable all widgets when not connected to GRBL
	BeginDisableWidgets(Grbl);
	
        ImGui::Separator();
	
	DrawJogController(Grbl);
	
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));
	
        ImGui::Separator();
	//feedrate / distance
	DrawJogSetting(Grbl);
	
	ImGui::PopStyleVar();
	
	// Disable all widgets when not connected to GRBL
	EndDisableWidgets(Grbl);
	ImGui::End();
    }
};
  
struct Overrides 
{
		//Grbl->SendRT(GRBL_RT_SPINDLE_STOP);
    /*
			
		Grbl->SendRT(GRBL_RT_FLOOD_COOLANT);
		Grbl->SendRT(GRBL_RT_MIST_COOLANT);
*/
    void Draw(GRBL* Grbl) 
    {
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
	if (!ImGui::Begin("Overrides", NULL)) {
	    ImGui::End();
	    return;
	}
	// Disable all widgets when not connected to GRBL
	BeginDisableWidgets(Grbl);
	
	
	
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));
	
	// Override Spindle Speed
	ImGui::Text("Override Spindle Speed: %d%%", Grbl->Param.status.override_SpindleSpeed());
	// unused parameter, we read directly from status reports instead
	static int spindleSpeed = 0;
		
	int buttonPress = Incrementer("ORSpindle1", "1%", 1, spindleSpeed, false);
	if(buttonPress == -1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT);
	else if(buttonPress == 1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT); 
	     
	ImGui::SameLine();
	buttonPress = Incrementer("ORSpindle10", "10%", 10, spindleSpeed, false);
	if(buttonPress == -1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT);
	else if(buttonPress == 1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT);  
	    
	ImGui::SameLine();
	if(ImGui::SmallButton("Reset##ORSpindleReset"))
	    Grbl->SendRT(GRBL_RT_OVERRIDE_SPINDLE_100PERCENT);
	
    
	ImGui::Separator();
	
	// Override Feed Rate	
	ImGui::Text("Override Feed Rate: %d%%", Grbl->Param.status.override_Feedrate());
	// Unused parameter, we read directly from status reports instead
	static int feedRate = 0;
	
	buttonPress = Incrementer("ORFeed1", "1%", 1, feedRate, false);
	if(buttonPress == -1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT);
	else if(buttonPress == 1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT); 
	     
	ImGui::SameLine();
	buttonPress = Incrementer("ORFeed10", "10%", 10, feedRate, false);
	if(buttonPress == -1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT);
	else if(buttonPress == 1) 
	    Grbl->SendRT(GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT);  
	    
	ImGui::SameLine();
	if(ImGui::SmallButton("Reset##ORFeedReset"))
	    Grbl->SendRT(GRBL_RT_OVERRIDE_FEED_100PERCENT);
	
	ImGui::Separator();
	
	// override rapid feed rate
	ImGui::Text("Override Rapid Feed Rate: %d%%", Grbl->Param.status.override_RapidFeed());
	
	
	if(ImGui::SmallButton("25%"))
	    Grbl->SendRT(GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT);
	ImGui::SameLine();
	if(ImGui::SmallButton("50%"))
	    Grbl->SendRT(GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT);
	ImGui::SameLine();
	if(ImGui::SmallButton("Reset##ORRapidFeed100"))
	    Grbl->SendRT(GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT);
	    
	ImGui::PopStyleVar();
	
	// Disable all widgets when not connected to GRBL
	EndDisableWidgets(Grbl);
	ImGui::End();
    }
};
  

void drawFrames(GRBL* Grbl)
{
    static Console console;
    console.Draw(Grbl);
    
    static Controller controller;
    controller.Draw(Grbl);
    
    static Stats stats;
    stats.Draw(Grbl);
    
    static JogController jogController(Grbl);
    jogController.Draw(Grbl);
    
    static Overrides overrides;
    overrides.Draw(Grbl);
    
}
  

