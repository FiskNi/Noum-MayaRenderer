#ifndef _COMDATA_H
#define _COMDATA_H

#define NAME_SIZE_L 256

namespace ComData
{
	enum class DataType : int
	{
		NONE,
		MESH,
		VERTEX,
		FACE,
		MESHREMOVED,
		MESHRENAMED,
		MESHTRANSFORMED,
		MATERIALCHANGED,

		MATERIAL,
		MATERIALTRANSFORMED,
		
		CAMERA,
		CAMERATRANSFORMED

	};

	enum class CameraType : int
	{
		PERSP,
		ORTHO,
		TRANSFORMPERPS,
		TRANSFORMORTHO
	};
	
	struct Header
	{
		DataType dataType = DataType::NONE;
	};

	struct Camera
	{
		char name[NAME_SIZE_L];
		CameraType type = CameraType::PERSP;

		float pos[3];	
		float yaw;		// rotation y
		float pitch;	// rotation x

		float width;	// Orto width
	};


	struct TransformChange
	{
		char name[NAME_SIZE_L];
		float translation[3];
		float rotation[4];
		float scale[3];

		float matrix[16];
	};

	struct Material
	{
		char name[NAME_SIZE_L];
		float ambient[3];
		float diffuse[3];
		float specular[3];
		float emissive[3];
		float opacity;

		char albedo[NAME_SIZE_L];
		char normal[NAME_SIZE_L];
	};

	struct MeshMatChange
	{
		char meshName[NAME_SIZE_L];
		char materialName[NAME_SIZE_L];
		char albedo[NAME_SIZE_L];
	};

	struct Vertex
	{
		float position[3];
		float uv[2];
		float normal[3];
		//float tangent[3];
		//float bitangent[3];
	};

	struct Face
	{
		int indices[3];
	};

	struct Mesh
	{
		char name[NAME_SIZE_L];
		char material[NAME_SIZE_L];
		float matrix[4][4];

		int vertexCount = 0;
		int faceCount = 0;

		// Need to fix
		float translation[3];
		float rotation[4];
		float scale[3];
	};


	struct DirLight
	{
		float position[3];
		float rotation[3];
		float color[3];
		float intensity;
	};

	struct PointLight
	{
		float position[3];
		float color[3];
		float intensity;
	};



};








#endif