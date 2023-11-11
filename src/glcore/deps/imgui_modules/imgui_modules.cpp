
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

void ProbeWidget() {
    // get height of line / frame
    std::cout << "LineHeight = " << ImGui::GetTextLineHeight() << " (With Spacing) = " << ImGui::GetTextLineHeightWithSpacing() << std::endl;
    std::cout << "FrameHeight = " << ImGui::GetFrameHeight() << " (With Spacing) = " << ImGui::GetFrameHeightWithSpacing() << std::endl;
    
    std::cout << "FrameHeight = " << ImGui::GetFrameHeight() << " (With Spacing) = " << ImGui::GetFrameHeightWithSpacing() << std::endl;
    

    std::cout << "IsItemHovered + flags = "     << ImGui::IsItemHovered()               << std::endl;       // allows flags 
    std::cout << "IsItemActive = "              << ImGui::IsItemActive()                << std::endl;                                    
    std::cout << "IsItemFocused = "             << ImGui::IsItemFocused()               << std::endl;                                   
    std::cout << "IsItemClicked + flags = "     << ImGui::IsItemClicked()               << std::endl;       // allows flags 
    std::cout << "IsItemVisible = "             << ImGui::IsItemVisible()               << std::endl;                                   
    std::cout << "IsItemEdited = "              << ImGui::IsItemEdited()                << std::endl;                                    
    std::cout << "IsItemActivated = "           << ImGui::IsItemActivated()             << std::endl;                                 
    std::cout << "IsItemDeactivated = "         << ImGui::IsItemDeactivated()           << std::endl;                               
    std::cout << "IsItemDeactivatedAfterEdit = "<< ImGui::IsItemDeactivatedAfterEdit()  << std::endl;                      
    std::cout << "IsItemToggledOpen = "         << ImGui::IsItemToggledOpen()           << std::endl;                               
    std::cout << "IsAnyItemHovered = "          << ImGui::IsAnyItemHovered()            << std::endl;                                
    std::cout << "IsAnyItemActive = "           << ImGui::IsAnyItemActive()             << std::endl;                                 
    std::cout << "IsAnyItemFocused = "          << ImGui::IsAnyItemFocused()            << std::endl;                                
    std::cout << "GetItemRectMin = "            << ImGui::GetItemRectMin()              << std::endl;                                  
    std::cout << "GetItemRectMax = "            << ImGui::GetItemRectMax()              << std::endl;                                  
    std::cout << "GetItemRectSize = "           << ImGui::GetItemRectSize()             << std::endl;                              

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

bool ImageButtonWithText(const std::string& text, ImageTexture image, const ImVec2& buttonSize, const ImVec2& imageSize, ImFont* font, const ImVec2& textOffset, const ImVec2& imageOffset, bool isActive, ImageTexture hoveredImage)
{ 
    ImGui::BeginGroup();
        // get initial cursor position
        ImVec2 p0 = ImGui::GetCursorPos();
        // set font for text on button
        ImGui::PushFont(font); 
        if(isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
        
            // Convert Text offset so that is is moving from 0 < x < 1  where 0 & 1 is text touching frame padding either side 
            ImVec2 innerFrame = buttonSize - ImGui::GetStyle().FramePadding*2.0f;
            // calculate text width (ignore everything after ##)
            ImVec2 textDimensions = ImGui::CalcTextSize(text.c_str(), NULL, true);
            // amount of movement text travels
            ImVec2 motionRange = innerFrame - textDimensions;
            // move 1/2 width of text + offset
            float halfTextOffset = textDimensions.x / (motionRange.x * 2.0f);
            ImVec2 offset = textOffset / motionRange;
            // Centre if offset is 0, left align if offset is +ve, right align isf offset is -ve
            if(textOffset.x < 0.0f) { halfTextOffset = -halfTextOffset; }
            else if(textOffset.x == 0.0f) { halfTextOffset = 0.0f; }
            // Sum the offsets. Final offset where 0 < x < 1 === motion traveled
            ImVec2 totalOffset = ImVec2(0.5f, 0.5f) + offset + ImVec2(halfTextOffset, 0.0f);

            // Offset text
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, totalOffset); 
                // draw button
                bool isClicked = ImGui::Button(text.c_str(), buttonSize);
            ImGui::PopStyleVar();
            
        if(isActive) ImGui::PopStyleColor();
        ImGui::PopFont();
        
        
        // get cursor end position  
        ImVec2 p1 = ImGui::GetCursorPos();
        // Offset Image
        // Move distance of image (imageOffset is 0 to 1, where:  Left 0 (0)  /  Centre 0.5 (outer-inner)/2  /  Right 1 (outer-inner)
        //ImVec2 moveDistance = (buttonSize - ImGui::GetStyle().FramePadding*2.0f - imageSize) * imageOffset;
        
        ImVec2 offetToCentre = (buttonSize-imageSize)/2.0f;
        ImGui::SetCursorPos(p0 + offetToCentre + imageOffset);
        // If hovered, set image to hoveredImage (if provided)
        ImageTexture imageOut = (hoveredImage.textureID != 0 && ImGui::IsItemHovered()) ? hoveredImage : image;
        // draw image
        ImGui::Image(imageOut, imageSize);
        
        // reset cursor position to after button
        ImGui::SetCursorPos(p1);
    
    ImGui::EndGroup();
    
    return isClicked;
}

bool ImageButtonWithText(const std::string& text, ImageTexture image, ImageButtonStyle* imageButton, bool isActive, ImageTexture hoveredImage)
{  
    return ImageButtonWithText(text, image, imageButton->buttonSize, imageButton->imageSize, imageButton->font, imageButton->textOffset, imageButton->imageOffset, isActive, hoveredImage);
}

bool ImageButton(ImageTexture image, ImageButtonStyle* imageButton, bool isActive, ImageTexture hoveredImage)
{ 
    // Set active
    if(isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
    // If hovered, set image to hoveredImage (if provided)
    ImageTexture imageOut = (hoveredImage.textureID != 0 && ImGui::IsItemHovered()) ? hoveredImage : image;
    // Draw image button
    bool isClicked = ImGui::ImageButton(imageButton->buttonSize, imageButton->imageSize, imageOut); 
    // Deactive
    if(isActive) ImGui::PopStyleColor();
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
