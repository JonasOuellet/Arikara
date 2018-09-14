#include "ArikaraSkinData.h"

//#include <maya/MIOStream.h>
#include <maya/MCommonSystemUtils.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MGlobal.h>
#include <maya/MFnTransform.h>

#include "../../GlobalDefine.h"
#include <fstream>
#include "../include/ArikaraSkinEditorCmd.h"


//List file in a folder c++
//https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
//#include <filesystem>


ArikaraSkinData::ArikaraSkinData()
{
	m_SaveFlag = false;
	m_LoadFlag = false;
	m_isSkinClusterCreated = false;
	m_LoadBindMatrixFlag = false;
	m_CleanFlag = false;
}

ArikaraSkinData::~ArikaraSkinData()
{

}

MStatus ArikaraSkinData::doIt(const MArgList& args)
{
	MStatus status;
	MSyntax mySyntax = newSyntax();
	MArgDatabase parsed(mySyntax, args);

	MSelectionList skinNodeSel;
	parsed.getObjects(skinNodeSel);
	unsigned int lastSelIndex = skinNodeSel.length() - 1;
	MDagPath tmp;
	status = skinNodeSel.getDagPath(lastSelIndex, tmp, m_components);
	if (status != MS::kSuccess)
	{
		status = skinNodeSel.getDependNode(lastSelIndex, m_SkinCluster); ReturnOnError(status);
		if (!m_SkinCluster.hasFn(MFn::kSkinClusterFilter))
		{
			displayError("Specified Node is not a Skin Cluster.");
			return MS::kInvalidParameter;
		}
		MItDependencyGraph dgIt(m_SkinCluster, MFn::Type::kMesh, MItDependencyGraph::kDownstream);
		if (!dgIt.isDone())
		{
			MObject geoObject = dgIt.currentItem();
			MFnDagNode dagNode(geoObject);
			dagNode.getPath(m_GeoDagPath);
		}
		else
		{
			displayError("Couldn't find geometry mesh.");
			return MS::kFailure;
		}
	}
	else
	{
		//Try to get the skin from the sel;
		MStatus status;
		if (m_components.isNull())
		{
			tmp.extendToShape();

			// Set Complete data.
			MFnSingleIndexedComponent MfnComp;
			m_components = MfnComp.create(MFn::kMeshVertComponent);
			MfnComp.setComplete(true);
		}
		m_GeoDagPath = tmp;
		MItDependencyGraph dgIt(tmp.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
		if (!dgIt.isDone())
		{
			m_SkinCluster = dgIt.currentItem();
		}
	}

	if (parsed.isFlagSet(kARISDSaveFlag))
	{
		if (m_SkinCluster.isNull())
		{
			displayError("Could not find skinCluster to save Data.");
			return MS::kInvalidParameter;
		}
		m_SaveFlag = true;
	}
	else if (parsed.isFlagSet(kARISDLoadFlag))
	{
		m_LoadFlag = true;

		m_LoadBindMatrixFlag = parsed.isFlagSet(kARISDBindMatFlag);
		m_CleanFlag = parsed.isFlagSet(kARISDCleanSkinFlag) || m_SkinCluster.isNull();
	}
	return redoIt();
}

MStatus ArikaraSkinData::redoIt()
{
	if (m_SaveFlag)
	{
		return saveSkinData();
	}
	else if (m_LoadFlag)
	{
		MStatus status;
		status = loadSkinData();
		if (status == MS::kSuccess)
			ArikaraSkinEditorCmd::updateToolUI();
		return status;
	}
	return MS::kSuccess;
}

MStatus ArikaraSkinData::undoIt()
{
	if (m_LoadFlag)
	{
		if (!m_CleanFlag)
		{
			MFnSkinCluster skin(m_SkinCluster);
			skin.setWeights(m_GeoDagPath, m_components, m_InfluenceArr, m_oldWeights, false);
		}
		if (m_LoadBindMatrixFlag)
		{
			m_MDGModMatrix.undoIt();
		}
		m_MDGModifier.undoIt();
		m_MDagModifier.undoIt();
		ArikaraSkinEditorCmd::updateToolUI();
	}

	return MS::kSuccess;
}

void* ArikaraSkinData::creator()
{
	return new ArikaraSkinData;
}

MSyntax ArikaraSkinData::newSyntax()
{
	MSyntax	syntax;
	syntax.setObjectType(MSyntax::MObjectFormat::kSelectionList, 1);
	syntax.useSelectionAsDefault(true);
	
	syntax.addFlag(kARISDSaveFlag, kARISDSaveFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDLoadFlag, kARISDLoadFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDBindMatFlag, kARISDBindMatFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDCleanSkinFlag, kARISDCleanSkinFlagLong, MSyntax::MArgType::kNoArg);

	return syntax;
}

MString ArikaraSkinData::pathName;
MString ArikaraSkinData::fileName;

MString ArikaraSkinData::DefaultPath()
{
	MString folder = MCommonSystemUtils::getEnv("MAYA_APP_DIR");
	folder += "/ArSkinData";
	MCommonSystemUtils::makeDirectory(folder);
	folder += "/";
	return folder;
}

MStatus ArikaraSkinData::saveSkinData()
{
	//displayInfo("Saving skin cluster");
	MStatus status;

	MString folder(DefaultPath());
	const char* fileName = (folder + "skinDataTest.ars").asChar();

#ifdef WIN32
	std::string test(fileName);
	std::replace(test.begin(), test.end(), '/', '\\');
	fileName = test.c_str();
#endif // WIN32

	displayInfo(fileName);

	ariJson::ariSkinCluster skinJson(m_SkinCluster, m_GeoDagPath);
	json j;
	skinJson.setJsonData(j);

	std::ofstream o;
	o.open(fileName, std::ios::out);
	if (o.good())
	{
		displayInfo("file is good");
		o << std::setw(4) << j << std::endl;
		o.close();
	}
	else
	{
		displayInfo("file is not good");
	}

	return MS::kSuccess;
}

MStatus ArikaraSkinData::loadSkinData()
{
	MStatus status;

	MString folder(DefaultPath());
	const char* fileName = (folder + "skinDataTest.ars").asChar();

#ifdef WIN32
	std::string test(fileName);
	std::replace(test.begin(), test.end(), '/', '\\');
	fileName = test.c_str();
#endif // WIN32

	std::ifstream i(fileName);

	json j;
	i >> j;

	ariJson::ariSkinCluster skinJson(j);


	MDoubleArray newWeights;
	//have to create a new skin cluster
	if (m_CleanFlag)
	{
		loadCleanSkinData(skinJson);

		newWeights = MDoubleArray(skinJson.influencesCount * skinJson.vertexCount);
		for (unsigned int v = 0; v < skinJson.vertexCount; v++)
		{
			for (unsigned int i = 0; i < skinJson.vertices[v].influences.size(); i++)
			{
				unsigned int ci = skinJson.vertices[v].influences[i];
				newWeights[v * skinJson.influencesCount + ci] = skinJson.vertices[v].weights[i];
			}
		}

		m_InfluenceArr.setLength(skinJson.influencesCount);
		for (unsigned int i = 0; i < skinJson.influencesCount; i++)
		{
			m_InfluenceArr[i] = i;
		}
	}
	else
	{
		//where not creating a new skin cluster
		//so we have to find the matching influences name (maybe influences aren't in the same order)
		//create a new influences and add it to the skin if needed.
		//Also m_component can be set from the selection.

		unsigned int influenceCount;
		MIntArray influIndex;
		loadSkinData(skinJson, influIndex, influenceCount);
		m_InfluenceArr.setLength(influenceCount);
		for (unsigned int i = 0; i < influenceCount; i++)
		{
			m_InfluenceArr[i] = i;
		}
		unsigned int vertexCount = skinJson.vertexCount;
		MFnSingleIndexedComponent MfnComp(m_components);
		if (!MfnComp.isComplete())
		{
			vertexCount = MfnComp.elementCount();
		}
		
		newWeights = MDoubleArray(vertexCount * influenceCount, 0);

		//if (!MfnComp.isComplete())
		//{
			for (unsigned int v = 0; v < vertexCount; v++)
			{
				int curVert = v;
				if (!MfnComp.isComplete())
					curVert = MfnComp.element(v);

				for (unsigned int i = 0; i < skinJson.vertices[curVert].influences.size(); i++)
				{
					int curInfluence = influIndex[skinJson.vertices[curVert].influences[i]];
					newWeights[v * influenceCount + curInfluence] = skinJson.vertices[curVert].weights[i];
				}
			}
		//}
		//else
		/*{
			for (unsigned int v = 0; v < vertexCount; v++)
			{
				for (unsigned int i = 0; i < skinJson.vertices[v].influences.size(); i++)
				{
					newWeights[v * influenceCount + influIndex[skinJson.vertices[v].influences[i]]] = skinJson.vertices[v].weights[i];
				}
			}
		}*/
	}

	MFnSkinCluster mfnSkin(m_SkinCluster);
	MFnDependencyNode mfnDep(m_SkinCluster);

	if (m_LoadBindMatrixFlag)
	{
		MPlug bindMatPlug = mfnDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);
		MFnMatrixData matData;
		for (unsigned int x = 0; x < skinJson.influencesCount; x++)
		{
			MSelectionList sel;
			sel.add(skinJson.influences[x].name.c_str());
			MDagPath dag;
			sel.getDagPath(0, dag);
			unsigned int physicIndex = x;
			if (!m_isSkinClusterCreated)
				physicIndex = mfnSkin.indexForInfluenceObject(dag);

			MMatrix mat(skinJson.influences[x].bindMat);
			MObject matObj = matData.create();
			matData.set(mat);
			MPlug curMat = bindMatPlug.elementByPhysicalIndex(physicIndex, &status); // .setValue(matObj);
			m_MDGModMatrix.newPlugValue(curMat, matObj);
		}
	}
	m_MDGModMatrix.newPlugValueInt(mfnDep.findPlug("mi", &status), skinJson.maxInfluences);
	m_MDGModMatrix.doIt();

	status = mfnSkin.setWeights(m_GeoDagPath, m_components, m_InfluenceArr, newWeights, false, &m_oldWeights);
	if (status != MS::kSuccess)
	{
		int errssss = 0;
	}

	return MS::kSuccess;
}

