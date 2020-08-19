#include <Pch/Pch.h>
#include <Loader/BGLoader.h>
#include "GameObject.h"

GameObject::GameObject()
{
	m_objectName = "Empty";
	m_type = 0;
	m_shouldRender = true;
}

GameObject::GameObject(std::string objectName)
{
	m_objectName = objectName;
	m_type = 0;
	m_shouldRender = true;
}

GameObject::~GameObject()
{

}

void GameObject::loadMesh(std::string fileName)
{
	BGLoader tempLoader;	// The file loader
	tempLoader.LoadMesh(MESHPATH + fileName);

	for (int i = 0; i < tempLoader.GetMeshCount(); i++)
	{
		// Get mesh
		MeshBox tempMeshBox;									// Meshbox holds the mesh identity and local transform to GameObject
		std::string meshName = tempLoader.GetMeshName(i);
		tempMeshBox.transform = tempLoader.GetTransform(i);		// One way of getting the meshes transform

		if (!MeshMap::getInstance()->existsWithName(meshName))	// This creates the mesh if it does not exist (by name)
		{
			Mesh tempMesh;
			tempMesh.saveFilePath(tempLoader.GetFileName(), i);
			tempMesh.nameMesh(tempLoader.GetMeshName(i));
			if (tempLoader.GetSkeleton(i).name != "")
			{
				// Mesh with skeleton requires extra vertex data
				tempMesh.setUpMesh(tempLoader.GetSkeleVertices(i), tempLoader.GetFaces(i));
				tempMesh.setUpSkeleBuffers();

				// Get skeleton
				Skeleton tempSkeleton = tempLoader.GetSkeleton(i);
				std::string skeletonName = tempSkeleton.name + "_" + m_objectName;
				if (skeletonName != "" && !SkeletonMap::getInstance()->existsWithName(skeletonName))
				{
					SkeletonMap::getInstance()->createSkeleton(skeletonName, tempSkeleton);
					logTrace("Skeleton created: {0}", skeletonName);
				}
				tempMesh.setSkeleton(skeletonName);
			}
			else
			{
				// Default mesh
				tempMesh.setUpMesh(tempLoader.GetVertices(i), tempLoader.GetFaces(i));
				tempMesh.setUpBuffers();
			}
			tempLoader.GetLoaderVertices(0)[0].tangent;

			// other way of getting the meshes transform
			// Value that may or may not be needed depening on how we want the meshes default position to be
			// Needs more testing, this value is per global mesh, the MeshBox value is per GameObject mesh
			// tempMesh.setTransform(tempLoader.GetTransform(id));


			// Get animation
			for (size_t a = 0; a < tempLoader.GetAnimation(i).size(); a++)
			{
				Animation tempAnimation = tempLoader.GetAnimation(i)[a];
				std::string animationName = tempAnimation.name + "_" + m_objectName;
				if (animationName != "" && !AnimationMap::getInstance()->existsWithName(animationName))
				{
					AnimationMap::getInstance()->createAnimation(animationName, tempAnimation);
					logTrace("Animation created: {0}", animationName);
				}
				tempMesh.addAnimation(animationName);
			}

			
			tempMesh.setMaterial(tempLoader.GetMaterial(i).name);
			//Get the mesh pointer so that we don't have to always search through the MeshMap, when rendering
			tempMeshBox.mesh = MeshMap::getInstance()->createMesh(meshName, tempMesh); 
			logTrace("Mesh loaded: {0}, Expecting material: {1}", tempMesh.getName().c_str(), tempMesh.getMaterial());
		}
		else {
			tempMeshBox.mesh = MeshMap::getInstance()->getMesh(meshName);
		}

		// Get material
		Material tempMaterial = tempLoader.GetMaterial(i);
		std::string materialName = tempMaterial.name;
		if (!MaterialMap::getInstance()->existsWithName(materialName)) 	// This creates the material if it does not exist (by name)
		{
			if (tempLoader.GetAlbedo(i) != "-1")
			{
				std::string albedoFile = TEXTUREPATH + tempLoader.GetAlbedo(i);
				GLuint texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				// set the texture wrapping/filtering options (on the currently bound texture object)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				// load and generate the texture
				int width, height, nrChannels;
				unsigned char* data = stbi_load(albedoFile.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
				if (data)
				{
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
					glGenerateMipmap(GL_TEXTURE_2D);

					tempMaterial.texture = true;
					tempMaterial.textureID.push_back(texture);
				}
				else
				{
					std::cout << "Failed to load texture" << std::endl;
				}
				stbi_image_free(data);
			}
			else
			{
				tempMaterial.texture = false;
			}

			tempMaterial.normalMap = false;

			//Get the material pointer so that we don't have to always search through the MatMap, when rendering
			tempMeshBox.material = MaterialMap::getInstance()->createMaterial(materialName, tempMaterial);
 			logTrace("Material created: {0}", materialName);
		}
		else {
			tempMeshBox.material = MaterialMap::getInstance()->getMaterial(materialName);
		}

		m_meshes.push_back(tempMeshBox);						// This effectively adds the mesh to the gameobject
	}

	//Allocate all of the model matrixes
	m_modelMatrixes.resize(m_meshes.size());
	for (size_t i = 0; i < m_modelMatrixes.size(); i++)
	{
		m_modelMatrixes[i] = glm::mat4(1.0);
	}

	tempLoader.Unload();
	updateTransform();
}

void GameObject::initMesh(Mesh mesh)
{
	MeshBox tempMeshBox;											// Meshbox holds the mesh identity and local transform to GameObject
	m_meshes.push_back(tempMeshBox);								// This effectively adds the mesh to the gameobject
	if (!MeshMap::getInstance()->existsWithName(mesh.getName()))	// This creates the mesh if it does not exist (by name)
	{
		//Add mesh
		MeshMap::getInstance()->createMesh(mesh.getName(), mesh);
	}

	//Allocate all of the model matrixes
	m_modelMatrixes.resize(m_meshes.size());
	updateTransform();
}

void GameObject::initMesh(std::string name, std::vector<Vertex> vertices, std::vector<Face> faces)
{
	MeshBox tempMeshBox;									// Meshbox holds the mesh identity and local transform to GameObject
	if (!MeshMap::getInstance()->existsWithName(name))		// This creates the mesh if it does not exist (by name)
	{
		Mesh tempMesh;
		tempMesh.nameMesh(name);
		
		// Default mesh
		tempMesh.setUpMesh(vertices, faces);
		tempMesh.setUpBuffers();

		//Add mesh
		tempMeshBox.mesh = MeshMap::getInstance()->createMesh(name, tempMesh);

	}
	else {
		tempMeshBox.mesh = MeshMap::getInstance()->getMesh(name);
	}
	m_meshes.push_back(tempMeshBox);
	//Allocate all of the model matrixes
	m_modelMatrixes.resize(m_meshes.size());
	updateTransform();
}

const bool& GameObject::getShouldRender() const
{
	return m_shouldRender;
}

const glm::vec3 GameObject::getLastPosition() const
{
	return m_lastPosition;
}

//Update each individual modelmatrix for the meshes
void GameObject::updateTransform() {
	
	Transform transform;
	for (size_t i = 0; i < m_modelMatrixes.size(); i++)
	{
		m_modelMatrixes[i] = glm::mat4(1.0f);
		transform = getTransform(i);

		m_modelMatrixes[i] = glm::translate(m_modelMatrixes.at(i), transform.position);
		m_modelMatrixes[i] *= glm::mat4_cast(transform.rotation);
		m_modelMatrixes[i] = glm::scale(m_modelMatrixes[i], transform.scale);
	}

	m_lastPosition = m_transform.position;
}

void GameObject::setTransform(Transform transform)
{
	m_transform = transform;
	updateTransform();
}

void GameObject::setTransform(glm::vec3 worldPosition, glm::quat worldRot, glm::vec3 worldScale)
{
	m_transform.position = worldPosition;
	m_transform.scale = worldScale;
	m_transform.rotation = worldRot;
	updateTransform();
}

void GameObject::setMeshOffsetTransform(Transform transform, int meshIndex)
{
	m_meshes[meshIndex].transform = transform;
	updateTransform();
}

void GameObject::setWorldPosition(glm::vec3 worldPosition)
{
	m_transform.position = worldPosition;
	updateTransform();
}

void GameObject::setWorldRotation(glm::quat worldRotation)
{
	m_transform.rotation = worldRotation;
	updateTransform();
}

void GameObject::setMeshOffsetPosition(glm::vec3 position, int meshIndex)
{
	m_meshes[meshIndex].transform.position = position;
	updateTransform();
}

void GameObject::setMeshOffsetRotation(glm::quat rotation, int meshIndex)
{
	m_meshes[meshIndex].transform.rotation = rotation;
	updateTransform();
}

void GameObject::setShouldRender(bool condition)
{
	m_shouldRender = condition;
}

void GameObject::setMaterial(Material* material, int meshIndex)
{
	if (meshIndex == -1)
	{
		// Special case to make all models use material of the first mesh
		for (int i = 0; i < (int)m_meshes.size(); i++)
		{
			if (m_meshes[i].mesh)
				m_meshes[i].material = m_meshes[0].material;
		}
	}
	else
	{
		if (m_meshes.size() >= meshIndex)
			m_meshes[meshIndex].material = material;
	}

}

Mesh* GameObject::getMesh(const int& meshIndex)
{
	return m_meshes[meshIndex].mesh;
}

Material* GameObject::getMaterial(const int& meshIndex)
{
	return m_meshes[meshIndex].material;
}

const Transform GameObject::getTransform(int meshIndex) const
{
	// Adds the inherited transforms together to get the world position of a mesh
	Transform world_transform;
	world_transform.position = m_transform.position + m_meshes[meshIndex].transform.position;
	world_transform.rotation = m_transform.rotation * m_meshes[meshIndex].transform.rotation;
	world_transform.scale = m_transform.scale * m_meshes[meshIndex].transform.scale;

	return world_transform;
}

const Transform GameObject::getObjectTransform() const
{
	return m_transform;
}

const Transform GameObject::getLocalTransform(int meshIndex) const
{
	return m_meshes[meshIndex].transform;
}

const glm::mat4& GameObject::getMatrix(const int& i) const
{
	if (m_modelMatrixes.size() == 0) {
		return glm::mat4(1.0f);
	}
	//if we are trying to access a matrix beyond our count
	if (i > static_cast<int>(m_modelMatrixes.size())) {
		return glm::mat4(1.0f);
	}
	return m_modelMatrixes[i];
}

void GameObject::bindMaterialToShader(std::string shaderName, int meshIndex)
{
	ShaderMap::getInstance()->getShader(shaderName)->setMaterial(m_meshes[meshIndex].material);
}

void GameObject::bindMaterialToShader(Shader* shader, const int& meshIndex)
{
	shader->setMaterial(m_meshes[meshIndex].material);
}


void GameObject::setNormalMap(const char* fileName)
{
	std::string normalFile = TEXTUREPATH + fileName;
	GLuint normalMap;
	glGenTextures(1, &normalMap);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(normalFile.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes.at(i).material->normalMap = true;
			m_meshes.at(i).material->normalMapID = normalMap;
		}		
	}
	else
	{
		std::cout << "Failed to load normal map texture" << std::endl;
	}
	stbi_image_free(data);
}

void GameObject::setTexture(const char* fileName)
{
	std::string albedoFile = TEXTUREPATH + fileName;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(albedoFile.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes.at(i).material->texture = true;
			m_meshes.at(i).material->textureID.push_back(texture);
		}
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);			
}
