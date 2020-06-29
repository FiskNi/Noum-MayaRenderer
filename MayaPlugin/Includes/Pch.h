#ifndef _PCH_H
#define _PCH_H

// some definitions for the DLL to play nice with Maya
#define NT_PLUGIN
#define REQUIRE_IOSTREAM
#define EXPORT __declspec(dllexport)

// Libraries to link from Maya
// This can be also done in the properties setting for the project.
#pragma comment(lib,"Foundation.lib")
#pragma comment(lib,"OpenMaya.lib")
#pragma comment(lib,"OpenMayaUI.lib")

#include <maya/MCameraSetMessage.h>
#include <maya/MItDependencyGraph.h>

#include <maya/MStreamUtils.h>

#include <maya/MFloatMatrix.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshFaceVertex.h>
#include <maya/MPxSurfaceShape.h>
#include <maya/MPlugArray.h>


#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnTransform.h>

#include <maya/MFnCamera.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnPointLight.h>

#include <maya/MImage.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MPoint.h>
#include <maya/MMatrix.h>
#include <maya/MEulerRotation.h>
#include <maya/MVector.h>
#include <maya/MItDag.h>
#include <maya/M3dView.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnNumericAttribute.h>

// Wrappers
#include <maya/MGlobal.h>
#include <maya/MCallbackIdArray.h>

// Messages
#include <maya/MMessage.h>
#include <maya/MTimerMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MEventMessage.h>
#include <maya/MPolyMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MDagPath.h>
#include <maya/MDagMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MModelMessage.h>

#include <maya/MTimer.h>

// Commands
#include <maya/MPxCommand.h>


#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <queue>
#include <sstream>
#include <map>
#include <unordered_map>

#include <Includes/ObjectFormats.h>
#include <Mayacommunicator/Sender.h>


#endif