void ArikaraSkinData::loadSkinData(ariJson::ariSkinCluster& skinJson, MIntArray &influIndex, unsigned int& totalNewInfluences)
{
	MFnSkinCluster mfnSkin(m_SkinCluster);
	MFnDependencyNode MfnDep(m_SkinCluster);
	MString skinName = MfnDep.name();

	MDagPathArray influencesDagPath;
	unsigned int influencesCount = mfnSkin.influenceObjects(influencesDagPath);
	unsigned int totalInfluences = influencesCount;

	influIndex.setLength(skinJson.influencesCount);

	MSelectionList sel;
	MStatus status;
	MFnMatrixData matData;
	for (unsigned int i = 0; i < skinJson.influencesCount; i++)
	{
		bool found = false;
		MDagPath curJoint;

		sel.clear();
		MString curName(skinJson.influences[i].name.c_str());
		status = sel.add(curName);
		if (status != MS::kSuccess)
		{
			//create the new node.
			MObject newNode = m_MDagModifier.createNode("joint");
			curJoint = MFnDagNode(newNode).dagPath();

			m_MDagModifier.renameNode(newNode, curName);

			MMatrix mat(skinJson.influences[i].bindMat);
			mat = mat.inverse();
			MObject matObj = matData.create();
			matData.set(mat);

			MFnTransform trans(newNode);
			trans.set(mat);

		}
		else
		{
			sel.getDagPath(0, curJoint);
			for (unsigned int x = 0; x < influencesCount; x++)
			{
				if (curJoint == influencesDagPath[x])
				{
					influIndex[i] = x;
					found = true;
					break;
				}
			}
		}

		if (!found)
		{
			//add the node to the skin cluster.
			influIndex[i] = totalInfluences;
			totalInfluences++;

			MString baseCommand("skinCluster -e -wt 0.0 -lw 1 -dr 10.0 -ai ");
			MString unlockCommand("skinCluster -e -lw 0 -inf ");

			MString com(baseCommand + curJoint.partialPathName() + " " + skinName + ";");
			m_MDGModifier.commandToExecute(com);
			MString com2(unlockCommand + curJoint.partialPathName() + " " + skinName + ";");
			m_MDGModifier.commandToExecute(com2);
		}
	}

	m_MDagModifier.doIt();
	m_MDGModifier.doIt();
	totalNewInfluences = totalInfluences;
}

