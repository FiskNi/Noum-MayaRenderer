#include <Pch/Pch.h>
#include <System/Input.h>
#include "Camera.h"

static float m_sensitivity;
static float m_distanceThirdPerson = 10.0f;
static float m_distanceModel = 100.f;

// Scrollwheel callback used below
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	m_distanceThirdPerson -= yoffset;

	if (m_distanceThirdPerson <= 2.0f)
		m_distanceThirdPerson = 2.0f;
	else if (m_distanceThirdPerson >= 35.0f)
		m_distanceThirdPerson = 35.0f;
}

Camera::Camera(bool perspective)
{
	m_camPos = glm::vec3(0.0f, 3.0f, 0.0f);
	m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	m_camYaw = -90.0f;
	m_camPitch = 0;

	m_camFace = glm::vec3(0.0f, 0.0f, -1.0f);
	m_sensitivity = 0.15f;

	InitSettings(perspective);

}	


void Camera::InitSettings(bool perspective)
{
	m_cameraMode = CameraMode::FreeCamera;

	// Screen and View settings
	m_width = SCREEN_WIDTH;
	m_height = SCREEN_HEIGHT;
	m_nearPlane = 0.1f;
	m_farPlane = 2000;

	m_cameraMoveSpeed = 20.0f;
	setWindowSize(m_width, m_height);
	
	if (!perspective)
	{
		m_projectionMatrix = glm::ortho(-m_width / 4, m_width / 4, -m_height / 4, m_height / 4, 0.0f, 1000.0f);
		m_camPos = glm::vec3(0.0f, 50.0f, 200.0f);
	}

	calcVectors();

	m_viewMatrix = glm::lookAt(m_camPos, m_camPos + m_camFace, m_camUp);

	m_fpEnabled = true;
	m_activeCamera = true;

	glfwSetScrollCallback(glfwGetCurrentContext(), scroll_callback);
	//glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}



Camera::~Camera()
{
}


void Camera::freeCameraMode()
{
	updateMouseMovement();
	
	glm::vec3 moveDir = glm::vec3(0.0f);
	// Move
	if (Input::isKeyHeldDown(GLFW_KEY_A))
		moveDir -= m_camRight;
	if (Input::isKeyHeldDown(GLFW_KEY_D))
		moveDir += m_camRight;
	if (Input::isKeyHeldDown(GLFW_KEY_W))
		moveDir += m_camFace;
	if (Input::isKeyHeldDown(GLFW_KEY_S))
		moveDir -= m_camFace;
	if (Input::isKeyHeldDown(GLFW_KEY_SPACE))
		moveDir.y += 1.0f;
	if (Input::isKeyHeldDown(GLFW_KEY_C) || Input::isKeyHeldDown(GLFW_KEY_LEFT_CONTROL))
		moveDir.y -= 1.0f;

	m_camPos += moveDir * DeltaTime * m_cameraMoveSpeed;
}

//void Camera::LE_orbitCamera()
//{
//	updateThirdPersonMouseMovement();
//	m_camPos.x = 0.0f + (m_distanceModel * cos(glm::radians(m_camYaw)) * cos(glm::radians(m_camPitch)));
//	m_camPos.y = 0.0f + (m_distanceModel * sin(glm::radians(m_camPitch)));
//	m_camPos.z = 0.0f + (m_distanceModel * sin(glm::radians(m_camYaw)) * cos(glm::radians(m_camPitch)));
//	lookAt(glm::vec3(0.0f));
//}


void Camera::resetMouseToMiddle()
{
	int wSizeX, wSizeY;
	glfwGetWindowSize(glfwGetCurrentContext(), &wSizeX, &wSizeY);
	m_lastX = static_cast<float>(wSizeX / 2);
	m_lastY = static_cast<float>(wSizeY / 2);
	glfwSetCursorPos(glfwGetCurrentContext(), m_lastX, m_lastY);
}

void Camera::calcVectors()
{
	float x = fmod(m_camPitch, 360.0f);
	if (x < 0.0f)
		x += 360.0f;


	if (x >= 90.0f && x <= 270.0f)
		m_worldUp = glm::vec3(0.0f, -1.0f, 0.0f);
	else
		m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);


	m_camFace.x = cos(glm::radians(m_camYaw)) * cos(glm::radians(m_camPitch));
	m_camFace.y = sin(glm::radians(m_camPitch));
	m_camFace.z = sin(glm::radians(m_camYaw)) * cos(glm::radians(m_camPitch));
	m_camFace = glm::normalize(m_camFace);

	m_camRight = glm::normalize(glm::cross(m_camFace, m_worldUp));
	m_camUp = glm::normalize(glm::cross(m_camRight, m_camFace));
}

void Camera::resetCamera()
{
	resetMouseToMiddle();
	m_camYaw = -90.0f;
	m_camPitch = 0;
	m_camFace = glm::vec3(0.0f, m_camPos.y, 0.0f);
	calcVectors();
}

