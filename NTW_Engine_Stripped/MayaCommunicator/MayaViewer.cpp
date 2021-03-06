#include <Pch/Pch.h>
#include <System/StateManager.h>
#include "MayaViewer.h"

#define SHOW_MEMORY_INFO false

MayaViewer::MayaViewer()
{
	Shader* basicTempShader = ShaderMap::getInstance()->getShader(BASIC_FORWARD);
	basicTempShader->use();
	basicTempShader->setInt("albedoTexture", 0);
	basicTempShader->setInt("normalMap", 1);

	m_cameras.emplace_back(new Camera());
	m_cameraMap["Default_pcamera"] = m_cameras.back();

	m_cameras.emplace_back(new Camera(false));
	m_cameraMap["Default_ocamera"] = m_cameras.back();


	mu.printBoth("After physics and camera init:");

	MaterialMap::getInstance()->createMaterial("Default");

	Renderer* renderer = Renderer::getInstance();
	renderer->setupCamera(m_cameraMap["Default_pcamera"]);
	mu.printBoth("After renderer:");

	LoadMap(renderer);


	Reciever::Init();
}

MayaViewer::~MayaViewer()
{
	mu.printBoth("Before deleting playstate:");

	for (GameObject* object : m_objects)
		delete object;

	for (Camera* camera : m_cameras)
		delete camera;

	for (Pointlight* light : m_pointlights)
		if (light)
			delete light;

	m_pointlights.clear();
	m_objects.clear();


	//BulletPhysics::getInstance()->destroy();
	MeshMap::getInstance()->cleanUp();
	Reciever::GetInstance()->Destroy();

	mu.printBoth("Afer deleting playstate:");
}


void MayaViewer::LoadMap(Renderer* renderer)
{
	// Map objects
	//m_objects.push_back(new MapObject("Debug_Map"));
	//m_objects[m_objects.size() - 1]->loadMesh("Debug_Map.mesh");

	// Submit objects
	if (m_objects.size() > 0)
		renderer->submit(m_objects[m_objects.size() - 1], RENDER_TYPE::STATIC);
	
	// Lights
	LoadLights(renderer);
	mu.printBoth("After point lights:");
}

void MayaViewer::LoadLights(Renderer* renderer)
{
// Light attenuation chart
/*
	7		1.0,	0.7,		1.8
	13		1.0,	0.35,		0.44
	20		1.0,	0.22,		0.20
	32		1.0,	0.14,		0.07
	50		1.0,	0.09,		0.032
	65		1.0,	0.07,		0.017
	100		1.0,	0.045,		0.0075
	160		1.0,	0.027,		0.0028
	200		1.0,	0.022,		0.0019
	325		1.0,	0.014,		0.0007
	600		1.0,	0.007,		0.0002
	3250	1.0,	0.0014,		0.000007
*/

	// Light Middle
	m_pointlights.emplace_back(new Pointlight(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(1.0, 1.0, 1.0), 40.0f));
	m_pointlights.back()->setAttenuationAndRadius(glm::vec4(1.0f, 0.07f, 0.017f, 100.0f));

	// Light Right Back
	m_pointlights.emplace_back(new Pointlight(glm::vec3(50.0f, 20.0f, 50.0f), glm::vec3(1.0, 0.0, 1.0), 40.0f));
	m_pointlights.back()->setAttenuationAndRadius(glm::vec4(1.0f, 0.07f, 0.017f, 100.0f));

	// Light Right Forward
	m_pointlights.emplace_back(new Pointlight(glm::vec3(50.0f, 20.0f, -50.0f), glm::vec3(0.0, 1.0, 1.0), 40.0f));
	m_pointlights.back()->setAttenuationAndRadius(glm::vec4(1.0f, 0.07f, 0.017f, 100.0f));

	// Light Left Back
	m_pointlights.emplace_back(new Pointlight(glm::vec3(-50.0f, 20.0f, 50.0f), glm::vec3(0.0, 0.0, 1.0), 40.0f));
	m_pointlights.back()->setAttenuationAndRadius(glm::vec4(1.0f, 0.07f, 0.017f, 100.0f));

	// Light Left Forward
	m_pointlights.emplace_back(new Pointlight(glm::vec3(-50.0f, 20.0f, -50.0f), glm::vec3(0.0, 1.0, 1.0), 40.0f));
	m_pointlights.back()->setAttenuationAndRadius(glm::vec4(1.0f, 0.07f, 0.017f, 100.0f));
	
	// Submit lights
	for (Pointlight* p : m_pointlights)
	{
		renderer->submit(p, RENDER_TYPE::POINTLIGHT_SOURCE);
	}
}



