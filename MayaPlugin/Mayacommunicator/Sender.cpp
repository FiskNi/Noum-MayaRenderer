#include "Includes/Pch.h"
#include "Sender.h"

Sender* Sender::m_senderInstance = 0;
Sender::Sender() 
{
	m_mayaCom = new MayaCom();
	m_connected = false;

	s_headerSize = sizeof(ComData::Header); 
	s_meshSize = sizeof(ComData::Mesh);
	s_vertexSize = sizeof(ComData::Vertex);
	s_faceSize = sizeof(ComData::Face);
	s_transformSize = sizeof(ComData::TransformChange);
	s_materialSize = sizeof(ComData::Material);
	s_matChangedSize = sizeof(ComData::MeshMatChange);
	s_cameraSize = sizeof(ComData::Camera);

	m_reconnectIn = reonnectTime;
	Connect();
}

void Sender::Init()
{
	if (m_senderInstance == 0) 
		m_senderInstance = new Sender();
}

Sender* Sender::GetInstance()
{
	if (m_senderInstance == 0) 
		m_senderInstance = new Sender();
	return m_senderInstance;
}

void Sender::Connect()
{
	std::cout << "Trying to connect" << endl;
	if (m_mayaCom->OpenFileMap())
	{
		m_mayaCom->GetMap();
		m_connected = true;

		std::cout << "Connected" << endl;
	}
	else
	{
		m_reconnectIn = reonnectTime;
		std::cout << "Failed to connect" << endl;
	}
}

void Sender::Process(float deltaTime)
{
	if (!m_connected)
	{
		m_reconnectIn -= deltaTime;
		if (m_reconnectIn <= 0)
			Connect();
	}
	
	if (m_connected)
	{
		Send();
	}
}

bool Sender::Done()
{
	if (m_packets.size() <= 0)
		return true;


	return false;
}

void Sender::Submit()
{
	std::cout << "Submitting" << std::endl;
}

