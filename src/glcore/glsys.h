#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "glcore.h"

struct Event_WindowResize
{
	GLFWwindow* m_Window;
    int Width;
    int Height;
};
struct Event_KeyInput
{
	GLFWwindow* m_Window;
    int Key;
    int Scancode;
    int Action;
    int Modifier;
};
struct Event_CharInput
{
	GLFWwindow* m_Window;
    uint32_t keycode;
};
struct Event_MouseMove
{
	GLFWwindow* m_Window;
    double PosX; 
    double PosY;
};
struct Event_MouseButton
{
	GLFWwindow* m_Window;
    int Button; 
    int Action; 
    int Modifier;
};
struct Event_MouseScroll
{
	GLFWwindow* m_Window;
    double OffsetX; 
    double OffsetY;
};

// EventDispatcher singleton
template<typename T>
class Event
{
public:
    // returns unique id of handler
    static uint RegisterHandler(const std::function<void(T data)>& eventHandler)
    {
        get().m_EventHandlers.push_back(eventHandler);
        return get().m_EventHandlers.size() - 1;
    }
    static void UnregisterHandler(uint id)
    {
        get().m_EventHandlers.erase(get().m_EventHandlers.begin() + id);
    } 
    
    static void Dispatch(T newEvent) 
    {        
        for(const std::function<void(T)>& eventHandler : get().m_EventHandlers) {
            eventHandler(newEvent);
        }
    }
private:
    std::vector<std::function<void(T)>> m_EventHandlers;
    
    static Event& get() 
    {
		static Event instance;
        return instance;
    }
    // delete constructor / copy constructor / assignment operator
    Event() {}
    Event(const Event&) = delete;
    Event& operator= (const Event&) = delete;
};

template<typename T>
class EventHandler
{
public:
    EventHandler(const std::function<void(T data)>& eventHandler) {
        m_ID = Event<T>::RegisterHandler(eventHandler);
    }
    ~EventHandler() {
        Event<T>::UnregisterHandler(m_ID);
    }
private:
    uint m_ID;
};

class Window
{
public:
	static int GetWidth()           { return get().m_Width; }
	static int GetHeight()          { return get().m_Height; }
	static void SetWidth(int w)     { get().m_Width = w; }
	static void SetHeight(int h)    { get().m_Height = h; }
	
	// opengl's origin is the bottom left of the screen, this makes it the top left
	static glm::vec2 InvertYCoord(glm::vec2 pos) { return { pos.x, get().m_Height - pos.y }; };
	
    static void Resize(int w, int h)
	{
		get().m_Width = w;
		get().m_Height = h;
	}
    
	static void ResizeEvent(Event_WindowResize data) { Resize(data.Width, data.Height); }
	
private:
	int m_Width = 0;
	int m_Height = 0;
	
	static Window& get() {
		static Window Window;
		return Window;
    }

    Window();
    Window(const Window&) = delete;
    Window& operator= (const Window&) = delete;
};

class Mouse
{
public:
	static bool IsLeftClicked() 		    { return get().m_Buttons[0]; }
	static bool IsRightClicked() 		    { return get().m_Buttons[1]; }
	static bool IsMiddleClicked() 		    { return get().m_Buttons[2]; }
	static glm::vec2 GetPosition() 			{ return get().m_Position; }
	static glm::vec2 GetPositionDif() 	    { return get().m_Position - get().m_PositionLast; }
	static glm::vec2 GetPositionClicked() 	{ return get().m_PositionClicked; }
	
	static void MousePositionEvent(Event_MouseMove data) 
    { 
        get().m_PositionLast = get().m_Position;
        get().m_Position = { data.PosX, data.PosY }; 
    }
	
	static void ButtonPressEvent(Event_MouseButton data)
	{
        if((data.Button != GLFW_MOUSE_BUTTON_LEFT) && (data.Button != GLFW_MOUSE_BUTTON_RIGHT) && (data.Button != GLFW_MOUSE_BUTTON_MIDDLE))
            return;
        if(data.Action == GLFW_PRESS) {
            get().m_PositionClicked = get().m_Position;
        }
        get().m_Buttons[data.Button] = data.Action;
	} 
        
private:
	glm::vec2 m_Position;
	glm::vec2 m_PositionLast;
	glm::vec2 m_PositionClicked;
	bool m_Buttons[3] = { false, false, false };
	
    static Mouse& get() {
		static Mouse mouse;
		return mouse;
    }

    Mouse();
    Mouse(const Mouse&) = delete;
    Mouse& operator= (const Mouse&) = delete;
};

class ActiveItem
{
public:

    static bool IsViewportHovered(Camera& camera);
    static bool IsViewport(Camera& camera);
   	static void ButtonPressEvent(Event_MouseButton data);
    
private:
    bool m_ImGuiClicked = false;  
    
    static ActiveItem& get() {
		static ActiveItem activeItem;
		return activeItem;
    }

    ActiveItem();
    ActiveItem(const ActiveItem&) = delete;
    ActiveItem& operator= (const ActiveItem&) = delete;      
};

class Timer
{
public:
    Timer() { Reset(); }
    
    void Reset() { m_CurrentTime = m_PreviousTime = glfwGetTime(); }
    // updates the current time
    void UpdateCurrentTime() { m_CurrentTime = glfwGetTime(); }
    // moves the interval time each call
    void UpdateInterval() {
		m_PreviousTime = m_CurrentTime;
        m_CurrentTime = glfwGetTime();
    }
    double dt() { return m_CurrentTime - m_PreviousTime; }
private:
    double m_CurrentTime = 0.0f;
    double m_PreviousTime = 0.0f;
};

class GLSystem
{
public:
    GLSystem(int w, int h, const char* name, const char* glsl_version, std::function<void()> cb_GLFW_ConfigVersion, std::function<void(GLFWwindow*)> cb_GLFW_Config, std::function<void(GLFWwindow*)> cb_imgui_Config);
    ~GLSystem();
    
    GLFWwindow*& GetWindow() { return m_Window; };
    
    int glfw_InitWindow(int w, int h, const char* name);
    int glew_Init();
    void imgui_Init();
    void imgui_Impl(const char* glsl_version);
    
    void glfw_ConfigVersion();
    void glfw_Config();
    void imgui_Config();
          
    void imgui_NewFrame();
    void imgui_Render();

    void imgui_Shutdown();
    void glfw_Shutdown();
private:
    GLFWwindow* m_Window = nullptr;
};

