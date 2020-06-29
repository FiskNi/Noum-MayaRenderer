#include "Includes/Pch.h"
#include "CameraMap.h"

CameraMap* CameraMap::m_cameraMapInstance = 0;

CameraMap::CameraMap() {}

void CameraMap::Init()
{
	if (m_cameraMapInstance == 0)
		m_cameraMapInstance = new CameraMap();
}

CameraMap* CameraMap::GetInstance()
{
	if (m_cameraMapInstance == 0)
		m_cameraMapInstance = new CameraMap();
	return m_cameraMapInstance;
}

void CameraMap::CleanUp()
{
	std::map<std::string, Camera*>::iterator it;

	for (it = m_cameraMap.begin(); it != m_cameraMap.end(); it++)
	{
		it->second->Clean();
		delete it->second;
	}

	m_cameraMap.clear();
}

bool CameraMap::ExistsWithName(std::string name)
{
	if (m_cameraMap.find(name) != m_cameraMap.end())
		return true;
	return false;
}

Camera* CameraMap::GetCamera(std::string name)
{
	return m_cameraMap[name];
}

std::map<std::string, Camera*>& CameraMap::GetMap()
{
	return m_cameraMap;
}

Camera* CameraMap::CreateCamera(std::string name, Camera material)
{
	if (ExistsWithName(name))
		return m_cameraMap[name];

	Camera* newCamera = new Camera ();
	*newCamera = material;
	m_cameraMap[name] = newCamera;
	return newCamera;
}

Camera* CameraMap::CreateCamera(Camera material)
{
	if (ExistsWithName(material.name))
		return m_cameraMap[material.name];

	Camera* newCamera = new Camera();
	*newCamera = material;
	m_cameraMap[material.name] = newCamera;
	return newCamera;
}

void CameraMap::RemoveCamera(std::string name)
{
	delete m_cameraMap[name];
	m_cameraMap.erase(name);
}

void CameraMap::Destroy()
{
	CleanUp();
	delete m_cameraMapInstance;
	m_cameraMapInstance = nullptr;
}