void Sender::Submit(Mesh *mesh)
{
	Packet newPacket;
	ComData::Header &header = newPacket.headData;
	ComData::Mesh &meshData = newPacket.mesh.meshData;

	// Get data
	MPointArray vertices;
	MIntArray faceTriangleCount;	//	Triangles amount per polygon
	MIntArray triangleVertices;		//	Triangle vertices	OBJECT RELATIVE
	mesh->meshFn->getPoints(vertices);
	mesh->meshFn->getTriangles(faceTriangleCount, triangleVertices);


	// For each polygon
	int triangleIndex = 0;
	int newFaceIndex = 0;
	for (int polygon = 0; polygon < faceTriangleCount.length(); polygon++)
	{
		MIntArray polygonVertices;		//	OBJECT RELATIVE
		mesh->meshFn->getPolygonVertices(polygon, polygonVertices);

		
		// For each triangle in polygon
		for (int j = 0; j < faceTriangleCount[polygon]; j++)
		{
			newPacket.mesh.facePackets.emplace_back();

			int vertexList[3];
			mesh->meshFn->getPolygonTriangleVertices(polygon, j, vertexList);

			// For each vertex in triangle
			for (int v = 0; v < 3; v++)
			{
				int thisVertexIndex = triangleVertices[v + (triangleIndex * 3)];

				newPacket.mesh.vertexPackets.emplace_back();
				newPacket.mesh.vertexPackets.back().vertexData.position[0] = vertices[thisVertexIndex].x;
				newPacket.mesh.vertexPackets.back().vertexData.position[1] = vertices[thisVertexIndex].y;
				newPacket.mesh.vertexPackets.back().vertexData.position[2] = vertices[thisVertexIndex].z;

				MVector normal;
				mesh->meshFn->getFaceVertexNormal(polygon, thisVertexIndex, normal);
				//mesh->meshFn->getPolygonNormal(polygon, normal);

				newPacket.mesh.vertexPackets.back().vertexData.normal[0] = normal.x;
				newPacket.mesh.vertexPackets.back().vertexData.normal[1] = normal.y;
				newPacket.mesh.vertexPackets.back().vertexData.normal[2] = normal.z;

				int vertexList[3];

				mesh->meshFn->getPolygonTriangleVertices(polygon, j, vertexList);

				int index = 0;
				for (int locali = 0; locali < polygonVertices.length(); locali++)
				{
					if (thisVertexIndex == polygonVertices[locali])
					{
						index = locali;
						break;
					}
				}

				float x, y;
				mesh->meshFn->getPolygonUV(polygon, index, x, y);


				newPacket.mesh.vertexPackets.back().vertexData.uv[0] = x;
				newPacket.mesh.vertexPackets.back().vertexData.uv[1] = y;

				//newPacket.mesh.vertexPackets.back().vertexData.uv[0] = xy[0];
				//newPacket.mesh.vertexPackets.back().vertexData.uv[1] = xy[1];
				
				newPacket.mesh.facePackets.back().faceData.indices[v] = newFaceIndex;
				newFaceIndex++;
			}

			


			

			// Moved up one triangle;
			triangleIndex++;
		}
	}



	// Assign data to write
	// Header
	header.dataType = ComData::DataType::MESH;
	// Mesh
	strcpy_s(meshData.name, mesh->transform.name.c_str());

	// Material
	MObjectArray shaders;	// Material list of mesh
	MIntArray indices;		// Polygon indices, not used for now
	mesh->meshFn->getConnectedShaders(0, shaders, indices);
	MPlugArray connections;
	MFnDependencyNode shaderGroup(shaders[0]);
	MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
	shaderPlug.connectedTo(connections, true, false);


	std::string matname;
	switch (connections[0].node().apiType())
	{
	case MFn::kLambert:
	{
		MFnLambertShader mfn(connections[0].node());
		matname = mfn.name().asChar();
		break;
	}
	case MFn::kPhong:
	{
		MFnPhongShader mfn(connections[0].node());
		matname = mfn.name().asChar();
		break;
	}
	case MFn::kBlinn:
	{
		MFnBlinnShader mfn(connections[0].node());
		matname = mfn.name().asChar();
		break;
	}

	default:
		break;
	}
	strcpy_s(meshData.material, matname.c_str());


	mesh->transform.transformFn->transformationMatrix().get(meshData.matrix);
	meshData.vertexCount = newPacket.mesh.vertexPackets.size();
	meshData.faceCount = newPacket.mesh.facePackets.size();

	m_packets.push_back(newPacket);
}

void Sender::SubmitMeshTransform(std::string name, Transform transform)
{
	Packet newPacket;
	ComData::Header& header = newPacket.headData;
	ComData::TransformChange& transformData = newPacket.transform.transformData;

	// Header
	header.dataType = ComData::DataType::MESHTRANSFORMED;
	//Transform
	strcpy_s(transformData.name, name.c_str());



	MDagPath thisDagNode;
	transform.dagPath->getPath(thisDagNode);
	//MMatrix transformMat = FindParent(transform.transformNode);
	MMatrix transformMat = thisDagNode.inclusiveMatrix();


	MVector translation = transform.transformFn->transformation().getTranslation(MSpace::kWorld);
	//MVector translation = thisDagNode.inclusiveMatrix().getTranslation(MSpace::kWorld);
	transformData.translation[0] = translation.x;
	transformData.translation[1] = translation.y;
	transformData.translation[2] = translation.z;
	
	double rot[4];
	transform.transformFn->transformation().getRotationQuaternion(rot[0], rot[1], rot[2], rot[3], MSpace::kWorld);
	transformData.rotation[0] = (float)rot[0];
	transformData.rotation[1] = (float)rot[1];
	transformData.rotation[2] = (float)rot[2];
	transformData.rotation[3] = (float)rot[3];

	double sc[3];
	transform.transformFn->transformation().getScale(sc, MSpace::kWorld);
	transformData.scale[0] = (float)sc[0];
	transformData.scale[1] = (float)sc[1];
	transformData.scale[2] = (float)sc[2];

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			transformData.matrix[j + (i * 4)] = transformMat[i][j];


	m_packets.push_back(newPacket);
}


