#include "ArikaraSkinDataCmd.h"

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
#include "../ArikaraOptions.h"
#include "../include/ArikaraSkinEditorCmd.h"

#define kFileExtension ".ars"


#include <maya/MThreadPool.h>
#include <maya/MTimer.h>

#define NUM_TASKS 64

typedef struct _taskDataTag
{
    ariSkinData::ariSkinCluster* ariSkin;
    MDoubleArray* weights;
    MPointArray* loadedPos;
    MPointArray* vertexPos;
    MIntArray* index;
    MIntArray* influIndex;
    unsigned int vertexCount;
    unsigned int influenceCount;
    double bias;
} taskData;

typedef struct _threadDataTag
{
    taskData* tdata;
    int start, end;
} threadData;

MThreadRetVal loadByPosTask(void *data)
{
    threadData *myData = (threadData *)data;
    taskData* tdata = myData->tdata;
    double bias = tdata->bias;
    unsigned int influenceCount = tdata->influenceCount;
    for (int v = myData->start; v < myData->end; v ++)
    {
        int curV = (*tdata->index)[v];

        double closestDist = 10000.0;
        unsigned int closestIndex = 0;
        MPoint curPos((*tdata->vertexPos)[curV]);
        for (unsigned int x = 0; x < tdata->loadedPos->length(); x++)
        {
            double dist = curPos.distanceTo((*tdata->loadedPos)[x]);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestIndex = x;
            }
            if (closestDist < bias)
                break;
        }

        for (unsigned int i = 0; i < (*tdata->ariSkin).vertices[closestIndex].count; i++)
        {
            int ci = (*tdata->influIndex)[(*tdata->ariSkin).vertices[closestIndex].influences[i]];
            (*tdata->weights)[v * influenceCount + ci] = (*tdata->ariSkin).vertices[closestIndex].weights[i];
        }

    }
    return (MThreadRetVal)0;
}

// Function to create thread tasks
void DecomposeLoadByPos(void *data, MThreadRootTask *root)
{
    taskData *taskD = (taskData *)data;
    threadData tdata[NUM_TASKS];
    unsigned int step = taskD->vertexCount / NUM_TASKS;
    if (step < 1)
        step = 1;
    unsigned int curStep = 0;
    for (int i = 0; i < NUM_TASKS; ++i)
    {
        tdata[i].tdata = taskD;

        tdata[i].start = curStep;
        if (i == NUM_TASKS - 1)
            tdata[i].end = taskD->vertexCount;
        else
            tdata[i].end = (curStep += step);

        MThreadPool::createTask(loadByPosTask, (void *)&tdata[i], root);

        // Check if selected vertices is smaller then thread count.
        if (curStep + step > taskD->vertexCount)
            break;
    }
    MThreadPool::executeAndJoin(root);
}

//List file in a folder c++
//https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c

MString ArikaraSkinDataCmd::lastPathName;
MString ArikaraSkinDataCmd::lastFileName;
MString ArikaraSkinDataCmd::lastPrefix;
MString ArikaraSkinDataCmd::lastSuffix;
MString ArikaraSkinDataCmd::defaultPath = ArikaraSkinDataCmd::GetBasePath();;
MString ArikaraSkinDataCmd::defaultPrefix = "";
MString ArikaraSkinDataCmd::defaultSuffix = "";
double ArikaraSkinDataCmd::defaultBias = 0.0001;

void loadOption(ArikaraOption* pData)
{
	json obj = pData->data["ArikaraCommands"]["arikaraSkinData"];

    if (!obj.is_null())
    {
        if (obj.count("defaultPath"))
        {
            ArikaraSkinDataCmd::defaultPath = obj["defaultPath"].get<std::string>().c_str();
        }
        if (obj.count("prefix"))
        {
            ArikaraSkinDataCmd::defaultPrefix = obj["prefix"].get<std::string>().c_str();
        }
        if (obj.count("suffix"))
        {
            ArikaraSkinDataCmd::defaultSuffix = obj["suffix"].get<std::string>().c_str();
        }
        if (obj.count("bias"))
        {
            ArikaraSkinDataCmd::defaultBias = obj["bias"].get<double>();
        }
	}
}

void saveOption(ArikaraOption* pData)
{
	json obj;

	obj["defaultPath"] = ArikaraSkinDataCmd::defaultPath.asChar();
	obj["prefix"] = ArikaraSkinDataCmd::defaultPrefix.asChar();
	obj["suffix"] = ArikaraSkinDataCmd::defaultSuffix.asChar();
    obj["bias"] = ArikaraSkinDataCmd::defaultBias;

	pData->data["ArikaraCommands"]["arikaraSkinData"] = obj;
}


