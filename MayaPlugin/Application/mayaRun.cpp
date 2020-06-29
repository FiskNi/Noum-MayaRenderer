#pragma once
#include "Includes/Pch.h"
#include "mayaRun.h"


// Function Declarations
void EvaluateObjects();
void CheckNewObjects();
void CheckTransformUpdates();

using namespace std;
MCallbackIdArray callbackIdArray;
MObject m_node;
MStatus status = MS::kSuccess;
bool initBool = false;


// Time to allow maya to work and assign or remove data
static double g_mayaWorktime = 0.1;

vector<MObject> g_newObjects;
vector<MObject> g_objects;
vector<MayaInfo::Update> g_updates;

float g_eventTimer;
float g_cameraUpdateLimiter = 0.1f;
const float g_cameraUpdateLimit = 0.1f;

float g_meshUodateLimiter = 3.0f;
const float g_meshUpdateLimit = 3.0f;


bool g_newEvent;
bool g_allowWork;

//MGlobal::displayInfo((MString)"CB: plug info: " + srcPlug.info());
/*
 * how Maya calls this method when a node is added.
 * new POLY mesh: kPolyXXX, kTransform, kMesh
 * new MATERIAL : kBlinn, kShadingEngine, kMaterialInfo
 * new LIGHT    : kTransform, [kPointLight, kDirLight, kAmbientLight]
 * new JOINT    : kJoint
 */
void NodeAdded(MObject& node, void* clientData = NULL)
{
	if (!node.isNull())
	{
		// Add to object list
		g_newObjects.push_back(node);
		cout << "Added: " << node.apiTypeStr() << endl;


		g_eventTimer = 0;
		g_allowWork = false;
		g_newEvent = true;
	}
}

void NodeRenamed(MObject &node, const MString &str, void *clientData = NULL)
{
	cout << "[A2] --- Node renamed: " << MFnDagNode(node).fullPathName() << endl;
	cout << str << endl;

	//for (MObject obj : g_newObjects)
	//{
	//	if (!obj.isNull())
	//	{
	//		if (obj.hasFn(MFn::kMesh))
	//		{
	//			// Check if mesh exists
	//			for (int i = 0; i < mr_meshes.size(); i++)
	//				if (mr_meshes[i].name == str.asChar())
	//					mr_meshes[i].name = MFnMesh(obj).name().asChar();
	//		}

	//		if (obj.hasFn(MFn::kTransform))
	//		{
	//			// Check if mesh exists
	//			for (int i = 0; i < mr_transform.size(); i++)
	//				if (mr_transform[i].name == str.asChar())
	//					mr_transform[i].name = MFnTransform(obj).name().asChar();
	//		}
	//	}
	//}
}


void NodeAboutToDelete(MObject& node, MDGModifier& modifier, void* clientData)
{
	cout << "Node about to be removed: " << MFnDagNode(node).fullPathName() << endl;
	
	MFn::Type nodeType = node.apiType();

	switch (nodeType)
	{
	case MFn::kMesh:
	{
		MFnMesh meshfn(node);

		MeshMap::GetInstance()->RemoveMesh(meshfn.name().asChar());

		cout << "Mesh deleted: " << meshfn.name().asChar() << endl;
		break;
	}

	default:
		break;
	}


}

void NodePreRemoval(MObject& node, void* clientData)
{
	//cout << "Node pre removal: " << MFnDagNode(node).fullPathName() << endl;
}

void NodeRemoved(MObject& node, void* clientData)
{
	//cout << "Node removed " << node.apiTypeStr() << endl;
}



void UpdateChildren(MObject child)
{
	// Case transform changed
	if (child.apiType() == MFn::kTransform)
	{
		MFnTransform transformFn(child);

		if (transformFn.child(0).apiType() == MFn::Type::kMesh)
		{
			MFnMesh meshFn(transformFn.child(0));
			g_updates.push_back(MayaInfo::Update(MFn::Type::kTransform, ComData::DataType::MESH, meshFn.name().asChar()));

			for (int i = 0; i < transformFn.childCount(); ++i)
			{
				MObject child = transformFn.child(i);
				UpdateChildren(child);
			}
		}
	}

}