void Sender::SubmitCamera(Camera camera, bool transform)
{
	Packet newPacket;
	ComData::Header& header = newPacket.headData;
	ComData::Camera& cameraData = newPacket.camera.data;

	MFnTransform transformFn(camera.cameraFn->parent(0));

	// Header
	header.dataType = ComData::DataType::CAMERA;

	// Camera
	strcpy_s(cameraData.name, camera.cameraFn->name().asChar());

	if (camera.cameraFn->isOrtho())
	{
		cameraData.type = ComData::CameraType::ORTHO;
		
	}
	if (transform)
	{
		cameraData.type = ComData::CameraType::TRANSFORMPERPS;
		if (camera.cameraFn->isOrtho())
			cameraData.type = ComData::CameraType::TRANSFORMORTHO;
	}

	MVector translation = transformFn.transformation().getTranslation(MSpace::kWorld);
	cameraData.pos[0] = translation.x;
	cameraData.pos[1] = translation.y;
	cameraData.pos[2] = translation.z;

	double rotation[3];
	MTransformationMatrix::RotationOrder order;
	transformFn.transformation().getRotation(rotation, order);

	cameraData.pitch = rotation[0];
	cameraData.yaw = rotation[1];

	cameraData.width = (float)camera.cameraFn->orthoWidth();

	m_packets.push_back(newPacket);
}


void Sender::SubmitMaterial(Material material)
{
	Packet newPacket;
	ComData::Header& header = newPacket.headData;
	ComData::Material &materialData = newPacket.material.materialData;

	// Header
	header.dataType = ComData::DataType::MATERIAL;
	//Transform
	strcpy_s(materialData.name, material.name.c_str());

	// Place data in these
	MColor color;
	std::string texturePath = "";
	
	// Find data
	switch (material.type)
	{
	case Material::Type::LAMBERT:
	{
		color = material.lambertFn->color();
		MItDependencyGraph itDep(material.lambertFn->object(), MFn::kFileTexture, MItDependencyGraph::kUpstream, MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kNodeLevel);
		MObject textureObj;
		while (!itDep.isDone())
		{
			textureObj = itDep.thisNode();
			if (textureObj.hasFn(MFn::kFileTexture))
			{
				MFnDependencyNode textureFn(textureObj);

				MPlug ftn = textureFn.findPlug("ftn");
				MString filename;
				ftn.getValue(filename);

				texturePath = filename.asChar();


				break;
			}

			itDep.next();
		}
		break;
	}
	case Material::Type::PHONG:
	{
		color = material.phongFn->color();
		break;
	}
	case Material::Type::BLINN:
	{
		color = material.blinnFn->color();
		break;
	}
	default:
		break;
	}

	// Parse data
	materialData.diffuse[0] = color.r;
	materialData.diffuse[1] = color.g;
	materialData.diffuse[2] = color.b;

	strcpy_s(materialData.albedo, texturePath.c_str());
	m_packets.push_back(newPacket);
}

void Sender::SubmitMatChange(std::string mesh, std::string material)
{
	Packet newPacket;
	ComData::Header& header = newPacket.headData;
	ComData::MeshMatChange& data = newPacket.meshmatChanged.data;

	// Header
	header.dataType = ComData::DataType::MATERIALCHANGED;

	// Data
	strcpy_s(data.meshName, mesh.c_str());
	strcpy_s(data.materialName, material.c_str());

	m_packets.push_back(newPacket);
}


