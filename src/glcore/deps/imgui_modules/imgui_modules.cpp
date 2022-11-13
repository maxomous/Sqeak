
#include "imgui_modules.h"
using namespace std; 
 
 
namespace ImGuiModules
{
    
ImGuiWindow::ImGuiWindow(std::string name, const ImVec2& position, const ImVec2& size) 
    : m_Name(name), m_Position(position), m_Size(size) 
{}

bool ImGuiWindow::Begin(ImGuiWindowFlags flags)
{
    // set default size / position
    ImGui::SetNextWindowSize(m_Size, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(m_Position, ImGuiCond_Appearing);
    
    if (!ImGui::Begin(m_Name.c_str(), NULL, flags)) {
        // window closed
        ImGui::End(); 
        return false;
    }
    // update size / position if user changed
    m_Size = ImGui::GetWindowSize();
    m_Position = ImGui::GetWindowPos();
        
    return true;
}

void ImGuiWindow::End() 
{ 
    ImGui::End(); 
}
 
 
 
ImGuiPopup::ImGuiPopup(const std::string& name) 
    : m_Name(name) 
{}

void ImGuiPopup::Open() 
{ 
    ImGui::OpenPopup(m_Name.c_str()); 
}


// returns true on close (next frame)
bool ImGuiPopup::Draw(std::function<void()> cb_ImGuiWidgets, ImGuiWindowFlags flags) 
{
    return Draw(cb_ImGuiWidgets, ImGui::BeginPopup(m_Name.c_str(), flags));
}

// returns true on close (next frame)
bool ImGuiPopup::DrawModal(std::function<void()> cb_ImGuiWidgets, ImGuiWindowFlags flags) 
{
    return Draw(cb_ImGuiWidgets, ImGui::BeginPopupModal(m_Name.c_str(), NULL, flags));
}

// returns true on close (next frame)
bool ImGuiPopup::Draw(std::function<void()> cb_ImGuiWidgets, bool isPopupVisible) 
{
    if (isPopupVisible) {
        // callback
        cb_ImGuiWidgets();
        ImGui::EndPopup();
        m_IsOpen = true;
    } else { // if popup has just been closed
        if (m_IsOpen == true) {
            m_IsOpen = false;
            return true;
        }
    }
    return false;
}



void KeepWindowInsideViewport() 
{
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    ImVec2 newPos = windowPos;
    
    if(windowPos.x < 0.0f)
        newPos.x = 0.0f;
    if(windowPos.y < 0.0f)
        newPos.y = 0.0f;
    if(windowPos.x + windowSize.x > screenSize.x)
        newPos.x = screenSize.x - windowSize.x;
    if(windowPos.y + windowSize.y > screenSize.y)
        newPos.y = screenSize.y - windowSize.y;
        
    ImGui::SetWindowPos(newPos);
}

// Converts hex colours e.g. 0xBFE0E0 to ImVec4
ImVec4 ConvertColourHexToVec4(ImU32 hexCol) {
    float s = 1.0f / 255.0f;
    return ImVec4(((hexCol >> 16) & 0xFF) * s, ((hexCol >> 8) & 0xFF) * s, ((hexCol >> 0) & 0xFF) * s, 1.0f);
}
 // Moves the cursor from current position
void MoveCursorPosX(float x) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);
}
void MoveCursorPosY(float y) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
}
void MoveCursorPos(ImVec2 distance) {
    ImGui::SetCursorPos(ImGui::GetCursorPos() + distance);
}
// produces n dummy items at standard frame height
void DummyItemHeight(uint nItems) {
    ImGui::Dummy(ImVec2(0.0f, ImGui::GetFrameHeight() * (float)nItems + ImGui::GetStyle().ItemSpacing.y));
}
// Returns the vertical centre of n items of standard height
float GetVerticalCentreOfItems(uint nItems) {
    return ((float)nItems * ImGui::GetFrameHeightWithSpacing() - (float)ImGui::GetStyle().ItemSpacing.y) / 2.0f;
}
// Sets the cursor y position such that an item of height 'itemHeight' is centred about 'nItems' of standard height
void CentreItemVertically(uint nItems, float thisItemHeight) {
    ImGuiModules::MoveCursorPosY(ImGuiModules::GetVerticalCentreOfItems(nItems) - thisItemHeight / 2.0f);
}
// Sets the cursor y position such that an item of height 'itemHeight' is centred about an arbitrary vertical distance
void CentreItemVerticallyAboutItem(float itemHeight, float thisItemHeight) {
    ImGuiModules::MoveCursorPosY((itemHeight - thisItemHeight) / 2.0f);
}
void TextUnformattedCentredHorizontally(const char* text, float width) {
    // calculate centre position from length of text
    ImGuiModules::MoveCursorPosX((width - ImGui::CalcTextSize(text).x) / 2.0f);
    // draw text
    ImGui::TextUnformatted(text);
}
void TextUnformattedCentredHorizontallyInTable(const char* text) {
    TextUnformattedCentredHorizontally(text, ImGui::GetColumnWidth());
}

void TextCentredHorizontally(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
        char buf[255];
        vsnprintf(buf, sizeof(buf), fmt, args);
        // calculate centre position from length of text
        ImGuiModules::TextUnformattedCentredHorizontally(buf);
    va_end(args);
}
void TextCentredHorizontallyInTable(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
        char buf[255];
        vsnprintf(buf, sizeof(buf), fmt, args);
        // calculate centre position from length of text
        ImGuiModules::TextUnformattedCentredHorizontallyInTable(buf);
    va_end(args);
}
bool WasLastItemRightClicked() {
    return (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right));
}
   
bool ImageButtonWithText(std::string name, ImVec2 buttonSize, ImageTexture& buttonImage, ImVec2 buttonImgSize, float imageYOffset, float textYOffset, ImFont* font)
{ 
    ImGui::BeginGroup();
        // get initial cursor position
        ImVec2 p0 = ImGui::GetCursorPos();
        // set small font for text on button
        ImGui::PushFont(font);
            // make text at bottom of button (values need to be normalised between 0-1)
            float yNormalised = textYOffset / (buttonSize.y - 2.0f*ImGui::GetStyle().FramePadding.y);
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.5f, yNormalised });
                // draw button
                bool isClicked = ImGui::Button(name.c_str(), buttonSize);
            ImGui::PopStyleVar();
        ImGui::PopFont();
        // get cursor end position 
        ImVec2 p1 = ImGui::GetCursorPos();
        
        // position of image
        ImVec2 imagePosition = { (buttonSize.x / 2.0f) - (buttonImgSize.x / 2.0f),    imageYOffset + ImGui::GetStyle().FramePadding.y}; // *** * 2  ??
        // set cursor for image
        ImGui::SetCursorPos(p0 + imagePosition);
        // draw image
        ImGui::Image(buttonImage, buttonImgSize);
        
        // reset cursor position to after button
        ImGui::SetCursorPos(p1);
    
    ImGui::EndGroup();
    return isClicked;
}


// Helper to display a little (?) mark which shows a tooltip when hovered.
void ToolTip_IfItemHovered(const std::string& text) {
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(text.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
void HelpMarker(const std::string& text) {
    ImGui::TextDisabled("(?)");
    ToolTip_IfItemHovered(text);
}

} // end namespace ImGuiModule
