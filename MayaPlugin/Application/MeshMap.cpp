#include "Includes/Pch.h"
#include "MeshMap.h"

MeshMap* MeshMap::m_meshMapInstance = 0;

MeshMap::MeshMap() {}

void MeshMap::Init()
{
	if (m_meshMapInstance == 0)
		m_meshMapInstance = new MeshMap();
}

MeshMap* MeshMap::GetInstance()
{
	if (m_meshMapInstance == 0)
		m_meshMapInstance = new MeshMap();
	return m_meshMapInstance;
}

void MeshMap::CleanUp()
{
	std::map<std::string, Mesh*>::iterator it;

	for (it = m_meshMap.begin(); it != m_meshMap.end(); it++)
	{
		it->second->Clean();
		delete it->second;
	}

	m_meshMap.clear();
}

bool MeshMap::ExistsWithName(std::string name)
{
	if (m_meshMap.find(name) != m_meshMap.end()) 
		return true;
	return false;
}

Mesh* MeshMap::GetMesh(std::string name)
{
	return m_meshMap[name];
}

std::map<std::string, Mesh*>& MeshMap::GetMap()
{
	return m_meshMap;
}

Mesh* MeshMap::CreateMesh(std::string name, Mesh mesh)
{
	if (ExistsWithName(name))
		return nullptr;

	Mesh* newMesh = new Mesh();
	*newMesh = mesh;
	m_meshMap[name] = newMesh;
	return newMesh;
}

Mesh* MeshMap::CreateMesh(Mesh mesh)
{
	if (ExistsWithName(mesh.name))
		return nullptr;

	Mesh* newMesh = new Mesh();
	*newMesh = mesh;
	m_meshMap[mesh.name] = newMesh;
	return newMesh;
}

void MeshMap::RemoveMesh(std::string name)
{
	m_meshMap[name]->Clean();
	delete m_meshMap[name];
	m_meshMap.erase(name);
}

void MeshMap::Destroy()
{
	CleanUp();
	delete m_meshMapInstance;
	m_meshMapInstance = nullptr;
}
