#include "Includes/Pch.h"
#include "MaterialMap.h"

MaterialMap* MaterialMap::m_materialMapInstance = 0;

MaterialMap::MaterialMap() {}

void MaterialMap::Init()
{
	if (m_materialMapInstance == 0)
		m_materialMapInstance = new MaterialMap();
}

MaterialMap* MaterialMap::GetInstance()
{
	if (m_materialMapInstance == 0)
		m_materialMapInstance = new MaterialMap();
	return m_materialMapInstance;
}

void MaterialMap::CleanUp()
{
	std::map<std::string, Material*>::iterator it;

	for (it = m_materialMap.begin(); it != m_materialMap.end(); it++)
	{
		it->second->Clean();
		delete it->second;
	}

	m_materialMap.clear();
}

bool MaterialMap::ExistsWithName(std::string name)
{
	if (m_materialMap.find(name) != m_materialMap.end())
		return true;
	return false;
}

Material* MaterialMap::GetMaterial(std::string name)
{
	return m_materialMap[name];
}

Material* MaterialMap::CreateMaterial(std::string name, Material material)
{
	if (ExistsWithName(name))
		return m_materialMap[name];

	Material* newMaterial = new Material ();
	*newMaterial = material;
	m_materialMap[name] = newMaterial;
	return newMaterial;
}

Material* MaterialMap::CreateMaterial(Material material)
{
	if (ExistsWithName(material.name))
		return m_materialMap[material.name];

	Material* newMaterial = new Material();
	*newMaterial = material;
	m_materialMap[material.name] = newMaterial;
	return newMaterial;
}

void MaterialMap::RemoveMesh(std::string name)
{
	delete m_materialMap[name];
	m_materialMap.erase(name);
}

void MaterialMap::Destroy()
{
	CleanUp();
	delete m_materialMapInstance;
	m_materialMapInstance = nullptr;
}
