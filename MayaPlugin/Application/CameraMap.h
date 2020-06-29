#ifndef _CAMERAMAP_H
#define _CAMERAMAP_H
#include <Includes/Pch.h>

class CameraMap
{

public:
	static void Init();
	static CameraMap* GetInstance();
	void CleanUp();
	bool ExistsWithName(std::string name);
	Camera* GetCamera(std::string name);
	std::map<std::string, Camera*>& GetMap();
	Camera* CreateCamera(std::string name, Camera camera);
	Camera* CreateCamera(Camera mesh);
	void RemoveCamera(std::string name);
	void Destroy();

private:
	CameraMap();
	static CameraMap* m_cameraMapInstance;
	std::map<std::string, Camera*> m_cameraMap;

};


#endif


