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
			strcpy(str, "");
		}
		reclaim_focus = true;
		ScrollToBottom = true;
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
			const char* history_str = (history.size() >= 0) ? history[historyPos].c_str() : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
        }
        return 0;
    }
};
  

struct Stats 
{
    bool visible = true;
    
	void Draw(GRBL* Grbl)
	{
		 // initialise
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Stats", &visible)) {
			ImGui::End();
			return;
		}
		
		if(ImGui::Button("Soft Reset")) 
			Grbl->SendRT(GRBL_RT_SOFT_RESET);
		ImGui::SameLine();
		if(ImGui::Button("Kill Alarm Lock")) 
			Grbl->Send("$X");
			
		ImGui::Checkbox("Status Report", &Grbl->verbose);
		
		gCodeParams_t* p = &(Grbl->Param.gcParam);
		modalGroup_t* m = &(Grbl->Param.mode);
		grblStatus_t* s = &(Grbl->Param.status);
		
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::Text("Z");
		
		char x[10], y[10], z[10];
		snprintf(x, 10, "%.3f", s->WPos.x);
		snprintf(y, 10, "%.3f", s->WPos.y);
		snprintf(z, 10, "%.3f", s->WPos.z);
		
		ImGui::Text(x);
		ImGui::SameLine();
		ImGui::Text(y);
		ImGui::SameLine();
		ImGui::Text(z);
			
		ImGui::Separator();
		
		
		ImGui::End();
		
	}
};
  
  
void drawFrames(GRBL* Grbl, ImGuiTextBuffer* consoleLog)
{
    static Console console;
    console.Draw(Grbl, consoleLog);
    static Stats stats;
    stats.Draw(Grbl);
    
}
  