void ArikaraSkinData::loadCleanSkinData(ariJson::ariSkinCluster& skinJson)
{
	MStatus status;

	if (!m_SkinCluster.isNull())
	{
		m_MDGModifier.deleteNode(m_SkinCluster);
	}

	m_isSkinClusterCreated = true;

	MDagPath trans = m_GeoDagPath;
	trans.pop();

	MSelectionList sel;
	MString command = "skinCluster -tsb " + trans.partialPathName();
	MFnMatrixData matData;
	MObjectArray newInfluences;
	MIntArray newInfluencesIndex;
	for (unsigned int i = 0; i < skinJson.influencesCount; i++)
	{
		const char* name = skinJson.influences[i].name.c_str();
		status = sel.add(name);
		if (status != MS::kSuccess)
		{
			MObject newBone = m_MDagModifier.createNode("joint");
			m_MDagModifier.renameNode(newBone, name);

			MMatrix mat(skinJson.influences[i].bindMat);
			mat = mat.inverse();
			MObject matObj = matData.create();
			matData.set(mat);

			MFnTransform trans(newBone);
			trans.set(mat);

			//newInfluences.append(newBone);
			//newInfluencesIndex.append(i);
		}

		command += " " + MString(name);
	}

	m_MDagModifier.doIt();

	/*for (unsigned int i = 0; i < newInfluences.length(); i++)
	{
		MMatrix mat(skinJson.influences[i].bindMat);
		mat = mat.inverse();
		MObject matObj = matData.create();
		matData.set(mat);
		
		MFnTransform trans(newInfluences[i]);
		trans.set(mat);
	}*/

	status = m_MDGModifier.commandToExecute(command);
	m_MDGModifier.doIt();

	//reset the new skincluster and rename it
	MItDependencyGraph dgIt(m_GeoDagPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
	if (!dgIt.isDone())
	{
		m_SkinCluster = dgIt.currentItem();
		MFnDependencyNode tmp(m_SkinCluster);
		tmp.setName(skinJson.name.c_str());
	}
}

/*
class used to save/load data;
*/

ariJson::ariInfluence::ariInfluence(const MObject& pSkin, const MDagPath& pDagPath)
{
	MFnSkinCluster skin(pSkin);
	name = pDagPath.partialPathName().asChar();

}

ariJson::ariInfluence::ariInfluence(const MString& pName, const MMatrix &pBindMatrix)
{
	name = pName.asChar();
	pBindMatrix.get(bindMat);
}

ariJson::ariInfluence::~ariInfluence()
{
	//delete[] name; 
	//delete[] bindMat;
}


void ariJson::ariInfluence::setJsonData(json& pJson)
{
	pJson["n"] = name;
	pJson["pbm"] = bindMat;
}

void ariJson::ariInfluence::initFromData(const json& pJson)
{
	name = pJson["n"].get<std::string>();
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			bindMat[i][j] = pJson["pbm"][i][j];
		}
	}
}

