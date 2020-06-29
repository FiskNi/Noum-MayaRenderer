#include "Pch/Pch.h"
#include "Reciever.h"

Reciever* Reciever::m_senderInstance = 0;

Reciever::Reciever()
{
	mayaCom = new MayaCom();

	mayaCom->CreateFileMap();
	mayaCom->GetMap();

	s_headerSize = sizeof(ComData::Header);
	s_meshSize = sizeof(ComData::Mesh);
	s_vertexSize = sizeof(ComData::Vertex);
	s_faceSize = sizeof(ComData::Face);
	s_transformSize = sizeof(ComData::TransformChange);
	s_materialSize = sizeof(ComData::Material);
	s_matChangedSize = sizeof(ComData::MeshMatChange);
	s_cameraSize = sizeof(ComData::Camera);
}

void Reciever::Init()
{
	if (m_senderInstance == 0)
		m_senderInstance = new Reciever();
}

Reciever* Reciever::GetInstance()
{
	if (m_senderInstance == 0)
		m_senderInstance = new Reciever();
	return m_senderInstance;
}

void Reciever::Recieve()
{
	// Check if we can a new packet
	if (m_incomingPacket.ready)
	{
		// Check if we can read memory
		if (mayaCom->CanRead(s_headerSize))
		{
			// Read header
			ComData::Header header;
			mayaCom->Read((char*)&header, s_headerSize);

			m_incomingPacket.type = header.dataType;
			m_incomingPacket.headerRead = true;
			m_incomingPacket.ready = false;
		}
	}

	// If packet is not ready content is in the process of reading
	if (!m_incomingPacket.ready)
	{
		// Check if the main header has been read, if not error has occured
		if (m_incomingPacket.headerRead)
		{
			// Check type based on header
			switch (m_incomingPacket.type)
			{
			case ComData::DataType::MESH:
			{
				// Check if mesh header has been read
				if (!m_incomingMesh.meshRead)
				{
					// Check if we can read memory
					if (mayaCom->CanRead(s_meshSize))
					{
						// Read mesh
						ComData::Mesh meshData;
						mayaCom->Read((char*)&meshData, s_meshSize);

						m_incomingMesh.mesh = new Mesh();
						m_incomingMesh.mesh->nameMesh(meshData.name);
						m_incomingMesh.material = meshData.material;
						m_incomingMesh.PrepVertices(meshData.vertexCount);
						m_incomingMesh.PrepFaces(meshData.faceCount);

						m_incomingMesh.meshRead = true;
					}
				}

				if (m_incomingMesh.meshRead)
				{
					// Check for read vertices and read as many as possible, else try again next loop
					for (int i = 0; i < m_incomingMesh.vertexCount; i++)
					{
						if (!m_incomingMesh.vertexRead[i])
						{
							// Check if we can read again after each read vertex
							if (mayaCom->CanRead(s_vertexSize))
							{
								ComData::Vertex vertexData;
								mayaCom->Read((char*)&vertexData, s_vertexSize);

								for (int a = 0; a < 3; a++)
									m_incomingMesh.vertices[i].position[a] = vertexData.position[a];
								for (int a = 0; a < 2; a++)
									m_incomingMesh.vertices[i].UV[a] = vertexData.uv[a];
								for (int a = 0; a < 3; a++)
									m_incomingMesh.vertices[i].Normals[a] = vertexData.normal[a];							

								m_incomingMesh.vertexRead[i] = true;
								//m_incomingMesh.currVertReadOffset = i + 1;

								if (i == m_incomingMesh.vertexCount - 1)
									m_incomingMesh.verticesRead = true;

							}
							else
							{
								break;
							}
						}
						else
						{
							//break;
						}
					}


					if (m_incomingMesh.verticesRead)
					{
						for (int i = 0; i < m_incomingMesh.faceCount; i++)
						{
							if (!m_incomingMesh.faceRead[i])
							{
								// Check if we can read again after each read face
								if (mayaCom->CanRead(s_faceSize))
								{
									ComData::Face faceData;
									mayaCom->Read((char*)&faceData, s_faceSize);

									for (int a = 0; a < 3; a++)
										m_incomingMesh.faces[i].indices[a] = faceData.indices[a];

									m_incomingMesh.faceRead[i] = true;
									//m_incomingMesh.currFaceReadOffset = i + 1;

									if (i == m_incomingMesh.faceCount - 1)
										m_incomingMesh.facesRead = true;

								}
								else
								{
									break;
								}
							}
							else
							{
								//break;
							}
						}
					}

				}


				// Declare packet has been read
				if (m_incomingMesh.facesRead)
				{
					m_incomingMesh.mesh->setUpMesh(m_incomingMesh.vertices, m_incomingMesh.faces);
					m_incomingMesh.mesh->setUpBuffers();
					m_incomingMesh.meshReady = true;
						
				}

				// Case end
				break;
			}

			case ComData::DataType::MESHTRANSFORMED:
			{
				
				// Check if transform has been parsed and is ready
				if (!m_incomingTransform.transformReady)
				{
					if (mayaCom->CanRead(s_transformSize))
					{
						// Read mesh
						ComData::TransformChange transformData;
						mayaCom->Read((char*)&transformData, s_transformSize);

						m_incomingTransform.name = transformData.name;
						m_incomingTransform.modelMatrix = glm::make_mat4x4(transformData.matrix);
						glm::vec3 empty1;
						glm::vec4 empty2;
						glm::decompose(m_incomingTransform.modelMatrix, 
							m_incomingTransform.transform.scale, 
							m_incomingTransform.transform.rotation, 
							m_incomingTransform.transform.position, 
							empty1, 
							empty2);

						m_incomingTransform.transform.rotation = glm::conjugate(m_incomingTransform.transform.rotation);


						//m_incomingTransform.transform.position = glm::make_vec3(transformData.translation);
						//m_incomingTransform.transform.rotation = glm::make_quat(transformData.rotation);
						//m_incomingTransform.transform.scale = glm::make_vec3(transformData.scale);




						m_incomingTransform.transformReady = true;

					}
				}
				

				// Case end
				break;
			}

			case ComData::DataType::MATERIAL:
			{
				// Check if material has been parsed and is ready
				if (!m_incomingMaterial.ready)
				{
					if (mayaCom->CanRead(s_materialSize))
					{
						// Read mesh
						ComData::Material materialData;
						mayaCom->Read((char*)&materialData, s_materialSize);

						m_incomingMaterial.material.name = materialData.name;
						m_incomingMaterial.material.diffuse[0] = materialData.diffuse[0];
						m_incomingMaterial.material.diffuse[1] = materialData.diffuse[1];
						m_incomingMaterial.material.diffuse[2] = materialData.diffuse[2];


						if (materialData.albedo != "")
						{
							std::string albedoFile = materialData.albedo;
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

								m_incomingMaterial.material.texture = true;
								m_incomingMaterial.material.textureID.resize(1);
								m_incomingMaterial.material.textureID[0] = texture;
							}
							else
							{
								std::cout << "Failed to load texture" << std::endl;
							}
							stbi_image_free(data);
						}
						else
						{
							m_incomingMaterial.material.texture = false;
						}


						m_incomingMaterial.ready = true;
					}
				}

				// Case end
				break;
			}

			case ComData::DataType::MATERIALCHANGED:
			{
				if (!m_incomingMatChange.ready)
				{
					if (mayaCom->CanRead(s_matChangedSize))
					{
						// Read mesh
						ComData::MeshMatChange data;
						mayaCom->Read((char*)&data, s_matChangedSize);
						m_incomingMatChange.mesh = data.meshName;
						m_incomingMatChange.material = data.materialName;
						m_incomingMatChange.ready = true;

					}
				}
				break;
			}

			case ComData::DataType::CAMERA:
			{
				if (!m_incomingCamera.ready)
				{
					if (mayaCom->CanRead(s_cameraSize))
					{
						// Read mesh
						ComData::Camera data;
						mayaCom->Read((char*)&data, s_cameraSize);
					
						if (data.type == ComData::CameraType::ORTHO)
						{
							m_incomingCamera.perspective = false;
							m_incomingCamera.width = data.width;
						}
						if (data.type == ComData::CameraType::TRANSFORMORTHO)
						{
							m_incomingCamera.perspective = false;
							m_incomingCamera.transform = true;
							m_incomingCamera.width = data.width;
						}
						if (data.type == ComData::CameraType::TRANSFORMPERPS)
						{
							m_incomingCamera.transform = true;
						}

						m_incomingCamera.name = data.name;
						m_incomingCamera.pos = glm::vec3(data.pos[0], data.pos[1], data.pos[2]);
						m_incomingCamera.pitch = glm::degrees(data.pitch);
						m_incomingCamera.yaw = -glm::degrees(data.yaw) - 90.0f;	//TODO: maybe fix this?

						m_incomingCamera.ready = true;
					}
				}
				break;
			}

			default:
				break;
			}


			// Switch end
		}
		
	}

	
}

