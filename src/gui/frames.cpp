/*
 * frames.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "../common.hpp"
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
  
#define MAX_HISTORY 100 
  
struct Console
{
    bool ScrollToBottom;
    bool reclaim_focus;
    bool visible;		
    int lines;
    vector<string> history;
    int historyCount = -1;
    int historyPos = 0;
	
    Console()
    {
        ScrollToBottom = false;
        visible = true;	
	lines = 0;
    }
    
    void Draw(GRBL* Grbl, ImGuiTextBuffer* log)
    {
	// initialise
	ImGui::SetNextWindowSize(ImVec2(0, 200), ImGuiCond_Appearing);
	if (!ImGui::Begin("Console", &visible)) {
	    ImGui::End();
	    return;
	}
	
	// Console    
	ImGui::BeginChild("Log", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - ImGui::GetTextLineHeight() - 26.0f));
	// print console log
        ImGui::PushTextWrapPos(0);//ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(log->begin(), log->end());
        ImGui::PopTextWrapPos();
	
	if (ScrollToBottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
	    ImGui::SetScrollHereY(1.0f);
	ScrollToBottom = false;
	reclaim_focus = false;
	
	ImGui::EndChild();
	
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
	    sendToConsole(Grbl, log, strConsole);
	    send = false;
	}
	
	// Auto-focus on window apparition
	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
	    ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	
	ImGui::SameLine();
	if (ImGui::Button("Clear")) { 
	    log->clear(); 
	    lines = 0; 
	}
	
	ImGui::End();
    }
    
    void sendToConsole(GRBL* Grbl, ImGuiTextBuffer* log, char* str) 
    {
	if(str[0]) {
	    Grbl->Send(str);
	    printf("Sending: %s\n", str);
	    log->append(str);
	    log->append("\n");
	    lines++;
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
	ScrollToBottom = true;
    }


    /*
    auto executeLine = [&](string& str) {
        Grbl->Send(&str);
    }; 
        
	readFile(file, executeLine)*/

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
	    const char* history_str = (history.size() >= 0) ? history[historyPos].c_str() : "";
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
static const char* fileTypesTrue[] 	= {   "nc",   "txt",	      "" };

struct FileBrowser {

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
    string showFileTypes = fileTypesTrue[0]; // filetypes to show in browser
    

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
	cout << showFileTypes << endl;
	if(getFilesInDir(curDir, showFileTypes, files)) {
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
    void Init(const string& workingDir) {
	updateDirAndFiles(workingDir);
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
	    curFileFull = getFilePath(curDir, curFile);
	    cout << "Opening file: " << curFileFull << endl;
	    curFileFull = shortenFilePath(curFileFull);
	    ImGui::CloseCurrentPopup();
	} else { //folder
	    updateDirAndFiles(getFilePath(curDir, filename));
	}
	
    }
    