ariJson::ariSkinCluster::ariSkinCluster(MObject &pSkinCluster, MDagPath& pGeo)
{
	m_skinCluster = pSkinCluster;
	MFnDependencyNode mfnDep(pSkinCluster);
	name = mfnDep.name().asChar();
	maxInfluences = mfnDep.findPlug("mi").asInt();

	MFnSkinCluster mfnSkin(pSkinCluster);
	MDagPathArray influencesDag;
	influencesCount = mfnSkin.influenceObjects(influencesDag);
	influences.resize(influencesCount);

	MPlug bindMatPlug = mfnDep.findPlug("bindPreMatrix");
	MStatus status;
	for (unsigned int x = 0; x < influencesCount; x++)
	{
		MObject val;
		MPlug curMat = bindMatPlug.elementByLogicalIndex(x, &status);
		curMat.getValue(val);
		//MDataHandle.asMatrix()
		if (status == MS::kSuccess)
		{
			MFnMatrixData matData(val);
			MMatrix mat = matData.matrix();
			influences[x] = ariInfluence(influencesDag[x].partialPathName(), mat);
		}
	}
	MPlug geoMat = mfnDep.findPlug("geomMatrix");
	MObject val;
	geoMat.getValue(val);
	MFnMatrixData matData(val);
	MMatrix mat = matData.matrix();
	mat.get(geoMatrix);

	MPlug inputPlug = mfnDep.findPlug("input", &status); //ReturnOnError(status);
	inputPlug = inputPlug.elementByLogicalIndex(0, &status); //ReturnOnError(status);
	MPlug inputGeoPlug = inputPlug.child(0, &status); //ReturnOnError(status);

	MObject lInputMeshObject = inputGeoPlug.asMObject();
	MFnMesh mfnMesh(lInputMeshObject);


	MFnSingleIndexedComponent MfnComp;
	MObject comp = MfnComp.create(MFn::kMeshVertComponent);
	MfnComp.setComplete(true);
	MDoubleArray weight;
	unsigned int inf;
	mfnSkin.getWeights(pGeo, comp, weight, inf);
	vertexCount = mfnMesh.numVertices();
	vertices.resize(vertexCount);
	MPointArray points;
	mfnMesh.getPoints(points);
	for (unsigned int v = 0; v < vertexCount; v++)
	{
		double pos[4];
		points[v].get(pos);
		vertices[v].position[0] = pos[0]; vertices[v].position[1] = pos[1]; vertices[v].position[2] = pos[2];
		for (unsigned int i = 0; i < inf; i++)
		{
			double w = weight[v * inf + i];
			if (w > 0.00001)
			{
				vertices[v].influences.push_back(i);
				vertices[v].weights.push_back(w);
			}
		}
	}
}

