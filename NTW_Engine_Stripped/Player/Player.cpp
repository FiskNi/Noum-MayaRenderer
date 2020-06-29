 #include "Pch/Pch.h"
#include "Player.h"

Player::Player(std::string name, glm::vec3 playerPosition, Camera *camera)
{
	// References
	m_playerCamera = camera;

	// Often moving values 
	m_playerPosition = playerPosition;
	m_name = name;
	m_directionVector = glm::vec3(0, 0, 0);
	m_moveDir = glm::vec3(0.0f);
	m_moveSpeed = 20.0f;
}

Player::~Player()
{
	
}

void Player::Update(float deltaTime)
{
	if (m_playerCamera->isCameraActive()) 
	{																		// IMPORTANT; DOING THESE WRONG WILL CAUSE INPUT LAG
		move(deltaTime);
		m_playerCamera->Update();											// Update this first so that subsequent uses are synced
		m_directionVector = glm::normalize(m_playerCamera->getCamFace());	// Update this first so that subsequent uses are synced
	}
}

void Player::move(float deltaTime)
{
	m_moveDir = glm::vec3(0.0f);
	m_oldMoveDir = glm::vec3(0.0f);

	glm::vec3 lookDirection = m_directionVector;
	lookDirection.y = 0.0f;
	glm::vec3 lookRightVector = m_playerCamera->getCamRight();

	// Movement
	if (Input::isKeyHeldDown(GLFW_KEY_A))
		m_moveDir -= lookRightVector;
	if (Input::isKeyHeldDown(GLFW_KEY_D))
		m_moveDir += lookRightVector;
	if (Input::isKeyHeldDown(GLFW_KEY_W))
		m_moveDir += lookDirection;
	if (Input::isKeyHeldDown(GLFW_KEY_S))
		m_moveDir -= lookDirection;
	if (Input::isKeyHeldDown(GLFW_KEY_SPACE))
		m_moveDir.y += 1.0f;
	if (Input::isKeyHeldDown(GLFW_KEY_C) || Input::isKeyHeldDown(GLFW_KEY_LEFT_CONTROL))
		m_moveDir.y -= 1.0f;

	// Make sure moving is a constant speed
	if (glm::length(m_moveDir) >= 0.0001f)
		m_moveDir = glm::normalize(m_moveDir);

	m_playerPosition += m_moveDir * m_moveSpeed * deltaTime;

	// Set cameraPos
	m_cameraPosition = m_playerPosition;

	m_playerCamera->setCameraPos(m_cameraPosition);
}

void Player::setPlayerPos(glm::vec3 pos)
{
	m_playerPosition = pos;
}

const glm::vec3& Player::getPlayerPos() const
{
	return m_playerPosition;
}

Camera* Player::getCamera()
{
	return m_playerCamera;
}

const std::string& Player::getName() const
{
	return m_name;
}