ArikaraSkinDataCmd::ArikaraSkinDataCmd()
{
	m_SaveFlag = false;
	m_LoadFlag = false;
	m_isSkinClusterCreated = false;
	m_LoadBindMatrixFlag = false;
	m_CleanFlag = false;
    m_WorldSpaceFlag = false;
    m_LoadByPosFlag = false;
    m_bias = defaultBias;
}

ArikaraSkinDataCmd::~ArikaraSkinDataCmd()
{

}

MStatus ArikaraSkinDataCmd::doIt(const MArgList& args)
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
        m_LoadByPosFlag = parsed.isFlagSet(kARISDLoadByPosFlag);
        m_WorldSpaceFlag = parsed.isFlagSet(kARISDWorldSpaceFlag);
        
	}

    if (parsed.isFlagSet(kARISDPathFlag))
    {
        m_PathName = parsed.flagArgumentString(kARISDPathFlag, 0);
        lastPathName = m_PathName;
    }
    else
    {
        if (lastPathName.length() == 0)
            m_PathName = defaultPath;
        else
            m_PathName = lastPathName;
    }

    if (parsed.isFlagSet(kARISDFileFlag))
    {
        m_FileName = parsed.flagArgumentString(kARISDFileFlag, 0);
    }
    else
    {
        MDagPath trans = m_GeoDagPath;
        trans.pop();
        m_FileName = trans.partialPathName();
        m_FileName += kFileExtension;
    }

    if (parsed.isFlagSet(kARISDPrefixFlag))
    {
        m_Prefix = parsed.flagArgumentString(kARISDPrefixFlag, 0);
        lastPrefix = m_Prefix;
    }
    else
    {
        if (lastPrefix.length() == 0)
            m_Prefix = defaultPrefix;
        else
            m_Prefix = lastPrefix;
    }

    if (parsed.isFlagSet(kARISDSuffixFlag))
    {
        m_Suffix = parsed.flagArgumentString(kARISDSuffixFlag, 0);
        lastSuffix = m_Suffix;
    }
    else
    {
        if (lastSuffix.length() == 0)
            m_Suffix = defaultSuffix;
        else
            m_Suffix = lastSuffix;
    }

    if (parsed.isFlagSet(kARISDBiasFlag))
    {
        m_bias = parsed.flagArgumentDouble(kARISDBiasFlag, 0);
    }


	return redoIt();
}

MStatus ArikaraSkinDataCmd::redoIt()
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

MStatus ArikaraSkinDataCmd::undoIt()
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

void ArikaraSkinDataCmd::Init()
{
	ArikaraOption::TheOne()->saveData.addCallback(&saveOption);
	ArikaraOption::TheOne()->loadData.addCallback(&loadOption);
}

void* ArikaraSkinDataCmd::creator()
{
	return new ArikaraSkinDataCmd;
}

MSyntax ArikaraSkinDataCmd::newSyntax()
{
	MSyntax	syntax;
	syntax.setObjectType(MSyntax::MObjectFormat::kSelectionList, 1);
	syntax.useSelectionAsDefault(true);
	
	syntax.addFlag(kARISDSaveFlag, kARISDSaveFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDLoadFlag, kARISDLoadFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDBindMatFlag, kARISDBindMatFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDCleanSkinFlag, kARISDCleanSkinFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDLoadByPosFlag, kARISDLoadByPosFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDWorldSpaceFlag, kARISDWorldSpaceFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kARISDFileFlag, kARISDFileFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kARISDPathFlag, kARISDPathFlagLong, MSyntax::MArgType::kString);
    syntax.addFlag(kARISDPrefixFlag, kARISDPrefixFlagLong, MSyntax::MArgType::kString);
    syntax.addFlag(kARISDSuffixFlag, kARISDSuffixFlagLong, MSyntax::MArgType::kString);
    syntax.addFlag(kARISDBiasFlag, kARISDBiasFlagLong, MSyntax::MArgType::kDouble);
	return syntax;
}

MString ArikaraSkinDataCmd::GetBasePath()
{
	MString folder = ArikaraOption::GetArikaraFolder();
#ifdef WIN32
	folder += "\\";
#else
	folder += "/";
#endif // WIN32
	folder += "SkinData";
	MCommonSystemUtils::makeDirectory(folder);
	return folder;
}