void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& srcPlug, MPlug& dstPlug, void* clientData = NULL)
{

	// Case transform changed
	if (srcPlug.node().apiType() == MFn::kTransform)
	{
		MFnTransform transformFn(srcPlug.node());

		if (transformFn.child(0).apiType() == MFn::Type::kMesh)
		{
			MFnMesh meshFn(transformFn.child(0));
			g_updates.push_back(MayaInfo::Update(MFn::Type::kTransform, ComData::DataType::MESH, meshFn.name().asChar()));

			for (int i = 0; i < transformFn.childCount(); ++i) 
			{
				MObject child = transformFn.child(i);
				UpdateChildren(child);
			}
		}

		if (transformFn.child(0).apiType() == MFn::Type::kCamera)
		{
			MFnCamera cameraFn(transformFn.child(0));
			CameraMap::GetInstance()->GetCamera(cameraFn.name().asChar())->updated = true;

			g_cameraUpdateLimiter = g_cameraUpdateLimit;
		}

		g_allowWork = true;
	}


	cout << srcPlug.node().apiTypeStr() << endl;

	if (srcPlug.node().apiType() == MFn::Type::kCamera)
	{

		MFnCamera cameraFn(srcPlug.node());
		CameraMap::GetInstance()->GetCamera(cameraFn.name().asChar())->updated = true;
			
		g_cameraUpdateLimiter = g_cameraUpdateLimit;
		g_allowWork = true;
		
	}


	// Case Material changed
	if (srcPlug.node().apiType() == MFn::kLambert)
	{
		MFnLambertShader lambertFn(srcPlug.node());
		g_updates.push_back(MayaInfo::Update(MFn::Type::kLambert, ComData::DataType::MATERIAL, lambertFn.name().asChar()));

		g_allowWork = true;
	}

	if (srcPlug.node().apiType() == MFn::kMesh)
	{
		MFnMesh meshFn(srcPlug.node());
		std::string matname;

		// Case Material changed
		if (dstPlug.node().apiType() == MFn::Type::kShadingEngine)
		{
			bool failed = true;

			MItDependencyGraph itDep(dstPlug.node(), MFn::kInvalid, MItDependencyGraph::kUpstream, MItDependencyGraph::kDepthFirst, MItDependencyGraph::kNodeLevel);
			MObject obj;
			while (!itDep.isDone())
			{
				obj = itDep.thisNode();
				std::cout << obj.apiTypeStr() << std::endl;

				if (obj.hasFn(MFn::kLambert) || obj.hasFn(MFn::kBlinn) || obj.hasFn(MFn::kPhong))
					break;

				itDep.next();
			}

			switch (obj.apiType())
			{
			case MFn::kLambert:
			{
				MFnLambertShader mfn(obj);
				matname = mfn.name().asChar();
				failed = false;
				break;
			}
			case MFn::kPhong:
			{
				MFnPhongShader mfn(obj);
				matname = mfn.name().asChar();
				failed = false;
				break;
			}
			case MFn::kBlinn:
			{
				MFnBlinnShader mfn(obj);
				matname = mfn.name().asChar();
				failed = false;
				break;
			}

			default:
				break;
			}


			if (!failed)
			{
				MFnTransform transformFn(meshFn.parent(0));

				Sender::GetInstance()->SubmitMatChange(transformFn.name().asChar(), matname);
				g_allowWork = true;
			}
		}


		int dataType = MFnNumericAttribute(srcPlug.attribute()).unitType();
		if (dataType == MFnNumericData::k3Float)
		{
			// If float array is not a array of more plugs (likely specific vertex)
			if (!srcPlug.isArray())
			{
				Mesh* m = MeshMap::GetInstance()->GetMesh(meshFn.name().asChar());
				m->updated = true;
				g_meshUodateLimiter = g_meshUpdateLimit;
			}
		}



	}

}

void Connection(MPlug &srcPlug, MPlug &dstPlug, bool made, void* clientData = NULL)
{
	if (made)
	{
		// Case mesh plug changed
		if (srcPlug.node().apiType() == MFn::kMesh)
		{
			MString meshName = MFnMesh(srcPlug.node()).name();
			
		}


		if (!srcPlug.isNull())
		{
			
		}
	}
}

