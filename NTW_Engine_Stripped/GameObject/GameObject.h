#ifndef	_GAMEOBJECT_h
#define _GAMEOBJECT_h
#include <Pch/Pch.h>
#include <Mesh/Mesh.h>
#include <GFX/MaterialMap.h>
#include <Mesh/Mesh.h>
//#include <System/BulletPhysics.h>
#include <Renderer/Camera.h>

class GameObject {
public:
	//Create an Empty object
	GameObject();
	//Create an Empty object with a different name
	GameObject(std::string objectName);
	virtual ~GameObject();
	virtual void Update(float dt) = 0;
	
	//Loads all the meshes from the file into the GameObject
	void loadMesh(std::string fileName);
	void initMesh(Mesh mesh);
	void initMesh(std::string name, std::vector<Vertex> vertices, std::vector<Face> faces);
	void bindMaterialToShader(std::string shaderName, int meshIndex = 0);
	void bindMaterialToShader(Shader* shader, const int& meshIndex = 0);
	
	// Bullet
	
	void setNormalMap(const char* fileName);
	void setTexture(const char* fileName);
	   	
	//Set functions
	void setTransform(Transform transform);
	void setTransform(glm::vec3 worldPosition = glm::vec3(), glm::quat worldRot = glm::quat(), glm::vec3 worldScale = glm::vec3(1.0f));
	void setMeshOffsetTransform(Transform transform, int meshIndex = 0);
	void setWorldPosition(glm::vec3 worldPosition);
	void setWorldRotation(glm::quat worldRotation);
	void setMeshOffsetPosition(glm::vec3 position, int meshIndex = 0);
	void setMeshOffsetRotation(glm::quat rotation, int meshIndex = 0);

	void setShouldRender(bool condition);
	void setMaterial(Material* material, int meshIndex = 0);


	//Get functions
	Mesh* getMesh(const int& meshIndex = 0); //Get a mesh from the meshbox
	Material* getMaterial(const int& meshIndex = 0); //Get a material from the meshbox

	const Transform getTransform(int meshIndex = 0) const;
	const Transform getObjectTransform() const;
	const Transform getLocalTransform(int meshIndex = 0) const;
	const int getMeshesCount() const { return (int)m_meshes.size(); }
	const glm::mat4& getMatrix(const int& i = 0) const;
	const int getType() const { return m_type; }
	const bool& getShouldRender() const;
	const glm::vec3 getLastPosition() const;	



	void updateTransform();

private:

	struct MeshBox // Handles seperate transforms for same mesh
	{
		Mesh* mesh;
		Material* material;
		Transform transform;


		MeshBox()
		{
			mesh = nullptr;
			material = nullptr;
		}
	};

	std::string m_objectName;
	Transform m_transform;
	glm::vec3 m_lastPosition;

	// Allocation for perfomance
	Transform t_transform;

protected:
	std::vector<glm::mat4> m_modelMatrixes;
	std::vector<MeshBox> m_meshes;
	bool m_shouldRender;
	int m_type;	
};


#endif