ariJson::ariSkinCluster::ariSkinCluster(const json &pJson)
{
	initFromData(pJson);
}

void ariJson::ariSkinCluster::setJsonData(json& pJson)
{
	pJson["name"] = name;
	pJson["maxInfluence"] = maxInfluences;
	pJson["influencesCount"] = influencesCount;
	pJson["geoMatrix"] = geoMatrix;
	pJson["verticesCount"] = vertexCount;

	for (unsigned int x = 0; x < influencesCount; x++)
	{
		json j;
		influences[x].setJsonData(j);
		pJson["influences"].push_back(j);
	}

	for (unsigned int v = 0; v < vertexCount; v++)
	{
		json j;
		vertices[v].setJsonData(j);
		pJson["vertices"].push_back(j);
	}
}

void ariJson::ariSkinCluster::initFromData(const json& pJson)
{
	name = pJson["name"].get<std::string>();
	influencesCount = pJson["influencesCount"].get<unsigned int>();
	maxInfluences = pJson["maxInfluence"].get<unsigned int>();
	for (unsigned int i = 0 ; i < 4; i++)
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			geoMatrix[i][j] = pJson["geoMatrix"][i][j];
		}
	}

	vertexCount = pJson["verticesCount"].get<unsigned int>();

	influences.resize(influencesCount);
	for (unsigned int i = 0; i < influencesCount; i++)
	{
		json j = pJson["influences"][i];
		influences[i].initFromData(j);
	}

	vertices.resize(vertexCount);
	for (unsigned int v = 0; v < vertexCount; v++)
	{
		json j = pJson["vertices"][v];
		vertices[v].initFromData(j);
	}

}

ariJson::ariVertex::ariVertex()
{

}

ariJson::ariVertex::~ariVertex()
{

}

void ariJson::ariVertex::setJsonData(json& pJson)
{
	pJson["p"] = position;
	pJson["i"] = influences;
	pJson["w"] = weights;
}

void ariJson::ariVertex::initFromData(json& pJson)
{
	position[0] = pJson["p"][0];
	position[1] = pJson["p"][1];
	position[2] = pJson["p"][2];

	size_t influ = pJson["i"].size();
	influences.resize(influ);
	weights.resize(influ);
	for (size_t x = 0; x < influ; x++)
	{
		influences[x] = pJson["i"][x];
		weights[x] = pJson["w"][x];
	}
}