void Camera::updateMouseMovement()
{
	if (Input::isKeyPressed(GLFW_KEY_P)) 
	{
		m_lock = true;
	}
	if (Input::isKeyPressed(GLFW_KEY_O)) 
	{
		m_lock = false;
	}
	if (m_lock) 
	{
		return;
	}
	else 
	{
		if (Input::isMouseHeldDown(GLFW_MOUSE_BUTTON_LEFT))
		{
			if (glfwGetInputMode(glfwGetCurrentContext(), GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
				glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			int wSizeX, wSizeY;
			glfwGetWindowSize(glfwGetCurrentContext(), &wSizeX, &wSizeY);
			m_lastX = static_cast<float>(wSizeX / 2);
			m_lastY = static_cast<float>(wSizeY / 2);

			glfwGetCursorPos(glfwGetCurrentContext(), &m_xpos, &m_ypos);
			glfwSetCursorPos(glfwGetCurrentContext(), m_lastX, m_lastY);

			if (this->m_firstMouse == true)
			{
				this->m_firstMouse = false;
				m_xpos = m_lastX;
				m_ypos = m_lastY;
			}

			float xoffset = static_cast<float>(m_xpos) - m_lastX;
			float yoffset = m_lastY - static_cast<float>(m_ypos);

			m_lastX = static_cast<float>(m_xpos);
			m_lastY = static_cast<float>(m_ypos);

			mouseControls(xoffset, yoffset, true);

			m_viewMatrix = glm::lookAt(m_camPos, m_camPos + m_camFace, m_camUp);
		}
		else
		{
			if (glfwGetInputMode(glfwGetCurrentContext(), GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
				glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void Camera::updateThirdPersonMouseMovement()
{
	static glm::dvec2 initialPos = glm::dvec2(0.0);
	
	if (Input::isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) 
	{
		glfwGetCursorPos(glfwGetCurrentContext(), &initialPos.x, &initialPos.y);
	}

	if (Input::isMouseHeldDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		glm::dvec2 currentMouse = glm::dvec2(0.0);
		glfwGetCursorPos(glfwGetCurrentContext(), &currentMouse.x, &currentMouse.y);

		float xoffset = static_cast<float>(currentMouse.x) - initialPos.x;
		float yoffset = initialPos.y - static_cast<float>(currentMouse.y);

		xoffset *= m_sensitivity;
		yoffset *= m_sensitivity;

		m_camYaw += xoffset;
		m_camPitch -= yoffset;

		if (m_camPitch > 89.0f)
			m_camPitch = 89.0f;
		if (m_camPitch < -89.0f)
			m_camPitch = -89.0f;

		initialPos = currentMouse;
	}

}

void Camera::setWindowSize(float width, float height)
{
	this->m_width = width;
	this->m_height = height;
	
	setProjMat(this->m_width, this->m_height, this->m_nearPlane, this->m_farPlane);
}

void Camera::mouseControls(float xOffset, float yOffset, bool pitchLimit)
{
	float sens = ((float)50 * 0.01 * 2) + 0.02f;

	xOffset *= m_sensitivity * sens;
	yOffset *= m_sensitivity * sens;
	
	m_camYaw += xOffset;
	m_camPitch += yOffset;

	if (pitchLimit)
	{
		if (m_camPitch > 89.0f)
			m_camPitch = 89.0f;
		if (m_camPitch < -89.0f)
			m_camPitch = -89.0f;
	}

	calcVectors();
}

void Camera::setProjMat(float widht, float height, float nearPlane, float farPlane)
{
	float fov = 60;

	m_projectionMatrix = glm::perspective(glm::radians(fov), widht / height, nearPlane, farPlane);
}

const glm::mat4 Camera::getViewMat() const
{
	return m_viewMatrix;
}

const glm::mat4& Camera::getProjMat() const
{
	return m_projectionMatrix;
}

const double& Camera::getXpos() const
{
	return m_xpos;
}

const double& Camera::getYpos() const
{
	return m_ypos;
}

const float& Camera::getPitch() const
{
	return m_camPitch;
}

const float& Camera::getYaw() const
{
	return m_camYaw;
}

const glm::vec3& Camera::getCamPos() const
{
	return m_camPos;
}

const bool& Camera::isCameraActive() const
{
	return m_activeCamera;
}

const glm::vec3& Camera::getCamFace()
{
	return m_camFace;
}


const glm::vec3& Camera::getCamUp()
{
	return m_camUp;
}

void Camera::setCameraPos(const glm::vec3& pos)
{
	m_camPos = pos;
}

void Camera::setCameraYaw(const float yaw)
{
	m_camYaw = yaw;
}

void Camera::setCameraPitch(const float pitch)
{
	m_camPitch = pitch;
}

void Camera::setOrthoWidth(const float width)
{
	m_orthoWidth = width / 2;

	float h = m_orthoWidth * (m_height / m_width);
	m_projectionMatrix = glm::ortho(-m_orthoWidth, m_orthoWidth, -h, h, 0.0f, 2000.0f);
}

const glm::vec3& Camera::getCamRight()
{
	return m_camRight;
}

void Camera::lookAt(const glm::vec3& position, bool perspective)
{
	m_camFace = glm::normalize(position - m_camPos);
	m_camRight = glm::normalize(glm::cross(m_camFace, m_worldUp));
	m_camUp = glm::normalize(glm::cross(m_camRight, m_camFace));


	m_viewMatrix = glm::lookAt(m_camPos, m_camPos + m_camFace, m_camUp);
}

void Camera::Update()
{
	if (m_fpEnabled && m_activeCamera) 
	{
		freeCameraMode();
	}


	calcVectors();
	m_viewMatrix = glm::lookAt(m_camPos, m_camPos + m_camFace, m_camUp);
}

void Camera::disableCameraMovement(const bool condition)
{
	m_activeCamera = !condition;
}