void Timer(float elapsedTime, float lastTime, void* clientData = NULL)
{
	g_meshUodateLimiter -= elapsedTime;
	g_cameraUpdateLimiter -= elapsedTime;

	// Time to allow maya to work
	if (g_eventTimer < g_mayaWorktime)
	{
		g_eventTimer += elapsedTime;
		if (g_eventTimer >= g_mayaWorktime)
			g_allowWork = true;
	}


	if (g_meshUodateLimiter <= 0.0f)
	{
		std::map<std::string, Mesh*>& meshMap = MeshMap::GetInstance()->GetMap();

		std::map<std::string, Mesh*>::iterator it;
		for (it = meshMap.begin(); it != meshMap.end(); it++)
		{
			if (it->second->updated)
			{
				g_allowWork = true;
			}

		}
	}
	if (g_cameraUpdateLimiter <= 0.0f)
	{
		std::map<std::string, Camera*>& cameraMap = CameraMap::GetInstance()->GetMap();

		std::map<std::string, Camera*>::iterator it;
		for (it = cameraMap.begin(); it != cameraMap.end(); it++)
		{
			if (it->second->updated)
			{
				g_allowWork = true;
			}
		}
	}


	if (g_allowWork)
	{
		g_allowWork = false;
		EvaluateObjects();

		// Process writing events
		Sender::GetInstance()->Process(elapsedTime);
		if (Sender::GetInstance()->Done())
			g_allowWork = false;


		g_newEvent = false;
		//g_allowWork = false;
	}
}

//MMatrix FindParent(MObject child)
//{
//	MFnDagNode dagPath(child);
//	MDagPath thisDagNode;
//	dagPath.getPath(thisDagNode);
//
//	if (dagPath.parentCount() == 1)
//	{
//		if (dagPath.parent(0).apiType() == MFn::kTransform)
//			return FindParent(dagPath.parent(0)) * dagPath.transformationMatrix();
//	}
//
//	return MMatrix();
//}

void EvaluateObjects()
{
	// Note
	// Temporary way of doing things
	// Currently evaluates all objects by removing ones with absent dagNodes
	// Adds new objects by checking if their name does not exist

	// Better solution:
	// Find deletion callbacks and remove objects as they get deleted
	// Never evaluate all objects, chance of being slow when a lot of objects are present in scene


	CheckNewObjects();
	CheckTransformUpdates();

	g_allowWork = true;
}