bool Reciever::GetMesh(Mesh*& mesh, std::string& material)
{
	if (m_incomingMesh.meshReady)
	{
		mesh = m_incomingMesh.mesh;
		material = m_incomingMesh.material;
		ClearPackets();
		return true;
	}
	else
	{
		mesh = nullptr;
		return false;
	}


	return false;
}

bool Reciever::UpdateTransform(std::string& name, Transform& transform)
{
	if (m_incomingTransform.transformReady)
	{
		name = m_incomingTransform.name;
		transform = m_incomingTransform.transform;



		ClearPackets();
		return true;
	}

	return false;
}


bool Reciever::UpdateMaterial()
{
	if (m_incomingMaterial.ready)
	{
		Material* newMat = MaterialMap::getInstance()->createMaterial(m_incomingMaterial.material.name);

		newMat->diffuse[0] = m_incomingMaterial.material.diffuse[0];
		newMat->diffuse[1] = m_incomingMaterial.material.diffuse[1];
		newMat->diffuse[2] = m_incomingMaterial.material.diffuse[2];

		newMat->texture = m_incomingMaterial.material.texture;
		newMat->textureID = m_incomingMaterial.material.textureID;

		ClearPackets();
		return true;
	}

	return false;
}

bool Reciever::UpdateCamera(std::vector<Camera*>& cameras, std::map<std::string, Camera*>& cameraMap)
{
	if (m_incomingCamera.ready)
	{
		if (cameraMap.find(m_incomingCamera.name) != cameraMap.end())
		{
			Camera* cam = cameraMap[m_incomingCamera.name];
			cam->setCameraPos(m_incomingCamera.pos);
			cam->setCameraYaw(m_incomingCamera.yaw);
			cam->setCameraPitch(m_incomingCamera.pitch);

			if (!m_incomingCamera.perspective)
				cam->setOrthoWidth(m_incomingCamera.width);
		}
		else
		{
			cameras.push_back(new Camera());
			cameraMap[m_incomingCamera.name] = cameras.back();


			cameras.back()->setCameraPos(m_incomingCamera.pos);
			cameras.back()->setCameraYaw(m_incomingCamera.yaw);
			cameras.back()->setCameraPitch(m_incomingCamera.pitch);

			if (!m_incomingCamera.perspective)
				cameras.back()->setOrthoWidth(m_incomingCamera.width);
		}				   


		Renderer* renderer = Renderer::getInstance();
		renderer->setupCamera(cameraMap[m_incomingCamera.name]);

		ClearPackets();
		return true;
	}


	return false;
}

bool Reciever::ChangeMat(std::string& mesh, std::string& mat)
{
	if (m_incomingMatChange.ready)
	{
		mesh = m_incomingMatChange.mesh;
		mat = m_incomingMatChange.material;


		if (!MaterialMap::getInstance()->getMaterial(mat))
			MaterialMap::getInstance()->createMaterial(mat);


		ClearPackets();
		return true;
	}

	return false;
}



void Reciever::ClearPackets()
{
	// Whatever type it was; clear it
	m_incomingMesh.Clear();
	m_incomingTransform.Clear();
	m_incomingMaterial.Clear();
	m_incomingMatChange.Clear();
	m_incomingCamera.Clear();

	// Clear header
	m_incomingPacket.Clear();
}

void Reciever::Destroy()
{
	mayaCom->UnmapView();
	mayaCom->CloseFile();
	if (mayaCom)
		delete mayaCom;

	delete m_senderInstance;
	m_senderInstance = nullptr;
}
