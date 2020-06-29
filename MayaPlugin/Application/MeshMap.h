#ifndef _MESHMAP_H
#define _MESHMAP_H
#include <Includes/Pch.h>

class MeshMap
{

public:
	static void Init();
	static MeshMap* GetInstance();
	void CleanUp();
	bool ExistsWithName(std::string name);
	Mesh* GetMesh(std::string name);
	std::map<std::string, Mesh*>& GetMap();
	Mesh* CreateMesh(std::string name, Mesh mesh);
	Mesh* CreateMesh(Mesh mesh);
	void RemoveMesh(std::string name);
	void Destroy();

private:
	MeshMap();
	static MeshMap* m_meshMapInstance;
	std::map<std::string, Mesh*> m_meshMap;

};


#endif


