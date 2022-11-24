#pragma once
#include <iostream>


class Camera
{

public:   
	// ensure inherited destructor is called
	virtual ~Camera() {}
    // returns the view matrix
    virtual glm::mat4 GetViewMatrix() = 0;
    // processes input received from keyboard
    virtual void Move(glm::vec2 direction, float deltaTime) = 0;
    // processes input received from a mouse
    void ChangeDirection(glm::vec2 offset, bool constrainPitch = true);
    // processes input received from a mouse scroll
    void ProcessMouseScroll(float yoffset);
    // returns projection matric
	glm::mat4 GetProjectionMatrix();
	// getters / setters
	void SetPerspective(bool isPerspective) 					{ m_Perspective = isPerspective; };
	void SetViewport(int x, int y, uint w, uint h) 				{ glViewport(x, y, w, h); m_Viewport = { x, y, (int)w, (int)h }; };
	glm::vec4 GetViewport() 									{ return m_Viewport; };
	void SetNearFar(float near, float far) 						{ m_Near = near; 		m_Far = far; };
	void SetMovementSpeed(float speed) 							{ m_MovementSpeed = speed; };
	void SetMouseSensitivity(float sensitivity) 				{ m_MouseSensitivity = sensitivity; };	
	void SetInvert(bool invertPitch, bool invertYaw) 			{ m_InvertPitch = invertPitch; m_InvertYaw = invertYaw; };
	void SetZoomMinMax(float min, float max)             		{ m_ZoomMin = min; m_ZoomMax = max; };		
	void SetZoom(float zoom)              		                { m_Zoom = zoom; updateCameraVectors(); };		
    float GetZoom()              		                        { return m_Zoom; };		
    float Is2DMode()                                            { return m_2DMode; }
    // returns focal point
	glm::vec3 GetPosition() { return m_Position; };
	glm::vec3 GetCentre() { return m_Centre; };
	void SetCentre(glm::vec3 centre) { m_Centre = centre; updateCameraVectors(); };
	// returns true if px is withing bounds of viewport
	bool IsInsideViewport(glm::vec2 px);
	// returns position in px from world coords
	// first value of pair states whether inside viewport
	// Note: y0 in opengl is bottom left of screen
	std::pair<bool, glm::vec2> GetScreenCoords(glm::vec3 coords);
	// returns worlds coords from screen px
	glm::vec3 GetWorldPosition(glm::vec2 px);
	// aligns a vector to rotation of the z axis (used for mouse dragging)
	glm::vec2 GetWorldVector(glm::vec2 mouseMove);
			
			
	void DrawImGui() 
	{
		ImGui::SliderFloat3("Position", &(m_Position.x), -100.0f, 100.0f);
		ImGui::SliderFloat3("Centre", &(m_Centre.x), -50.0f, 50.0f);
		
		ImGui::SliderFloat3("LookAt", &(m_Front.x), -50.0f, 50.0f);
		ImGui::SliderFloat3("WorldUp", &(m_WorldUp.x), -500.0f, 500.0f);
		ImGui::SliderFloat3("Up", &(m_Up.x), -1.0f, 1.0f);
		ImGui::SliderFloat3("Right", &(m_Right.x), -500.0f, 500.0f);
		
		ImGui::SliderFloat("Yaw", &m_Yaw, -500.0f, 500.0f);
		ImGui::SliderFloat("Pitch", &m_Pitch, -500.0f, 500.0f);
		ImGui::SliderFloat("FOV", &m_FOV, -500.0f, 500.0f);
		ImGui::SliderFloat("MovementSpeed", &m_MovementSpeed, -500.0f, 500.0f);
		ImGui::SliderFloat("MouseSensitivity", &m_MouseSensitivity, -500.0f, 500.0f);
		ImGui::Text("Zoom: %g", m_Zoom);
	}
	
protected:
	
	Camera() {}
	
	glm::mat4 m_Proj; // for caching the result
	glm::mat4 m_View;
	
	bool m_Perspective = true; // isometric if false
	
	glm::vec4 m_Viewport;
	float m_Near 				= 0.1f;
	float m_Far 				= 1000.0f;
	    
    // camera position
    glm::vec3 m_WorldUp;	// worlds up
    glm::vec3 m_Position;	// cameras position
    glm::vec3 m_Front; 		// direction we are looking at
    glm::vec3 m_Centre;		// point we are looking at
    glm::vec3 m_Up;			// up vector
    glm::vec3 m_Right;		// right vector
    
    // euler Angles
    float m_YawDefault;
    float m_PitchDefault;
    float m_Yaw;
    float m_Pitch;
    bool m_InvertYaw = true;
    bool m_InvertPitch = false;
    
    bool m_2DMode = false;
    
    // camera options
    float m_FOV 				=  45.0f;
    float m_MouseSensitivity 	=  0.1f;
    float m_MovementSpeed;
	
	// To make zoom applicable for first person, modify Camera_FirstPerson::GetViewMatrix()
	// zoom should likely be inverted also
    float m_Zoom  				=  1.0f;
    float m_ZoomMin  			=  1.0f;
    float m_ZoomMax  			=  100.0f;
    // updates vectors
    virtual void updateCameraVectors() = 0;
	
};

class Camera_CentreObject : public Camera
{
public:
    Camera_CentreObject(int screenWidth, int screenHeight, glm::vec3 centre = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = 45.0f, float pitch = 45.0f);
    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() override;
    // set the camera to be top down
    void Set2DMode(bool isTrue);
    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void Move(glm::vec2 direction, float deltaTime = 0.1f) override;
    
private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors() override;
};



class Camera_FirstPerson : public Camera
{
public:
    
    Camera_FirstPerson(int screenWidth, int screenHeight, glm::vec3 position = glm::vec3(0.0f, 0.0f, 10.0f));
    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() override;

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void Move(glm::vec2 direction, float deltaTime = 0.1f) override;	
	
private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors() override;	
};


