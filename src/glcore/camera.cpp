#include <iostream>
#include <algorithm>
#include <utility>
 
#include "glcore.h"
#include "camera.h"
 
using namespace std;


// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ChangeDirection(glm::vec2 offset, bool constrainPitch)
{
    // ignore input in 2D mode, only used for CentreObject
    if(m_2DMode) { 
        return;
    }
	if(offset.x == 0.0f && offset.y == 0.0f) {
		return;
    }
	offset.x *= m_MouseSensitivity; 
	offset.y *= m_MouseSensitivity;
	m_Yaw   += (m_InvertYaw ? -1 : 1) * offset.x;
	
	// wrap Yaw between 0-360degs
	while(m_Yaw > 360.0f)
		m_Yaw -= 360.0f;
	while(m_Yaw < 0.0f)
		m_Yaw += 360.0f;
	
	m_Pitch += (m_InvertPitch ? -1 : 1) * offset.y;
	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
		m_Pitch = clamp(m_Pitch, -89.0f, 89.0f);
    
	// update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yoffset)
{ 
	if(yoffset == 0.0f)
		return;
    // sqrt allows us to zoom slower when closer and faster when 
    // farther away giving, this makes it feel more linear
    m_Zoom -= yoffset * sqrt(m_Zoom);
        
	m_Zoom = clamp(m_Zoom, m_ZoomMin, m_ZoomMax);
	updateCameraVectors(); 
} 
  
glm::mat4 Camera::GetProjectionMatrix() 
{
	float aspectRatio = (float)m_Viewport[2] / (float)m_Viewport[3];
	
	if(m_Perspective) {
		m_Proj = glm::perspective(glm::radians(m_FOV), aspectRatio, m_Near, m_Far);
	} 
	else { // orthographic
		float scaledWidth = m_Zoom * aspectRatio / 2.0f;
		float scaledHeight = m_Zoom * 1.0f / 2.0f;
		m_Proj = glm::ortho(-scaledWidth, scaledWidth, -scaledHeight, scaledHeight, -m_Far, m_Far);
	}
	return m_Proj;
}

bool Camera::IsInsideViewport(glm::vec2 px)
{
	float right = m_Viewport[0] + m_Viewport[2];
	float top = m_Viewport[1] + m_Viewport[3];
	return ((px.x > m_Viewport[0] && px.x < right) && (px.y > m_Viewport[1] && px.y < top));
}

pair<bool, glm::vec2> Camera::GetScreenCoords(glm::vec3 coords)
{
	if(!m_2DMode) { 
        return make_pair(false, glm::vec2());
    }
	// we are assuming no model tranform
	glm::mat4 model = glm::mat4(1.0f);

	glm::vec3 project = glm::project(coords, model * m_View, m_Proj, m_Viewport);
	glm::vec2 px(project.x, project.y);
	// if z is outside the bounds of 0-1, it is invalid and will display in strange places
	bool valid = (project.z > 0.0f && project.z < 1.0f);
	
	return make_pair(IsInsideViewport(px) && valid, px);
}

glm::vec3 Camera::GetWorldPosition(glm::vec2 px) 
{
	// we are assuming no model tranform
	glm::mat4 model = glm::mat4(1.0f);
	return glm::unProjectNO(glm::vec3(px, 0.0f), model * m_View, m_Proj, m_Viewport);
}
glm::vec2 Camera::GetWorldVector(glm::vec2 mouseMove)
{
	// invert y when below xy plane
	if(m_Position.z < 0)
		mouseMove.y = -mouseMove.y;
	// rotate by -yaw angle
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f - m_Yaw), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 output = rotationMatrix * glm::vec4(mouseMove, 0.0f, 1.0f);
	// scale to amount zoomed
	output *= m_Zoom;
	return { output.x, -output.y };
}
Camera_CentreObject::Camera_CentreObject(int screenWidth, int screenHeight, glm::vec3 centre, float yaw, float pitch)
{
	SetViewport(0, 0, screenWidth, screenHeight);
    m_YawDefault = yaw;
    m_PitchDefault = pitch;
    m_Yaw = yaw;
    m_Pitch = pitch;
	m_WorldUp = glm::vec3(0.0f, 0.0f, 1.0f);  
	m_Centre = centre;
	m_MovementSpeed = 0.01f;
	updateCameraVectors();
}
 
// returns the view matrix calculated using Euler Angles and the LookAt Matrix 
glm::mat4 Camera_CentreObject::GetViewMatrix()
{ 
	m_View = glm::lookAt(m_Position, m_Centre, m_Up);
	return m_View;
} 

void Camera_CentreObject::Set2DMode(bool isTrue)
{
    if(isTrue) {
        m_2DMode = true;
        m_Perspective = false;
        m_Yaw = 90.0f;
        m_Pitch = 90.0f;
    } else { // 3d mode
        m_2DMode = false;
        m_Perspective = true;
        m_Yaw = m_YawDefault;
        m_Pitch = m_PitchDefault;
    }
    updateCameraVectors();
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera_CentreObject::Move(glm::vec2 direction, float deltaTime)
{	
	float velocity = m_MovementSpeed * deltaTime;
    float invertFor2DMode = (m_2DMode) ? -1.0f : 1.0f;
	m_Centre += glm::vec3(direction, 0.0f) * velocity * invertFor2DMode;
	updateCameraVectors();
}

// calculates the front vector from the Camera's (updated) Euler Angles
void Camera_CentreObject::updateCameraVectors()
{ 
	// yaw goes around z
	glm::vec3 pos;
	pos.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	pos.y = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	pos.z = sin(glm::radians(m_Pitch)); 
	
	// Re-calculate the Front, Right and Up vector
	m_Front = glm::normalize(pos);
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_Up = glm::normalize(glm::cross(m_Right, m_Front));
	
	// Calculate the camera position
	m_Position = (m_Zoom * pos) + m_Centre;
}
 



Camera_FirstPerson::Camera_FirstPerson(int screenWidth, int screenHeight, glm::vec3 position)
{
	SetViewport(0, 0, screenWidth, screenHeight);
    m_YawDefault = -90.0f;
    m_PitchDefault = 0.0f;
    m_Yaw = -90.0f;
    m_Pitch = 0.0f;
	m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	m_Position = position;
	m_MovementSpeed = 2.5f;
	updateCameraVectors();
} 
// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera_FirstPerson::GetViewMatrix()
{
	m_View = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
	return m_View;
} 

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera_FirstPerson::Move(glm::vec2 direction, float deltaTime)
{	
	float velocity = m_MovementSpeed * deltaTime;
	
	if (direction.y > 0.0f)
		m_Position += velocity * m_Front;
	if (direction.y < 0.0f)
		m_Position -= velocity * m_Front;
	if (direction.x < 0.0f)
		m_Position -= velocity * m_Right;
	if (direction.x > 0.0f)
		m_Position += velocity * m_Right;
}
 
// calculates the front vector from the Camera's (updated) Euler Angles
void Camera_FirstPerson::updateCameraVectors()
{
	// calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	front.y = sin(glm::radians(m_Pitch));
	front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Front = glm::normalize(front);
	
	// also re-calculate the Right and Up vector
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
}
