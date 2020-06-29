#pragma once
#ifndef _OBJECTFORMATS_H
#define _OBJECTFORMATS_H


struct Transform
{
	// Maya
	MFnTransform* transformFn = nullptr;
	MFnDagNode* dagPath = nullptr;
	MDagPath thisDagNode;

	MObject transformNode;
	MCallbackIdArray callbackIdArray;

	// Com
	std::string name;
	bool updated = false;



	void Clean()
	{
		if (transformFn)
			delete transformFn;
		MMessage::removeCallbacks(callbackIdArray);
	}

};

struct Camera
{
	enum class Type
	{
		PERSP,
		ORTHO
	};

	bool updated = false;

	MFnCamera* cameraFn;
	MCallbackIdArray callbackIdArray;

	Transform transform;

	// Com
	std::string name;
	Type type;

	void Clean()
	{
		transform.Clean();
		MMessage::removeCallbacks(callbackIdArray);
	}
};


struct Material
{
	enum class Type
	{
		LAMBERT,
		PHONG,
		BLINN
	};

	MFnLambertShader* lambertFn;
	MFnPhongShader* phongFn;
	MFnBlinnShader*  blinnFn;

	MCallbackIdArray callbackIdArray;

	// Com
	std::string name;
	Type type;


	void Clean()
	{
		/*if (lambertFn)
			delete lambertFn;
		if (phongFn)
			delete phongFn;
		if (blinnFn)
			delete blinnFn;*/

		MMessage::removeCallbacks(callbackIdArray);
	}
};


struct Vertex
{
	int index;
	bool updated;

	Vertex()
	{
		index = 0;
		updated = false;
	}
};

struct Mesh
{
	Transform transform;
	
	// Maya
	MFnMesh *meshFn = nullptr;
	MFnDagNode *dagPath = nullptr;
	MDagPath thisDagNode;
	
	MObject meshNode;
	MCallbackIdArray callbackIdArray;

	// Com
	std::string name;
	bool updated = false;
	std::vector<Vertex> vertices;


	void Clean()
	{
		transform.Clean();
		if (meshFn)
			delete meshFn;
		MMessage::removeCallbacks(callbackIdArray);
	}



	void NodeAboutToDelete(MObject& node, MDGModifier& modifier, void* clientData)
	{
		cout << "Node about to be removed: " << MFnDagNode(node).fullPathName() << endl;


		cout << name << endl;
	}
};


#endif