void CheckNewObjects()
{
	if (g_newObjects.size() == 0)
		return;

	// Check for bad or deleted objects, could also be done in some node or attribute removed function
	for (int i = (int)g_newObjects.size() - 1; i >= 0; i--)
	{
		/*!g_newObjects[i].hasFn(MFn::kMesh) &&
			!g_newObjects[i].hasFn(MFn::kTransform) ||*/
		// Delete objects that do not have a dagnode from list or ones that were most likely deleted (no parent)
		if (
			g_newObjects[i].hasFn(MFn::kDagNode) &&
			MFnDagNode(g_newObjects[i]).parentCount() == 0)
		{
			cout << "Deleted from object list: " << g_newObjects[i].apiTypeStr() << endl;
			g_newObjects.erase(g_newObjects.begin() + i);
		}
	}
	// List objects after cleaning
	cout << "Objects in list: " << g_newObjects.size() << endl;

	//MObject obj : g_newObjects
	for (int i = 0; i < g_newObjects.size(); i++)
	{
		MObject& obj = g_newObjects[i];
		MFn::Type objectType = obj.apiType();

		switch (objectType)
		{
		case MFn::kMesh:
		{
			cout << "Found new mesh" << endl;
			if (MeshMap::GetInstance()->ExistsWithName(MFnMesh(obj).name().asChar()))
			{
				cout << "------- ERROR: Mesh already exists with name: " << MFnMesh(obj).name().asChar() << endl;
				continue;
			}

			// If parent is transform (should always be)
			if (MFnMesh(obj).parent(0).apiType() == MFn::kTransform)
			{
				Mesh newMesh;
				// Assign function set
				newMesh.meshFn = new MFnMesh(obj);

				// Raw maya object
				newMesh.meshNode = obj;
				// Append callbacks
				newMesh.callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(obj, AttributeChanged));
				newMesh.callbackIdArray.append(MNodeMessage::addNameChangedCallback(obj, NodeRenamed));
				newMesh.callbackIdArray.append(MNodeMessage::addNodeAboutToDeleteCallback(obj, NodeAboutToDelete));
				newMesh.callbackIdArray.append(MNodeMessage::addNodePreRemovalCallback(obj, NodePreRemoval));
				// Com data
				newMesh.name = newMesh.meshFn->name().asChar();
				newMesh.vertices.resize(newMesh.meshFn->numVertices());
				

				// Transform node
				Transform& t = newMesh.transform;
				MObject& tobj = newMesh.meshFn->parent(0);		// Parent is the transform node of the mesh
				t.transformFn = new MFnTransform(newMesh.meshFn->parent(0));
				t.dagPath = new MFnDagNode(newMesh.meshFn->parent(0));
				t.dagPath->getPath(newMesh.transform.thisDagNode);

				// Raw maya object
				t.transformNode = tobj;
				// Append callbacks
				t.callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(tobj, AttributeChanged));
				t.callbackIdArray.append(MNodeMessage::addNameChangedCallback(tobj, NodeRenamed));
				t.callbackIdArray.append(MNodeMessage::addNodeAboutToDeleteCallback(tobj, NodeAboutToDelete));
				t.callbackIdArray.append(MNodeMessage::addNodePreRemovalCallback(tobj, NodePreRemoval));
				// Com data
				t.name = t.transformFn->name().asChar();

				Mesh *m = MeshMap::GetInstance()->CreateMesh(newMesh);
				Sender::GetInstance()->Submit(m);

				g_updates.push_back(MayaInfo::Update(MFn::Type::kTransform, ComData::DataType::MESH, newMesh.name));
				/*for (int i = 0; i < t.transformFn->childCount(); ++i)
				{
					MObject child = t.transformFn->child(i);
					UpdateChildren(child);
				}*/
			}
			
			
			break;
		}

		case MFn::kLambert:
		{
			// Assign function set
			Material newMaterial;
			newMaterial.lambertFn = new MFnLambertShader(obj);

			newMaterial.name = newMaterial.lambertFn->name().asChar();
			newMaterial.type = Material::Type::LAMBERT;

			// Append callbacks
			newMaterial.callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(obj, AttributeChanged));
			newMaterial.callbackIdArray.append(MNodeMessage::addNameChangedCallback(obj, NodeRenamed));
			newMaterial.callbackIdArray.append(MNodeMessage::addNodeAboutToDeleteCallback(obj, NodeAboutToDelete));
			newMaterial.callbackIdArray.append(MNodeMessage::addNodePreRemovalCallback(obj, NodePreRemoval));


			MaterialMap::GetInstance()->CreateMaterial(newMaterial);
			Sender::GetInstance()->SubmitMaterial(newMaterial);
			break;
		}
		case MFn::kPhong:
		{
			// Assign function set
			Material newMaterial;
			newMaterial.phongFn = new MFnPhongShader(obj);

			newMaterial.name = newMaterial.phongFn->name().asChar();
			newMaterial.type = Material::Type::PHONG;

			// Append callbacks
			newMaterial.callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(obj, AttributeChanged));
			newMaterial.callbackIdArray.append(MNodeMessage::addNameChangedCallback(obj, NodeRenamed));
			newMaterial.callbackIdArray.append(MNodeMessage::addNodeAboutToDeleteCallback(obj, NodeAboutToDelete));
			newMaterial.callbackIdArray.append(MNodeMessage::addNodePreRemovalCallback(obj, NodePreRemoval));

			MaterialMap::GetInstance()->CreateMaterial(newMaterial);
			Sender::GetInstance()->SubmitMaterial(newMaterial);
			break;
		}
		case MFn::kBlinn:
		{
			// Assign function set
			Material newMaterial;
			newMaterial.blinnFn = new MFnBlinnShader(obj);

			newMaterial.name = newMaterial.blinnFn->name().asChar();
			newMaterial.type = Material::Type::BLINN;

			// Append callbacks
			newMaterial.callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(obj, AttributeChanged));
			newMaterial.callbackIdArray.append(MNodeMessage::addNameChangedCallback(obj, NodeRenamed));
			newMaterial.callbackIdArray.append(MNodeMessage::addNodeAboutToDeleteCallback(obj, NodeAboutToDelete));
			newMaterial.callbackIdArray.append(MNodeMessage::addNodePreRemovalCallback(obj, NodePreRemoval));

			MaterialMap::GetInstance()->CreateMaterial(newMaterial);
			Sender::GetInstance()->SubmitMaterial(newMaterial);
			break;
		}
		case MFn::kCamera:
		{
			// Assign function set
			Camera newCamera;
			newCamera.cameraFn = new MFnCamera(obj);

			newCamera.name = newCamera.cameraFn->name().asChar();
			newCamera.type = Camera::Type::PERSP;
			if (newCamera.cameraFn->isOrtho())
				newCamera.type = Camera::Type::ORTHO;

			// Append callbacks
			newCamera.callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(obj, AttributeChanged));
			newCamera.callbackIdArray.append(MNodeMessage::addNameChangedCallback(obj, NodeRenamed));
			newCamera.callbackIdArray.append(MNodeMessage::addNodeAboutToDeleteCallback(obj, NodeAboutToDelete));
			newCamera.callbackIdArray.append(MNodeMessage::addNodePreRemovalCallback(obj, NodePreRemoval));


			// Transform node
			Transform& t = newCamera.transform;
			MObject& tobj = newCamera.cameraFn->parent(0);		// Parent is the transform node of the mesh
			t.transformFn = new MFnTransform(newCamera.cameraFn->parent(0));
			t.dagPath = new MFnDagNode(newCamera.cameraFn->parent(0));
			t.dagPath->getPath(newCamera.transform.thisDagNode);

			// Raw maya object
			t.transformNode = tobj;
			// Append callbacks
			t.callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(tobj, AttributeChanged));
			t.callbackIdArray.append(MNodeMessage::addNameChangedCallback(tobj, NodeRenamed));
			t.callbackIdArray.append(MNodeMessage::addNodeAboutToDeleteCallback(tobj, NodeAboutToDelete));
			t.callbackIdArray.append(MNodeMessage::addNodePreRemovalCallback(tobj, NodePreRemoval));
			// Com data
			t.name = t.transformFn->name().asChar();



			CameraMap::GetInstance()->CreateCamera(newCamera);
			Sender::GetInstance()->SubmitCamera(newCamera);
			break;
		}



		default:
			break;

		}
	}


	// Clear list after parsing new objects
	g_newObjects.clear();
}

