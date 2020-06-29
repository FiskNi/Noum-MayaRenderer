#pragma once
#include <Pch/Pch.h>

class Player
{

public:
	Player(std::string name, glm::vec3 playerPosition, Camera* camera);
	~Player();

	void Update(float deltaTime);

	//-----Get-----//
	Camera* getCamera();
	const glm::vec3& getPlayerPos() const;
	const std::string& getName() const;

	//-----Set-----//
	void setPlayerPos(glm::vec3 pos);


private:
	void move(float deltaTime); 	

	std::string m_name;
	glm::vec3 m_directionVector;
	glm::vec3 m_playerPosition;
	glm::vec3 m_cameraPosition;
	
	glm::vec3 m_moveDir;
	float m_moveSpeed;

	glm::vec3 m_oldMoveDir;
	Camera* m_playerCamera;

};