    void sortFiles(const ImGuiTableColumnSortSpecs* sort_spec) 
    {
	auto compare = [&](const filedesc_t& a, const filedesc_t& b) {
	    
	    switch (sort_spec->ColumnIndex)
	    {	// 0 is file/folder icon
		case 1: // name
		    if(sort_spec->SortDirection == ImGuiSortDirection_Ascending)
			return (lowerCase(a.name) < lowerCase(b.name));
		    else
			return (lowerCase(a.name) > lowerCase(b.name));
		    break;
		    
		case 2: // Type
		    if(sort_spec->SortDirection == ImGuiSortDirection_Ascending)
			return (lowerCase(a.ext) < lowerCase(b.ext));
		    else
			return (lowerCase(a.ext) > lowerCase(b.ext));
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
    
    void DrawFolders(int i) {
	
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf  | ImGuiTreeNodeFlags_FramePadding;//ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if(i == nodeSelected)
	    node_flags |= ImGuiTreeNodeFlags_Selected;
	    
	if (ImGui::TreeNodeEx(curDirFolders[i].c_str(), node_flags))
	{
	    if (ImGui::IsItemClicked())
		nodeClicked = i;
	    i++;
	    if(i < curDirFolders.size())
		DrawFolders(i);
	    
	    ImGui::TreePop();
	}
    }
    void DrawFolders() 
    {
	int s = curDirFolders.size();
	// recursively draw all the folders in curDirFolders (which are the individual folders of curDir)
	int startAt = (s > MAX_DISPLAY_FOLDERS) ? s - MAX_DISPLAY_FOLDERS : 0;
        DrawFolders(startAt);
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
		if(j < curDirFolders.size()) {	// just in case to prevent overflow
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
	
	const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

        if (ImGui::BeginTable("table_sorting", 4, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f)) { // ImVec2(0.0f, TEXT_BASE_HEIGHT * 15)
	    // Setup columns 	
	    // (add index numbers as next parameter to this if we want a more specific selection 
	    // i.e. ImGuiID filesColID_Name  and select using sort_spec->ColumnUserID)
	    ImGui::TableSetupColumn("", 		ImGuiTableColumnFlags_NoSort, 		0.0f);
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
	    
            // Demonstrate using clipper for large vertical lists
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
			ImGui::TextUnformatted("Folder");
		    
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
				updateDirAndFiles(getFilePath(curDir, filename));
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
	    showFileTypes = fileTypesTrue[selectedFileType];
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
    
    bool visible = true;
    FileBrowser fileBrowser;
    
    Controller(const string& workingDir) {
	fileBrowser.Init(workingDir);
    }
    
    void Draw(GRBL* Grbl, ImGuiTextBuffer* log) 
    {
        // initialise
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);	//ImVec2(520, 100)
	if (!ImGui::Begin("Run From File", &visible)) {
	    ImGui::End();
	    return;
	}
	
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
	{   // check we have selected a file and we haven't already got a file running
	    if(fileBrowser.curFile != "" && !Grbl->IsFileRunning()) 
	    {	// check machine is idle
		if(!Grbl->WaitForIdle(log)) {
		    log->appendf("Sending File: %s\n", getFilePath(fileBrowser.curDir, fileBrowser.curFile).c_str());
		    if((Grbl->SendFile(getFilePath(fileBrowser.curDir, fileBrowser.curFile)))) {
			log->append("Error running file\n");
			//couldn't open file
		    }
		} else
		    log->append("Error: Timeout, no response recieved from grbl");
	    }
	}
	ImGui::SameLine();
	
	if (ImGui::Button("Cancel", ImVec2(w, h))) {	
	    if(Grbl->IsFileRunning()) 
	    {  
		log->append("Cancelling... Note: Any commands remaining in GRBL's buffer will still execute.\n");
		Grbl->Cancel();
	    }
	}
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(50, 0));
	ImGui::SameLine();
	
	if (ImGui::Button("Pause", ImVec2(w, h))) {
	    log->append("Pausing...\n");
	    Grbl->SendRT(GRBL_RT_HOLD);
	}
	ImGui::SameLine();
	if (ImGui::Button("Resume", ImVec2(w, h))) {
	    log->append("Resuming...\n");
	    Grbl->SendRT(GRBL_RT_RESUME);
	}
	
        // Always center the fileviewer windoe
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Open File", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
	    fileBrowser.Draw();
	    ImGui::EndPopup();
	}
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
	
	int accessory_SpindleDirection;
	int accessory_FloodCoolant;
	int accessory_MistCoolant;
	
*/
struct Stats 
{
    //bool visible = true;
    
    void DrawPosition(GRBL* Grbl) 
    {
	MainSettings* settings = &(Grbl->Param.settings);
	
	grblStatus_t* status = &(Grbl->Param.status);
	
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
	    ImGui::Text("%.3f", status->WPos.x);
	    ImGui::TableSetColumnIndex(1);
	    ImGui::Text("%.3f", status->WPos.y);
	    ImGui::TableSetColumnIndex(2);
	    ImGui::Text("%.3f", status->WPos.z);
			
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	    ImGui::TextUnformatted(settings->units_Distance.c_str());	// mm
	    ImGui::TableSetColumnIndex(1);
	    ImGui::TextUnformatted(settings->units_Distance.c_str());
	    ImGui::TableSetColumnIndex(2);
	    ImGui::TextUnformatted(settings->units_Distance.c_str());
			
	    ImGui::EndTable();
	}
    }
    void DrawMotion(GRBL* Grbl) 
    {
	grblStatus_t* status = &(Grbl->Param.status);
	MainSettings* settings = &(Grbl->Param.settings);
			
	if (ImGui::BeginTable("Motion", 2, ImGuiTableFlags_NoSavedSettings))
	{
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	    ImGui::TextUnformatted("Feed Rate");
	    ImGui::TableSetColumnIndex(1);
	    ImGui::TextUnformatted("Spindle");
	    
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	    ImGui::Text("%.0f", status->feedRate);
	    ImGui::TableSetColumnIndex(1);
	    ImGui::Text("%.0d", status->spindleSpeed);
			
	    ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(0);
	    ImGui::TextUnformatted(settings->units_Feed.c_str());
	    ImGui::TableSetColumnIndex(1);
	    ImGui::TextUnformatted("RPM");
			
	    ImGui::EndTable();
	}
    }

    void Draw(GRBL* Grbl)
    {
	 // initialise
	//ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
	if (!ImGui::Begin("Stats", NULL)) {
	    ImGui::End();
	    return;
	}
	
	//gCodeParams_t* p = &(Grbl->Param.gcParam);
	//modalGroup_t* m = &(Grbl->Param.mode);
	//grblStatus_t* s = &(Grbl->Param.status);
	grblStatus_t* status = &(Grbl->Param.status);
	// current state
	ImGui::TextUnformatted(status->state.c_str());
	ImGui::Separator();
	// current x y z location
	DrawPosition(Grbl);
	ImGui::Separator();
	// current feedrate & spindle speed
	DrawMotion(Grbl); 
	ImGui::Separator();
	
	if (ImGui::BeginTable("Commands", 3, ImGuiTableFlags_NoSavedSettings))
	{
	    int w = 80, h = 60;
		    
	    ImGui::TableNextRow();

	    ImGui::TableSetColumnIndex(0);
	    if(ImGui::Button(" Soft\nReset", ImVec2(w, h))) 
		Grbl->SendRT(GRBL_RT_SOFT_RESET);
		    
	    ImGui::TableSetColumnIndex(1);
	    if(ImGui::Button("  Kill\nAlarm\n Lock", ImVec2(w, h))) 
		Grbl->Send("$X");
		    
	    ImGui::TableSetColumnIndex(2);
	    if(ImGui::Button("Home", ImVec2(w, h))) 
		Grbl->Send("$H");
	    
	    ImGui::TableNextRow();
    
		ImGui::TableSetColumnIndex(0);
		if(ImGui::Button("Check\nMode", ImVec2(w, h))) 
		    Grbl->Send("$C");
		
		// return to zero
		// reset zero
		// get state
		
		ImGui::EndTable();
	    }
	    
	    ImGui::Checkbox("Status Report", &Grbl->verbose);
	    
	    
	    
	    ImGui::Separator();
	    
	    ImGui::End();
	}
};
  
struct JogController 
{
    bool visible = true;
    
    void Draw(GRBL* Grbl)
    {
	 // initialise
	//ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(246, 229), ImGuiCond_Always);
	if (!ImGui::Begin("Jog Controller", &visible)) {
		ImGui::End();
		return;
	}
	
	grblStatus_t& status = Grbl->Param.status;
	MainSettings& settings = Grbl->Param.settings;
	
	static float jogDistance = 10;
	static int feedRate = 6000;
				
		//jog controller
        if (ImGui::BeginTable("Jog Controller", 5, ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_NoSavedSettings))
        {
	    int w = 40, h = 40;
			
            ImGui::TableNextRow();
	    ImGui::TableSetColumnIndex(1);
	    if(ImGui::Button("Y+", ImVec2(w, h)))
		Grbl->SendJog(Y_AXIS, FORWARD, jogDistance, feedRate);
	    ImGui::TableSetColumnIndex(4);
	    if(ImGui::Button("Z+", ImVec2(w, h)))
		Grbl->SendJog(Z_AXIS, FORWARD, jogDistance, feedRate);
	    
	    ImGui::TableNextRow(ImGuiTableRowFlags_None);
	    ImGui::TableSetColumnIndex(0);
	    if(ImGui::Button("X-", ImVec2(w, h)))
		Grbl->SendJog(X_AXIS, BACKWARD, jogDistance, feedRate);
	    ImGui::TableSetColumnIndex(2);
	    if(ImGui::Button("X+", ImVec2(w, h)))
		Grbl->SendJog(X_AXIS, FORWARD, jogDistance, feedRate);
	    
	    ImGui::TableNextRow(ImGuiTableRowFlags_None);
	    ImGui::TableSetColumnIndex(1);
	    if(ImGui::Button("Y-", ImVec2(w, h)))
		Grbl->SendJog(Y_AXIS, BACKWARD, jogDistance, feedRate);
	    ImGui::TableSetColumnIndex(4);
	    if(ImGui::Button("Z-", ImVec2(w, h)))
		Grbl->SendJog(Z_AXIS, BACKWARD, jogDistance, feedRate);
			
            ImGui::EndTable();
        }
        ImGui::Separator();
		
	ImGui::PushItemWidth(100);
        ImGui::Indent();
	ImGui::InputFloat("Jog Distance", &jogDistance);
	float maxFeedRate = (settings.max_FeedRateX > settings.max_FeedRateY) ? settings.max_FeedRateX : settings.max_FeedRateY;
	ImGui::SliderInt("Feed Rate", &feedRate, 0, (int)maxFeedRate);
	ImGui::Unindent();
	ImGui::PopItemWidth();
	
	ImGui::End();
    }
};
  
void drawFrames(const string& workingDir, GRBL* Grbl, ImGuiTextBuffer* consoleLog)
{
    static Console console;
    console.Draw(Grbl, consoleLog);
    
    static Controller controller(workingDir);
    controller.Draw(Grbl, consoleLog);
    
    static Stats stats;
    stats.Draw(Grbl);
    
    static JogController jogController;
    jogController.Draw(Grbl);
    
}
  
