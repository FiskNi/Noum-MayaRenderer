#include <Pch/Pch.h>
#include "Mesh.h"

Mesh::Mesh()
{
	m_name = {};
	m_filePath = {};
	m_materialName = {};
	m_skeleton = {};
	m_indexInFile = 0;

	m_vertexCount = 0;
	m_faceCount = 0;
}

Mesh::~Mesh()
{
	
}

void Mesh::Destroy()
{	
	glDeleteVertexArrays(1, &m_vertexBuffer.vao);
	glDeleteBuffers(1, &m_vertexBuffer.vbo);
	glDeleteBuffers(1, &m_vertexBuffer.ibo);
	
}

void Mesh::setUpMesh(std::vector<Vertex> vertices, std::vector<Face> faces)
{
	m_vertexCount = (int)vertices.size();
	m_faceCount = (int)faces.size();

	m_vertices = vertices;
	m_faces = faces;
}

void Mesh::setUpMesh(std::vector<Vertex2> vertices, std::vector<Face> faces)
{
	m_vertexCount = (int)vertices.size();
	m_faceCount = (int)faces.size();

	m_skeleVertices = vertices;
	m_faces = faces;
}

void Mesh::loadMesh(std::string fileName)
{
	BGLoader tempLoader;	// The file loader
	tempLoader.LoadMesh(MESHPATH + fileName);

	saveFilePath(tempLoader.GetFileName(), 0);
	nameMesh(tempLoader.GetMeshName());
	setUpMesh(tempLoader.GetVertices(), tempLoader.GetFaces());
}

void Mesh::nameMesh(std::string name)
{
	m_name = name;
}

void Mesh::saveFilePath(std::string name, int index)
{
	m_filePath = name;
	m_indexInFile = index;
}

void Mesh::setUpBuffers()
{
	glGenVertexArrays(1, &m_vertexBuffer.vao);
	glGenBuffers(1, &m_vertexBuffer.vbo);
	glGenBuffers(1, &m_vertexBuffer.ibo);

	glBindVertexArray(m_vertexBuffer.vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBuffer.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_faces.size() * sizeof(int) * 3,
		&m_faces[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normals));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UV));
	//vertex tangent for normal mapping
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glBindVertexArray(0);


	m_vertexBuffer.nrOfFaces = static_cast<int>(m_faces.size());
}

void Mesh::setUpSkeleBuffers()
{
	glGenVertexArrays(1, &m_vertexBuffer.vao);
	glGenBuffers(1, &m_vertexBuffer.vbo);
	glGenBuffers(1, &m_vertexBuffer.ibo);

	glBindVertexArray(m_vertexBuffer.vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, m_skeleVertices.size() * sizeof(Vertex2), &m_skeleVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBuffer.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_faces.size() * sizeof(int) * 3,
		&m_faces[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, Normals));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, UV));
	// vertex bone index
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex2), (void*)offsetof(Vertex2, bone));
	// vertex weights
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, weight));
	glBindVertexArray(0);

	m_vertexBuffer.nrOfFaces = static_cast<int>(m_faces.size());
}

void Mesh::setMaterial(std::string matName)
{
	m_materialName = matName;
}

void Mesh::addAnimation(std::string name)
{
	m_animations.push_back(name);
}

void Mesh::setSkeleton(std::string name)
{
	m_skeleton = name;
}

const std::string& Mesh::getMaterial() const
{
	return m_materialName;
}

Buffers Mesh::getBuffers() const
{
	return m_vertexBuffer;
}
