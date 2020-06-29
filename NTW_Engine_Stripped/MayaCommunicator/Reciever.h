#ifndef _RECIEVER_H
#define _RECIEVER_H
#include <Pch/Pch.h>
#include "ComData.h"
#include "MayaCom.h"

struct IncomingPacket
{
	ComData::DataType type;
	bool headerRead = false;


	bool ready = true;

	void Clear()
	{
		type = ComData::DataType::NONE;
		headerRead = false;
		ready = true;
	}
};

struct IncomingMesh
{
	// If this is true the mesh can be sent to the rendering pipeline
	bool meshReady = false;

	// This should be taken over by the MeshMap
	// Memory needs to be cleaned if it's not sent there
	Mesh *mesh = nullptr;
	bool meshRead = false;

	std::string material;

	int vertexCount = 0;
	int currVertReadOffset = 0;
	std::vector<Vertex> vertices;
	std::vector<bool> vertexRead;
	bool verticesRead = false;

	int faceCount = 0;
	int currFaceReadOffset = 0;
	std::vector<Face> faces;
	std::vector<bool> faceRead;
	bool facesRead = false;

	void Clear()
	{
		meshReady = false;

		mesh = nullptr;
		meshRead = false;

		// Clear vertices
		vertexCount = 0;
		currVertReadOffset = 0;
		vertices.clear();
		vertexRead.clear();
		verticesRead = false;

		// Clear faces
		faceCount = 0;
		currFaceReadOffset = 0;
		faces.clear();
		faceRead.clear();
		facesRead = false;
	}

	void PrepVertices(int count)
	{
		vertexCount = count;
		vertices.resize(count);
		vertexRead.resize(count);
	}
	void PrepFaces(int count)
	{
		faceCount = count;
		faces.resize(count);
		faceRead.resize(count);
	}

};


struct IncomingTransform
{
	bool transformReady = false;

	std::string name;
	Transform transform;
	glm::mat4 modelMatrix;

	void Clear()
	{
		transformReady = false;
		name.clear();
	}
};

struct IncomingMaterial
{
	bool ready = false;
	Material material;

	void Clear()
	{
		ready = false;
	}
};

struct IncomingMatChange
{
	bool ready = false;
	std::string mesh;
	std::string material;

	void Clear()
	{
		ready = false;
	}
};

struct IncomingCamera
{
	bool ready = false;
	std::string name;

	bool perspective = true;
	bool transform = false;
	glm::vec3 pos;
	float yaw;		// rotation y
	float pitch;	// rotation x
	float width;	// Orto width


	void Clear()
	{
		name = "";
		perspective = true;
		transform = false;
		ready = false;
	}
};

using namespace MayaComLib;
class Reciever
{
public:
	static void Init();
	static Reciever* GetInstance();
	void Recieve();
	bool GetMesh(Mesh*& mesh, std::string& material);
	bool UpdateTransform(std::string& name, Transform& transform);
	bool UpdateMaterial();

	bool UpdateCamera(std::vector<Camera*>& cameras, std::map<std::string, Camera*>& cameraMap);

	bool ChangeMat(std::string& mesh, std::string& mat);


	void ClearPackets();
	void Destroy();

private:
	Reciever();
	static Reciever* m_senderInstance;

	MayaCom* mayaCom;

	// Data sizes
	size_t s_headerSize;
	size_t s_meshSize;
	size_t s_vertexSize;
	size_t s_faceSize;
	size_t s_transformSize;
	size_t s_materialSize;
	size_t s_matChangedSize;
	size_t s_cameraSize;

	IncomingPacket m_incomingPacket;
	IncomingMesh m_incomingMesh;
	IncomingTransform m_incomingTransform;
	IncomingMaterial m_incomingMaterial;
	IncomingMatChange m_incomingMatChange;
	IncomingCamera m_incomingCamera;


};

#endif