void CheckTransformUpdates()
{
	// Updates all the vertices of the mesh

	if (g_meshUodateLimiter <= 0.0f)
	{
		std::map<std::string, Mesh*>& meshMap = MeshMap::GetInstance()->GetMap();

		std::map<std::string, Mesh*>::iterator it;
		for (it = meshMap.begin(); it != meshMap.end(); it++)
		{
			if (it->second->updated)
			{
				Sender::GetInstance()->Submit(it->second);
				it->second->updated = false;
			}
		}
	}

	if (g_cameraUpdateLimiter <= 0.0f)
	{
		std::map<std::string, Camera*>& cameraMap = CameraMap::GetInstance()->GetMap();

		std::map<std::string, Camera*>::iterator it;
		for (it = cameraMap.begin(); it != cameraMap.end(); it++)
		{
			if (it->second->updated)
			{
				g_updates.push_back(MayaInfo::Update(MFn::Type::kTransform, ComData::DataType::CAMERA, it->second->cameraFn->name().asChar()));
				it->second->updated = false;
			}
		}
	}



	



	for (int i = 0; i < g_updates.size(); i++)
	{
		switch (g_updates[i].dataType)
		{
			case ComData::DataType::MESH:
			{
				switch (g_updates[i].mayaType)
				{
					case MFn::Type::kTransform:
					{
						Transform& t = MeshMap::GetInstance()->GetMesh(g_updates[i].name)->transform;
						Sender::GetInstance()->SubmitMeshTransform(t.name, t);	
						break;
					}

			
					default:
						break;
				}

				break;
			}

			case ComData::DataType::CAMERA:
			{
				//if (g_updates.back().dataType == ComData::DataType::CAMERA && i != g_updates.size() - 1)
				//	break;

				switch (g_updates[i].mayaType)
				{
				case MFn::Type::kTransform:
				{
					
					const Camera* c = CameraMap::GetInstance()->GetCamera(g_updates[i].name);
					Sender::GetInstance()->SubmitCamera(*c, true);
					break;
				}


				default:
					break;
				}

				break;
			}

			case ComData::DataType::MATERIAL:
			{
				switch (g_updates[i].mayaType)
				{
					case MFn::Type::kLambert:
					{
						Material* mat = MaterialMap::GetInstance()->GetMaterial(g_updates[i].name);
						Sender::GetInstance()->SubmitMaterial(*mat);
						break;
					}

					default:
						break;
				}
			}


			default:
				break;

		}


		g_updates.erase(g_updates.begin() + i);
		i--;
	}
	
}