MStatus ArikaraSkinDataCmd::saveSkinData()
{
    MString lFilePath = m_PathName;

#ifdef WIN32
    lFilePath += "\\";
#else
    lFilePath += "/";
#endif // WIN32

    lFilePath += m_FileName;

	ariSkinData::ariSkinCluster ariSkin(m_SkinCluster, m_GeoDagPath);
	if (ariSkin.writeToFile(lFilePath.asChar()))
	{
        displayInfo("Skin data successfully saved: " + lFilePath);
		return MS::kSuccess;
	}
    displayError("Could not save skin data: " + lFilePath);
	return MS::kFailure;
}

MStatus ArikaraSkinDataCmd::loadSkinData()
{
	MStatus status;

    MString lFilePath = m_PathName;

#ifdef WIN32
    lFilePath += "\\";
#else
    lFilePath += "/";
#endif // WIN32

    lFilePath += m_FileName;

	ariSkinData::ariSkinCluster ariSkin;

	if (!ariSkin.loadFromFile(lFilePath.asChar()))
	{
		displayError("Could not open specified file: " + lFilePath);
		return MS::kFailure;
	}

    displayInfo("Load skin data from file: " + lFilePath);

    MFnMesh mfnMesh(m_GeoDagPath);
    unsigned int vertexCount = static_cast<unsigned int>(mfnMesh.numVertices());
    if (vertexCount != ariSkin.vertexCount
        && m_LoadByPosFlag == false)
    {
        displayError("Current Geometry doesn't have the same vertex Count as the specified skinData.\n Must " 
            "specify \"-position\" flag to load weight by position instead of by vertex order.");
        return MS::kFailure;
    }

	MDoubleArray newWeights;
    unsigned int influenceCount;
    MIntArray influIndex;

    if (m_CleanFlag)
    {
        loadCleanSkinData(ariSkin, influIndex, influenceCount);
    }
    else
    {
        loadSkinData(ariSkin, influIndex, influenceCount);
    }

    m_InfluenceArr.setLength(influenceCount);
    for (unsigned int i = 0; i < influenceCount; i++)
    {
        m_InfluenceArr[i] = i;
    }

    MFnSingleIndexedComponent MfnComp(m_components);
    if (!MfnComp.isComplete())
    {
        vertexCount = MfnComp.elementCount();
    }
    else
    {
        MfnComp.setCompleteData(vertexCount);
    }

    newWeights = MDoubleArray(vertexCount * influenceCount, 0);

    if (m_LoadByPosFlag)
    {

#ifdef VERBAL
        MTimer timer;
        timer.beginTimer();
#endif // VERBAL

        ParalelLoadByPos(ariSkin, mfnMesh, newWeights, influIndex, influenceCount);

#ifdef VERBAL
        timer.endTimer();
        double parallelTime = timer.elapsedTime();

        MString info = "Parallel matching took: ";
        info += parallelTime;
        info += " using bias: ";
        info += m_bias;
        displayInfo(info);
#endif // VERBAL
    }
    else
    {
        for (unsigned int v = 0; v < vertexCount; v++)
        {
            int curVert = MfnComp.element(v);
            for (unsigned int i = 0; i < ariSkin.vertices[curVert].count; i++)
            {
                int ci = influIndex[ariSkin.vertices[curVert].influences[i]];
                newWeights[v * influenceCount + ci] = ariSkin.vertices[curVert].weights[i];
            }
        }
    }

	MFnSkinCluster mfnSkin(m_SkinCluster);
	MFnDependencyNode mfnDep(m_SkinCluster);

	if (m_LoadBindMatrixFlag)
	{
		MPlug bindMatPlug = mfnDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);
		MFnMatrixData matData;
		for (unsigned int x = 0; x < ariSkin.influencesCount; x++)
		{
			MSelectionList sel;
			sel.add(ariSkin.influences[x].name);
			MDagPath dag;
			sel.getDagPath(0, dag);
			unsigned int physicIndex = x;
			if (!m_isSkinClusterCreated)
				physicIndex = mfnSkin.indexForInfluenceObject(dag);

			MMatrix mat(ariSkin.influences[x].bindMat);
			MObject matObj = matData.create();
			matData.set(mat);
			MPlug curMat = bindMatPlug.elementByPhysicalIndex(physicIndex, &status); // .setValue(matObj);
			m_MDGModMatrix.newPlugValue(curMat, matObj);
		}
	}
	m_MDGModMatrix.newPlugValueInt(mfnDep.findPlug("mi", &status), ariSkin.maxInfluences);
	m_MDGModMatrix.doIt();

	status = mfnSkin.setWeights(m_GeoDagPath, m_components, m_InfluenceArr, newWeights, false, &m_oldWeights);

    /*Select the new loaded mesh.
    */
    MDagPath trans = m_GeoDagPath;
    trans.pop();
    MGlobal::select(trans.node(), MGlobal::kReplaceList);

    displayInfo("Skin data successfully loaded");

	return MS::kSuccess;
}