static float memTimer = 0.0f;
static float fpsTimer = 0.0f;
void MayaViewer::Update(float dt)
{
	

	UpdateMaya();
	Update_isPlaying(dt);

#if SHOW_MEMORY_INFO
	memTimer += DeltaTime;
	if (memTimer >= 1.0f)
	{
		memTimer = 0.0f;
		mu.updateBoth();
		std::string memTex =
			"Ram: " + std::to_string(mu.getCurrentRamUsage()) +
			" | VRam: " + std::to_string(mu.getCurrentVramUsage()) +
			" | Highest Ram: " + std::to_string(mu.getHighestRamUsage()) +
			" | Highest VRam: " + std::to_string(mu.getHighestVramUsage());
		logInfo(memTex);
	}

	fpsTimer += DeltaTime;
	if (fpsTimer >= 1.0f)
	{
		fpsTimer = 0.0f;
		logInfo("fps: " + std::to_string(Framerate));
	}
#endif



}

void MayaViewer::UpdateMaya()
{
	Renderer* renderer = Renderer::getInstance();
	if (Input::isKeyPressed(GLFW_KEY_R))
	{
		renderer->setupCamera(m_cameraMap["Default_pcamera"]);
	}
	if (Input::isKeyPressed(GLFW_KEY_T))
	{
		renderer->setupCamera(m_cameraMap["Default_ocamera"]);
	}
	if (Input::isKeyPressed(GLFW_KEY_E))
	{
		renderer->setupCamera(m_cameraMap["Default_ocamera"]);
	}


			

	Reciever* reciver = Reciever::GetInstance();

	reciver->Recieve();

	// Update camera
	reciver->UpdateCamera(m_cameras, m_cameraMap);

	// Update material
	reciver->UpdateMaterial();


	std::string meshDest;
	std::string newMat;
	if (reciver->ChangeMat(meshDest, newMat))
	{
		Material* mat = MaterialMap::getInstance()->getMaterial(newMat);
		m_objectMap[meshDest]->setMaterial(mat);
	}


	// Get new mesh
	Mesh* newMesh = nullptr;
	std::string material;
	if (reciver->GetMesh(newMesh, material))
	{
		if (m_objectMap.find(newMesh->getName()) != m_objectMap.end())
		{
			Renderer::getInstance()->removeRenderObject(m_objectMap[newMesh->getName()], STATIC);
			static_cast<MayaObject*>(m_objectMap[newMesh->getName()])->Destroy();
			static_cast<MayaObject*>(m_objectMap[newMesh->getName()])->InitMesh(newMesh, material);
			Renderer::getInstance()->submit(m_objectMap[newMesh->getName()], STATIC);
		}
		else
		{
			m_objects.emplace_back(new MayaObject(newMesh->getName()));
			m_objectMap[newMesh->getName()] = m_objects.back();
			static_cast<MayaObject*>(m_objects.back())->InitMesh(newMesh, material);

		}
		Renderer::getInstance()->submit(m_objects.back(), STATIC);

	}


	// Update transform
	std::string name;
	Transform transform;
	if (reciver->UpdateTransform(name, transform))
	{
		m_objectMap[name]->setTransform(transform);
	}

}

void MayaViewer::Update_isPlaying(const float& dt)
{
	//BulletPhysics::getInstance()->Update(dt);

	// Update game objects
	for (GameObject* object : m_objects)
		object->Update(dt);

	// Update cameras (lazy, should only update used one)
	for (Camera* camera : m_cameras)
		camera->Update();
}

void MayaViewer::render()
{
	Renderer::getInstance()->render();
}