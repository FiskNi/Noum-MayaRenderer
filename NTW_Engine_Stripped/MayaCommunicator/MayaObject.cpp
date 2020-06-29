#include "Pch/Pch.h"
#include "MayaObject.h"

MayaObject::MayaObject(std::string name) : GameObject(name)
{
}

MayaObject::~MayaObject()
{
	for (int i = 0; i < (int)m_meshes.size(); i++)
	{
		m_meshes[i].mesh->Destroy();
		delete m_meshes[i].mesh;
		m_meshes[i].mesh = nullptr;
	}
	m_meshes.clear();
}

void MayaObject::Destroy()
{
	for (int i = 0; i < (int)m_meshes.size(); i++)
	{
		m_meshes[i].mesh->Destroy();
		delete m_meshes[i].mesh;
		m_meshes[i].mesh = nullptr;
	}
	m_meshes.clear();
}


void MayaObject::Update(float dt)
{
}

void MayaObject::InitMesh(Mesh *mesh, std::string material)
{
	m_meshes.emplace_back();						
	m_meshes.back().material = MaterialMap::getInstance()->getMaterial(material);
	m_meshes.back().mesh = mesh;


	//Allocate all of the model matrixes
	m_modelMatrixes.resize(m_meshes.size());
	updateTransform();
}