void cameraChanged(MObject& cameraSetNode, unsigned int multiIndex, MObject& oldCamera, MObject& newCamera, void* clientData)
{
	//MFnCamera cameraFn(newCamera);
	//g_updates.push_back(MayaInfo::Update(MFn::Type::kTransform, ComData::DataType::CAMERA, cameraFn.name().asChar()));
}


/*
 * Plugin entry point
 * For remote control of maya
 * open command port: commandPort -nr -name ":1234"
 * close command port: commandPort -cl -name ":1234"
 * Write command: see loadPlugin.py and unloadPlugin.py
 */
EXPORT MStatus initializePlugin(MObject obj) 
{
	MStatus res = MS::kSuccess;
	MFnPlugin myPlugin(obj, "level editor", "1.0", "Any", &res);
	if (MFAIL(res)) 
	{
		CHECK_MSTATUS(res);
		return res;
	}  
	
	// redirect cout to cerr, so that when we do cout goes to cerr
	// in the maya output window (not the scripting output!)
	std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
	std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());
	cout << "Plugin loaded ===========================" << endl << endl;
	

	// Initialize
	g_eventTimer = g_mayaWorktime + 1.0f;
	g_allowWork = false;
	g_newEvent = false;
	
	// Global callbacks for Maya events
	callbackIdArray.append(MDGMessage::addNodeAddedCallback(NodeAdded, "dependNode", NULL, &status));
	callbackIdArray.append(MDGMessage::addNodeRemovedCallback(NodeRemoved, "dependNode", NULL, &status));
	callbackIdArray.append(MDGMessage::addConnectionCallback(Connection, NULL, &status));
	//callbackIdArray.append(MCameraSetMessage::addCameraChangedCallback(cameraChanged, NULL, &status));

	// Current main loop
	callbackIdArray.append(MTimerMessage::addTimerCallback(0.01f, Timer, NULL, &status));

	// Materials
	MItDependencyNodes itDep(MFn::kLambert);
	while (!itDep.isDone())
	{
		// get a handle to this node
		MObject obj = itDep.thisNode();
		NodeAdded(obj, 0);

		itDep.next();
	}

	// Objects in scene
	MItDependencyNodes it(MFn::kDagNode);
	while (!it.isDone())
	{
		// get a handle to this node
		MObject obj = it.thisNode();
		NodeAdded(obj, 0);
		
		// move on to next node
		it.next();
	}


	g_eventTimer = 0;
	g_allowWork = false;
	g_newEvent = true;


	// Initialize static classes
	Sender::Init();
	MeshMap::Init();
	MaterialMap::Init();
	CameraMap::Init();
	

	return res;
}
	

EXPORT MStatus uninitializePlugin(MObject obj) 
{
	MFnPlugin plugin(obj);


	MeshMap::GetInstance()->CleanUp();
	MaterialMap::GetInstance()->CleanUp();
	CameraMap::GetInstance()->CleanUp();
	Sender::GetInstance()->Destroy();

	MMessage::removeCallbacks(callbackIdArray);


	return MS::kSuccess;
	cout << "Plugin unloaded =========================" << endl;
}