void ArikaraSkinDataCmd::loadSkinData(ariSkinData::ariSkinCluster& skinJson, MIntArray &influIndex, unsigned int& totalNewInfluences)
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
		MString curName = m_Prefix + skinJson.influences[i].name + m_Suffix;
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


void ArikaraSkinDataCmd::ParalelLoadByPos(ariSkinData::ariSkinCluster& ariSkin, MFnMesh& mfnMesh, 
    MDoubleArray& newWeights, MIntArray& influIndex, unsigned int influenceCount)
{
    MMatrix geoMat(ariSkin.geoMatrix);

    MPointArray loadedPts(ariSkin.vertexCount);
    for (unsigned int x = 0; x < ariSkin.vertexCount; x++)
    {
        double* pos = ariSkin.vertices[x].position;
        loadedPts[x].x = pos[0];
        loadedPts[x].y = pos[1];
        loadedPts[x].z = pos[2];

        if (m_WorldSpaceFlag)
        {
            loadedPts[x] *= geoMat;
        }
    }

    MFnSingleIndexedComponent mfnComp(m_components);
    MIntArray indexArray(mfnComp.elementCount());
    for (int x = 0; x < mfnComp.elementCount(); x++)
    {
        indexArray[x] = mfnComp.element(x);
    }

    MPointArray vertexPos;

    if (m_WorldSpaceFlag)
    {
        mfnMesh.getPoints(vertexPos, MSpace::Space::kWorld);
    }
    else
    {
        mfnMesh.getPoints(vertexPos);
    }

    MStatus stat = MThreadPool::init();
    if (stat != MS::kSuccess)
    {
        MString str = MString("Error creating threadpool");
        displayError(str);
        return;
    }

    taskData tdata;
    tdata.ariSkin = &ariSkin;
    tdata.index = &indexArray;
    tdata.influenceCount = influenceCount;
    tdata.influIndex = &influIndex;
    tdata.loadedPos = &loadedPts;
    tdata.vertexCount = mfnComp.elementCount();
    tdata.vertexPos = &vertexPos;
    tdata.weights = &newWeights;
    tdata.bias = m_bias;

    MThreadPool::newParallelRegion(DecomposeLoadByPos, (void*)&tdata);
    MThreadPool::release();
    MThreadPool::release();
}

void ArikaraSkinDataCmd::loadCleanSkinData(ariSkinData::ariSkinCluster& skinJson, MIntArray &influIndex, unsigned int& totalNewInfluences)
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

	for (unsigned int i = 0; i < skinJson.influencesCount; i++)
	{
        MString name = m_Prefix + skinJson.influences[i].name + m_Suffix;
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
		}

		command += " " + MString(name);
	}

	m_MDagModifier.doIt();

	status = m_MDGModifier.commandToExecute(command);
	m_MDGModifier.doIt();

	//reset the new skincluster and rename it
	MItDependencyGraph dgIt(m_GeoDagPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
	if (!dgIt.isDone())
	{
		m_SkinCluster = dgIt.currentItem();
		MFnDependencyNode tmp(m_SkinCluster);
		tmp.setName(skinJson.name);
	}

    totalNewInfluences = skinJson.influencesCount;
    influIndex.setLength(skinJson.influencesCount);
    for (unsigned int x = 0; x < totalNewInfluences; x++)
    {
        influIndex[x] = x;
    }
}