void Sender::Send()
{
	for (int i = 0; i < m_packets.size(); i++)
	{
		if (!m_packets[i].headWritten)
		{
			if (m_mayaCom->CanWrite(s_headerSize))
			{
				m_mayaCom->Write(&m_packets[i].headData, s_headerSize);
				m_packets[i].headWritten = true;
			}
			else
			{
				break;
			}
		}

		switch (m_packets[i].headData.dataType)
		{
		case ComData::DataType::MESH:
		{
			MeshPacket& p = m_packets[i].mesh;
			if (!p.meshWritten)
			{
				if (m_mayaCom->CanWrite(s_meshSize))
				{
					m_mayaCom->Write(&p.meshData, s_meshSize);
					p.meshWritten = true;
				}
				else
				{
					break;
				}
			}

			// Vertices
			if (p.meshWritten)
			{
				for (int i = 0; i < p.vertexPackets.size(); i++)
				{
					if (!p.vertexPackets[i].written)
					{
						if (m_mayaCom->CanWrite(s_vertexSize))
						{
							m_mayaCom->Write(&p.vertexPackets[i].vertexData, s_vertexSize);
							p.vertexPackets[i].written = true;

							if (i == p.vertexPackets.size() - 1)
								p.verticesWritten = true;
						}
						else
						{
							break;
						}
					}
				}

				if (p.verticesWritten)
				{
					for (int i = 0; i < p.facePackets.size(); i++)
					{
						if (!p.facePackets[i].written)
						{
							if (m_mayaCom->CanWrite(s_faceSize))
							{
								m_mayaCom->Write(&p.facePackets[i].faceData, s_faceSize);
								p.facePackets[i].written = true;

								if (i == p.facePackets.size() - 1)
									p.facesWritten = true;
							}
							else
							{
								break;
							}
						}
					}
				}
			}
			

			if (p.facesWritten)
				m_packets[i].allWritten = true;

			// Case end
			break;
		}

		case ComData::DataType::MESHTRANSFORMED:
		{
			TransformPacket& p = m_packets[i].transform;

			if (!p.written)
			{
				if (m_mayaCom->CanWrite(s_transformSize))
				{
					m_mayaCom->Write(&p.transformData, s_transformSize);
					p.written = true;
				}
				else
				{
					break;
				}
			}

			if (p.written)
				m_packets[i].allWritten = true;

			// Case end
			break;
		}

		case ComData::DataType::MATERIAL:
		{
			MaterialPacket& p = m_packets[i].material;

			if (!p.written)
			{
				if (m_mayaCom->CanWrite(s_materialSize))
				{
					m_mayaCom->Write(&p.materialData, s_materialSize);
					p.written = true;
				}
				else
				{
					break;
				}
			}

			if (p.written)
				m_packets[i].allWritten = true;

			// Case end
			break;
		}

		case ComData::DataType::MATERIALCHANGED:
		{
			MeshMatChangedPacket& p = m_packets[i].meshmatChanged;
			if (!p.written)
			{
				if (m_mayaCom->CanWrite(s_matChangedSize))
				{
					m_mayaCom->Write(&p.data, s_matChangedSize);
					p.written = true;
				}
				else
				{
					break;
				}
			}

			if (p.written)
				m_packets[i].allWritten = true;

			break;
		}

		case ComData::DataType::CAMERA:
		{
			CameraPacket& p = m_packets[i].camera;
			if (!p.written)
			{
				if (m_mayaCom->CanWrite(s_cameraSize))
				{
					m_mayaCom->Write(&p.data, s_cameraSize);
					p.written = true;
				}
				else
				{
					break;
				}
			}

			if (p.written)
				m_packets[i].allWritten = true;

			break;
		}

		
		// Switch end
		} 

		if (m_packets[i].allWritten)
		{
			m_packets.erase(m_packets.begin() + i);
			i--;
		}
		else
		{
			break;
		}
	}




}

void Sender::Send(ComData::Header *header)
{
	
}

void Sender::Send(ComData::Mesh* mesh)
{
	size_t dataSize = sizeof(ComData::Mesh);
	m_mayaCom->Write(mesh, dataSize);

	std::cout << "Sending" << std::endl;
}

void Sender::Destroy()
{
	m_mayaCom->UnmapView();
	if (m_mayaCom)
		delete m_mayaCom;
}




