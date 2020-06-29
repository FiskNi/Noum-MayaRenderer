#ifndef _MATERIALMAP_H
#define _MATERIALMAP_H
#include <Includes/Pch.h>

class MaterialMap
{

public:
	static void Init();
	static MaterialMap* GetInstance();
	void CleanUp();
	bool ExistsWithName(std::string name);
	Material* GetMaterial(std::string name);
	Material* CreateMaterial(std::string name, Material mesh);
	Material* CreateMaterial(Material mesh);
	void RemoveMesh(std::string name);
	void Destroy();

private:
	MaterialMap();
	static MaterialMap* m_materialMapInstance;
	std::map<std::string, Material*> m_materialMap;

};


#endif