/*
class used to save/load data;
*/
ariSkinData::ariSkinCluster::ariSkinCluster(MObject &pSkinCluster, MDagPath& pGeo)
{
	MFnDependencyNode mfnDep(pSkinCluster);
	name = mfnDep.name().asChar();
	maxInfluences = mfnDep.findPlug("mi").asInt();

	MFnSkinCluster mfnSkin(pSkinCluster);
	MDagPathArray influencesDag;
	influencesCount = mfnSkin.influenceObjects(influencesDag);
	influences = new ariInfluence[influencesCount];

	MPlug bindMatPlug = mfnDep.findPlug("bindPreMatrix");
	MStatus status;
	for (unsigned int x = 0; x < influencesCount; x++)
	{
		MObject val;
		MPlug curMat = bindMatPlug.elementByLogicalIndex(mfnSkin.indexForInfluenceObject(influencesDag[x]), &status);
		curMat.getValue(val);
		//MDataHandle.asMatrix()
		if (status == MS::kSuccess)
		{
			MFnMatrixData matData(val);
			MMatrix mat = matData.matrix();
			MString tmpMString = influencesDag[x].partialPathName();
			const char* tmp = tmpMString.asChar();
			size_t len = strlen(tmp)+1;
			influences[x].name = new char[len];
			memcpy((void*)influences[x].name, tmp, len);
			mat.get(influences[x].bindMat);
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
	vertices = new ariVertex[vertexCount];
	MPointArray points;
	mfnMesh.getPoints(points);
	for (unsigned int v = 0; v < vertexCount; v++)
	{
		double pos[4];
		points[v].get(pos);
		vertices[v].position[0] = pos[0]; vertices[v].position[1] = pos[1]; vertices[v].position[2] = pos[2];

		std::vector<unsigned int> tmpInfl;
		std::vector<double> tmpWeight;
		for (unsigned int i = 0; i < inf; i++)
		{
			double w = weight[v * inf + i];
			if (w > 0.00001)
			{
				vertices[v].count++;
				tmpInfl.push_back(i);
				tmpWeight.push_back(w);
			}
		}
		vertices[v].influences = new unsigned int[vertices[v].count];
		vertices[v].weights = new double[vertices[v].count];

		for (unsigned int x = 0; x < vertices[v].count; x++)
		{
			vertices[v].influences[x] = tmpInfl[x];
			vertices[v].weights[x] = tmpWeight[x];
		}
	}
}

bool ariSkinData::ariSkinCluster::writeToFile(const char* pFilePath) const
{
	std::ofstream o;
	o.open(pFilePath, std::ios::out | std::ios::binary);
	if (!o.good())
		return false;

	size_t n = strlen(name) + 1;
	o.write((char*)&n, sizeof(size_t));
	o.write((char*)name, n);

	o.write((char*)&maxInfluences, sizeof(maxInfluences));
	o.write((char*)&vertexCount, sizeof(vertexCount));
	o.write((char*)&influencesCount, sizeof(influencesCount));
	o.write((char*)geoMatrix, sizeof(geoMatrix));

	for (unsigned int i = 0; i < influencesCount; i++)
	{
		size_t tmp = strlen(influences[i].name)+1;
		o.write((char*)&tmp, sizeof(size_t));
		o.write((char*)influences[i].name, tmp);
		o.write((char*)influences[i].bindMat, sizeof(double[4][4]));
	}

	for (unsigned int v = 0; v < vertexCount; v++)
	{
		o.write((char*)vertices[v].position, sizeof(double[3]));
		unsigned int count = vertices[v].count;
		o.write((char*)&count, sizeof(unsigned int));
		o.write((char*)vertices[v].influences, sizeof(unsigned int) * count);
		o.write((char*)vertices[v].weights, sizeof(double) * count);
	}

	o.close();
	return true;
}

bool ariSkinData::ariSkinCluster::loadFromFile(const char* pFilePath)
{
	std::ifstream i;
	i.open(pFilePath, std::ios::in | std::ios::binary);
	if (!i.good())
		return false;
	{
		size_t len;
		i.read((char*)&len, sizeof(size_t));
		name = new char[len];
		i.read((char*)name, len);
	}

	i.read((char*)&maxInfluences, sizeof(maxInfluences));
	i.read((char*)&vertexCount, sizeof(vertexCount));
	i.read((char*)&influencesCount, sizeof(influencesCount));
	i.read((char*)geoMatrix, sizeof(geoMatrix));

	influences = new ariInfluence[influencesCount];
	vertices = new ariVertex[vertexCount];

	for (unsigned int x = 0; x < influencesCount; x++)
	{
		size_t n;
		i.read((char*)&n, sizeof(size_t));
        influences[x].name = new char[n];

		i.read((char*)influences[x].name, n);
		i.read((char*)influences[x].bindMat, sizeof(double[4][4]));
	}

	for (unsigned int v = 0; v < vertexCount; v++)
	{
		i.read((char*)vertices[v].position, sizeof(double[3]));
		unsigned int count;
		i.read((char*)&count, sizeof(unsigned int));
		vertices[v].count = count;

		vertices[v].influences = new unsigned int[count];
		vertices[v].weights = new double[count];

		i.read((char*)vertices[v].influences, sizeof(unsigned int) * count);
		i.read((char*)vertices[v].weights, sizeof(double) * count);
	}

	i.close();
	return true;
}
