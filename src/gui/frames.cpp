/*
 * frames.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "../common.hpp"
using namespace std;


  
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
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Console", &(visible))) {
			ImGui::End();
			return;
		}
		// Console    
		ImGui::BeginChild("Log", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - ImGui::GetTextLineHeight() - 26.0f));
		// Single call to TextUnformatted() with a big buffer
		ImGui::TextUnformatted(log->begin(), log->end());
		
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
		printf("string to console = %s\n", str);
		if(str[0]) {
			Grbl->Send(str);
			printf("Send: %s\n", str);
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
    bool visible = true;
    
    void DrawPosition(GRBL* Grbl) 
    {
		MainSettings* settings = &(Grbl->Param.settings);
		
		char distanceUnit[16];
		strncpy(distanceUnit, settings->units_Distance.c_str(), 16);
		char feedrateUnit[16];
		strncpy(feedrateUnit, settings->units_Feed.c_str(), 16);
		
		grblStatus_t* status = &(Grbl->Param.status);
		
		if (ImGui::BeginTable("Position", 3))
        {
            ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("X");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Y");
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("Z");
			
			char x[10], y[10], z[10];
			snprintf(x, 10, "%.3f", status->WPos.x);
			snprintf(y, 10, "%.3f", status->WPos.y);
			snprintf(z, 10, "%.3f", status->WPos.z);
			
            ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(x);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(y);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text(z);
			
            ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(distanceUnit);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(distanceUnit);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text(distanceUnit);
			
            ImGui::EndTable();
		}
	}
	void DrawMotion(GRBL* Grbl) 
    {
		grblStatus_t* status = &(Grbl->Param.status);
		MainSettings* settings = &(Grbl->Param.settings);
				
		if (ImGui::BeginTable("Motion", 2))
        {
            ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Feed Rate");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Spindle");
			
			char feed[10], spindle[10];
			snprintf(feed, 10, "%.0f", status->feedRate);
			snprintf(spindle, 10, "%d", status->spindleSpeed);
			
            ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(feed);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(spindle);
			
            ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(settings->units_Feed.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("RPM");
			
            ImGui::EndTable();
		}
	}
    
	void Draw(GRBL* Grbl)
	{
		 // initialise
		ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Stats", &visible)) {
			ImGui::End();
			return;
		}
		
		//gCodeParams_t* p = &(Grbl->Param.gcParam);
		//modalGroup_t* m = &(Grbl->Param.mode);
		//grblStatus_t* s = &(Grbl->Param.status);
		grblStatus_t* status = &(Grbl->Param.status);
		
		// current state
		ImGui::Text(status->state.c_str());
		ImGui::Separator();
		// current x y z location
		DrawPosition(Grbl);
		ImGui::Separator();
		// current feedrate & spindle speed
		DrawMotion(Grbl); 
		ImGui::Separator();
		
		
		if (ImGui::BeginTable("Commands", 3))
        {
			int w = 80, h = 60;
			
            ImGui::TableNextRow();
            
			ImGui::TableSetColumnIndex(0);
			if(ImGui::Button("Soft Reset", ImVec2(w, h))) 
				Grbl->SendRT(GRBL_RT_SOFT_RESET);
				
			ImGui::TableSetColumnIndex(1);
			if(ImGui::Button("Kill Alarm Lock", ImVec2(w, h))) 
				Grbl->Send("$X");
				
			ImGui::TableSetColumnIndex(2);
			if(ImGui::Button("Home", ImVec2(w, h))) 
				Grbl->Send("$H");
			
            ImGui::TableNextRow();
            
			ImGui::TableSetColumnIndex(0);
			if(ImGui::Button("Check Mode", ImVec2(w, h))) 
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
		ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Jog Controller", &visible)) {
			ImGui::End();
			return;
		}
		
		grblStatus_t* status = &(Grbl->Param.status);
		
		//grblStatus_t* status = &(Grbl->Param.status);
		MainSettings* settings = &(Grbl->Param.settings);
		static float jogDistance = 10;
		static int feedRate = 6000;
		
		ImGui::Text("Jog Contoller");
				
		//jog controller
        if (ImGui::BeginTable("Jog Controller", 5, ImGuiTableFlags_SizingFixedSame))
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
			float maxFeedRate = (settings->max_FeedRateX > settings->max_FeedRateY) ? settings->max_FeedRateX : settings->max_FeedRateY;
			ImGui::SliderInt("Feed Rate", &feedRate, 0, (int)maxFeedRate);
		ImGui::Unindent();
		ImGui::PopItemWidth();
		
		ImGui::End();
	}
};
  
void drawFrames(GRBL* Grbl, ImGuiTextBuffer* consoleLog)
{
    static Console console;
    console.Draw(Grbl, consoleLog);
    static Stats stats;
    stats.Draw(Grbl);
    static JogController jogController;
    jogController.Draw(Grbl);
    
    
}
  

