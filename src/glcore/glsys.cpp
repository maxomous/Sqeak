#include <iostream>
using namespace std;

#include "glsys.h" 

Window::Window() {
    Event<Event_WindowResize>::RegisterHandler(&(Window::ResizeEvent));
}
Mouse::Mouse() { 
	Event<Event_MouseMove>::RegisterHandler(&(Mouse::MousePositionEvent));
	Event<Event_MouseButton>::RegisterHandler(&(Mouse::ButtonPressEvent));
} 

ActiveItem::ActiveItem() { 
	Event<Event_MouseButton>::RegisterHandler(&(ActiveItem::ButtonPressEvent));
} 
bool ActiveItem::IsViewportHovered(Camera& camera)
{
    if(ImGui::GetCurrentContext()->HoveredWindow)
        return false;
    return camera.IsInsideViewport(Window::InvertYCoord(Mouse::GetPositionClicked()));
}
bool ActiveItem::IsViewport(Camera& camera)
{
    if(get().m_ImGuiClicked)
        return false;
    return camera.IsInsideViewport(Window::InvertYCoord(Mouse::GetPositionClicked()));
}
void ActiveItem::ButtonPressEvent(Event_MouseButton data)
{
    if((data.Button != GLFW_MOUSE_BUTTON_LEFT) && (data.Button != GLFW_MOUSE_BUTTON_RIGHT) && (data.Button != GLFW_MOUSE_BUTTON_MIDDLE))
        return;
    if(data.Action == GLFW_PRESS)
        get().m_ImGuiClicked = ImGui::GetCurrentContext()->HoveredWindow;
    if(data.Action == GLFW_RELEASE)
        get().m_ImGuiClicked = false;
}

static void Callback_Error(int error, const char* description)
{
	std::cerr << "[GLFW Error] (" << error << "): " << description << std::endl;
}

static void Callback_ResizeWindow(GLFWwindow* m_Window, int width, int height)
{
	Event_WindowResize data = { m_Window, width, height };
	Event<Event_WindowResize>::Dispatch(data);
}

static void Callback_Keypress(GLFWwindow* m_Window, int key, int scancode, int action, int modifier)
{ 
	Event_KeyInput data = { m_Window, key, scancode, action, modifier };
	Event<Event_KeyInput>::Dispatch(data);
} 

static void Callback_CharacterInput(GLFWwindow* m_Window, uint32_t keycode)
{ 
	Event_CharInput data = { m_Window, keycode };
	Event<Event_CharInput>::Dispatch(data);
} 

static void Callback_SetMouseMove(GLFWwindow* window, double xPos, double yPos)
{ 
	Event_MouseMove data = { window, xPos, yPos }; 
	Event<Event_MouseMove>::Dispatch(data); 
} 

static void Callback_SetMouseButton(GLFWwindow* window, int button, int action, int modifier)
{   
	Event_MouseButton data = { window, button, action, modifier };
	Event<Event_MouseButton>::Dispatch(data);
} 
static void Callback_SetMouseScroll(GLFWwindow* window, double xOffset, double yOffset)
{
	Event_MouseScroll data = { window, xOffset, yOffset };
	Event<Event_MouseScroll>::Dispatch(data);
}

GLSystem::GLSystem(int w, int h, const char* name, const char* glsl_version)
{        
    if (!glfwInit()) {
        std::cerr << "Error: Couldn't initialise glfw" << std::endl;
        exit(1);
    }
        
    glfw_ConfigVersion();
    
    if(glfw_InitWindow(w, h, name))
        exit(1);
        
    glfw_Config();
    
    if(glew_Init())
        exit(1);
   
    imgui_Init(glsl_version); // glsl version        
    imgui_Config();
}
GLSystem::~GLSystem()
{
    imgui_Shutdown();
    glfw_Shutdown();
}

int GLSystem::glfw_InitWindow(int w, int h, const char* name)
{
	/* Create a windowed mode window and its OpenGL context */
	m_Window = glfwCreateWindow(w, h, name, nullptr, nullptr);
	if (!m_Window) {
		std::cerr << "Error: Couldn't initialise window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(m_Window);
	// initialise the m_Window size
    Window::SetWidth(w); 
    Window::SetHeight(h);
	// initialise mouse with dummy call
    Mouse::GetPosition();
    
	std::cout << "OpenGL Info:" << std::endl;
	std::cout << "  Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "  Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "  Version: " << glGetString(GL_VERSION) << std::endl;
	
	// Syncronise with monitors refresh rate (vsync)
	glfwSwapInterval(1);
	// Set GLFW callbacks
	glfwSetErrorCallback(Callback_Error);
	glfwSetWindowSizeCallback(m_Window, Callback_ResizeWindow);
	glfwSetKeyCallback(m_Window, Callback_Keypress);
	glfwSetCharCallback(m_Window, Callback_CharacterInput); // called when printable key is pressed
	glfwSetCursorPosCallback(m_Window, Callback_SetMouseMove); 
	glfwSetMouseButtonCallback(m_Window, Callback_SetMouseButton);
	glfwSetScrollCallback(m_Window, Callback_SetMouseScroll);

	return 0;
}

int GLSystem::glew_Init()
{
    glewExperimental = GL_TRUE;
    // initialise glew
    if (glewInit() != GLEW_OK) {
		cerr << "Error: Couldn't initialise glew" << endl;
		return -1;
	}
    return 0;
}

void GLSystem::imgui_Init(const char* glsl_version)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}
      
void GLSystem::imgui_NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void GLSystem::imgui_Render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
}
void GLSystem::imgui_Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GLSystem::glfw_Shutdown()
{
	GLCall(glfwDestroyWindow(m_Window));
    GLCall(glfwTerminate());
}
