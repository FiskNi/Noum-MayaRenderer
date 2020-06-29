#ifndef _SENDER_H
#define _SENDER_H
#include <Includes/Pch.h>
#include <ComData.h>
#include "MayaCom.h"

struct CameraTransformPacket
{
	ComData::Camera data;
	bool written = false;
};

struct CameraPacket
{
	ComData::Camera data;
	bool written = false;
};

struct MaterialPacket
{
	ComData::Material materialData;
	bool written = false;
};

struct MeshMatChangedPacket
{
	ComData::MeshMatChange data;
	bool written = false;
};

struct VertexPacket
{
	ComData::Vertex vertexData;
	bool written = false;
};

struct FacePacket
{
	ComData::Face faceData;
	bool written = false;
};

struct MeshPacket
{
	ComData::Mesh meshData;
	bool meshWritten = false;

	std::vector<VertexPacket> vertexPackets;
	bool verticesWritten = false;

	std::vector<FacePacket> facePackets;
	bool facesWritten = false;
};

struct TransformPacket
{
	ComData::TransformChange transformData;
	bool written = false;
};


struct Packet
{
	ComData::Header headData;
	bool headWritten = false;

	MeshPacket mesh;
	TransformPacket transform;
	MaterialPacket material;
	MeshMatChangedPacket meshmatChanged;
	CameraPacket camera;
	bool allWritten = false;
};

namespace MayaInfo
{
	struct Update
	{
		MFn::Type mayaType;
		ComData::DataType dataType;
		std::string name;

		Update(MFn::Type mt, ComData::DataType dt, std::string n)
		{
			mayaType = mt;
			dataType = dt;
			name = n;
		}
	};


}



using namespace MayaComLib;
const float reonnectTime = 1.0f;
class Sender
{
public:
	static void Init();
	static Sender* GetInstance();
	void Connect();
	void Process(float deltaTime);
	bool Done();
	void Submit();
	void Submit(Mesh* mesh);
	void SubmitMeshTransform(std::string name, Transform transform);
	void SubmitCamera(Camera camera, bool transform = false);
	void SubmitMaterial(Material material);
	void SubmitMatChange(std::string mesh, std::string material);
	void Send();
	void Send(ComData::Header* data);
	void Send(ComData::Mesh* data);

	void Destroy();


private:
	Sender();
	static Sender* m_senderInstance;

	MayaCom* m_mayaCom;
	float m_reconnectIn;

	bool m_connected;

	std::vector<Packet> m_packets;

	// Data sizes
	size_t s_headerSize;
	size_t s_meshSize;
	size_t s_vertexSize;
	size_t s_faceSize;
	size_t s_transformSize;
	size_t s_materialSize;
	size_t s_matChangedSize;
	size_t s_cameraSize;





};

#endif

