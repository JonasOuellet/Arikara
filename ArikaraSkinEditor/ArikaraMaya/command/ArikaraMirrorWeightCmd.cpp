#include "ArikaraMirrorWeightCmd.h"
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MFnComponent.h>
#include "../../GlobalDefine.h"
#include <maya/MGlobal.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MArgList.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnMatrixData.h>

#include <string>

#include "../include/MayaSkinFunction.h"
#include "../ArikaraOptions.h"

unsigned int ArikaraMirrorWeightCmd::m_defaultAxis = 0;
MString ArikaraMirrorWeightCmd::m_defaultFind = "Left";
MString ArikaraMirrorWeightCmd::m_defaultReplace = "Right";
double ArikaraMirrorWeightCmd::m_defaultBias = 0.05;


void loadData(ArikaraOption* data)
{
    json obj = data->data["ArikaraCommands"]["arikaraMirrorWeight"];
    if (!obj.is_null())
    {
        if (obj.count("axis")){
            ArikaraMirrorWeightCmd::m_defaultAxis = obj["axis"];
        }

        if (obj.count("find")){
            ArikaraMirrorWeightCmd::m_defaultFind = obj["find"].get<std::string>().c_str();
        }

        if (obj.count("replace")){
            ArikaraMirrorWeightCmd::m_defaultReplace = obj["replace"].get<std::string>().c_str();
        }

        if (obj.count("bias")){
            ArikaraMirrorWeightCmd::m_defaultBias = obj["bias"];
        }
    }
}

void saveData(ArikaraOption* data)
{
    json obj;
    obj["axis"] = ArikaraMirrorWeightCmd::m_defaultAxis;
    obj["find"] = ArikaraMirrorWeightCmd::m_defaultFind.asChar();
    obj["replace"] = ArikaraMirrorWeightCmd::m_defaultReplace.asChar();
    obj["bias"] = ArikaraMirrorWeightCmd::m_defaultBias;

    data->data["ArikaraCommands"]["arikaraMirrorWeight"] = obj;
}

ArikaraMirrorWeightCmd::ArikaraMirrorWeightCmd()
{

}

ArikaraMirrorWeightCmd::~ArikaraMirrorWeightCmd()
{

}

MStatus ArikaraMirrorWeightCmd::doIt(const MArgList& args)
{
	MSyntax mySyntax = newSyntax();
	MStatus status;

	mBias = m_defaultBias;
	mFind = m_defaultFind;
	mReplace = m_defaultReplace;
	mAxis = m_defaultAxis;

	MArgDatabase parsed(mySyntax, args);

	/*MSelectionList viewSel;
	MGlobal::getActiveSelectionList(viewSel);
	if (viewSel.length() == 0)
	{
		displayError("Nothing selected to mirror");
		return MS::kInvalidParameter;
	}
	//status = viewSel.getDagPath(0, mGeoDagPath, mComponent); ReturnOnError(status);
	if (!ArikaraSelection::GetSelectedObjectAndVertex(0, viewSel, mGeoDagPath, mComponent))
		return MS::kFailure;
	//}
	//Check for the component 
	MFnComponent comp(mComponent);
	if (comp.elementCount() == 0)
	{
		displayError("No Component Specified");
		return MS::kInvalidParameter;
	}*/

	MSelectionList objectSelectionList;
	parsed.getObjects(objectSelectionList);
	status = objectSelectionList.getDagPath(0, mGeoDagPath, mComponent);

	MFnSingleIndexedComponent MfnComp(mComponent);
	if (MfnComp.elementCount() == 0)
	{
		displayError("No Component Selected.");
		return MS::kInvalidParameter;
	}

	//and if the object as a skincluster;
	MItDependencyGraph dgIt(mGeoDagPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
	if (!dgIt.isDone())
	{
		mSkinCluster = dgIt.currentItem();
	}
	else
	{
		displayError(MString("Object \"") + mGeoDagPath.partialPathName() + MString("\" has no SkinCluster"));
		return MS::kFailure;
	}

	if (parsed.isFlagSet(kAMWFindFlag))
		mFind = parsed.flagArgumentString(kAMWFindFlag, 0, &status); ReturnOnError(status);

	if (parsed.isFlagSet(kAMWReplaceFlag))
		mReplace = parsed.flagArgumentString(kAMWReplaceFlag, 0, &status);  ReturnOnError(status);

	if (parsed.isFlagSet(kAMWAxisFlag))
		mAxis = parsed.flagArgumentInt(kAMWAxisFlag, 0, &status);  ReturnOnError(status);

	if (parsed.isFlagSet(kAMWBiasFlag))
		mBias = parsed.flagArgumentDouble(kAMWBiasFlag, 0, &status);  ReturnOnError(status);

	buildMirrorInfluencesArray();
	status = findMatchingComponent(); ReturnOnError(status);

	return redoIt();
}

MStatus ArikaraMirrorWeightCmd::redoIt()
{
	MFnSkinCluster lSkin(mSkinCluster);
	MStatus status = lSkin.setWeights(mGeoDagPath, mMirrorComponent, mInfluences, mNewValues, false, &mOldValues);
	return status;
}

MStatus ArikaraMirrorWeightCmd::undoIt()
{
	MFnSkinCluster lSkin(mSkinCluster);

	MDagPathArray influences;
	lSkin.influenceObjects(influences);

	MIntArray mBaseInfluences(influences.length());
	for (unsigned int x = 0; x < influences.length(); x++)
		mBaseInfluences[x] = x;

	return lSkin.setWeights(mGeoDagPath, mMirrorComponent, mBaseInfluences, mOldValues, false);
}

void* ArikaraMirrorWeightCmd::creator()
{
	return new ArikaraMirrorWeightCmd;
}


MSyntax ArikaraMirrorWeightCmd::newSyntax()
{
	MSyntax syntax;

	//Geometry
	//syntax.addArg(MSyntax::MArgType::kString);
	//Component
	//syntax.addArg(MSyntax::MArgType::kSelectionItem);

	syntax.setObjectType(MSyntax::MObjectFormat::kSelectionList, 1);
	syntax.useSelectionAsDefault(true);

	syntax.addFlag(kAMWFindFlag, kAMWFindFlagLong, MSyntax::kString);
	syntax.addFlag(kAMWReplaceFlag, kAMWReplaceFlagLong, MSyntax::kString);
	syntax.addFlag(kAMWAxisFlag, kAMWBiasFlagLong, MSyntax::kLong);
	syntax.addFlag(kAMWBiasFlag, kAMWBiasFlagLong, MSyntax::kDouble);

	return syntax;
}

void ArikaraMirrorWeightCmd::Init()
{
    ArikaraOption::TheOne()->saveData.addCallback(&saveData);
    ArikaraOption::TheOne()->loadData.addCallback(&loadData);
}

void ArikaraMirrorWeightCmd::buildMirrorInfluencesArray()
{
	MFnSkinCluster mfnSkin(mSkinCluster);

	MDagPathArray influences;
	mfnSkin.influenceObjects(influences);

	mInfluences.setLength(influences.length());

	for (unsigned int x = 0; x < influences.length(); x++)
	{
		MString currentInfluenceName = influences[x].partialPathName();
		MString mirrorInfluenceName(currentInfluenceName);
		mirrorInfluenceName.substitute(mFind, mReplace);
		if (currentInfluenceName == mirrorInfluenceName)
		{
			// probably a bone of the other side so swap it.
			mirrorInfluenceName.substitute(mReplace, mFind);
		}

		if (currentInfluenceName == mirrorInfluenceName)
		{
			//if the bone stay the same add it directly to the array
			mInfluences.set(x, x);
		}
		else
		{
			//find the new influences index;
			MDagPath mirrorInfluencesDagPath;
			MSelectionList selList;
			selList.clear(); selList.add(mirrorInfluenceName);
			selList.getDagPath(0, mirrorInfluencesDagPath);
			for (unsigned int y = 0; y < influences.length(); y++)
			{
				if (mirrorInfluencesDagPath == influences[y])
				{
					mInfluences.set(x, y);
					break;
				}
			}
		}
	}

}

MStatus ArikaraMirrorWeightCmd::findMatchingComponent()
{
	MStatus status;

	//Use the input mesh in the skincluster to match vertices;
	MFnDependencyNode mfnDep(mSkinCluster);
	MPlug inputPlug = mfnDep.findPlug("input", &status); ReturnOnError(status);
	inputPlug = inputPlug.elementByLogicalIndex(0, &status); ReturnOnError(status);
	MPlug inputGeoPlug = inputPlug.child(0, &status); ReturnOnError(status);

	MObject lInputMeshObject = inputGeoPlug.asMObject();
	MFnMesh mfnMesh(lInputMeshObject);
	//////////////////////////////////////////////////////////////////////////

	//Transform the mirror axis with the geometry orientation
	MPlug geoMatPlug = mfnDep.findPlug("geomMatrix", &status); ReturnOnError(status);
	MObject geoMatObject = geoMatPlug.asMObject();
	MFnMatrixData matData(geoMatObject);
	MMatrix geoMat(matData.matrix());
	//remove translation data in the matrix
	geoMat(3, 0) = 0.0; geoMat(3, 1) = 0.0; geoMat(3, 2) = 0.0;
	MVector lAxisVector(0.0, 0.0);
	lAxisVector[mAxis] = 1.0;
	lAxisVector = lAxisVector * geoMat;
	unsigned int lAxis = 2;
	if (lAxisVector[0] != 0.0)
		lAxis = 0;
	else if (lAxisVector[1] != 0.0)
		lAxis = 1;
	//////////////////////////////////////////////////////////////////////////


	MFnSingleIndexedComponent vIter(mComponent);

	//MItMeshVertex vIter(mGeoDagPath, mComponent);
	//MFnMesh mfnMesh(mGeoDagPath);

	unsigned int verticesCount = mfnMesh.numVertices();

	double smallnumber = 0.000001;

	MFnSingleIndexedComponent mirrorvertComp;
	mMirrorComponent = mirrorvertComp.create(MFn::Type::kMeshVertComponent, &status);

	MFnSingleIndexedComponent sourceVertComp;
	MObject sourceComponent = sourceVertComp.create(MFn::Type::kMeshVertComponent, &status);
	
	MIntArray targetArr;
	//
	// try getting input geometry in the skin cluster for matching vertex
	//
	MPointArray allPos;
	mfnMesh.getPoints(allPos, MSpace::kPostTransform);
	unsigned int unfoundVertex = 0;
	for (int x = 0; x < vIter.elementCount(); x++)
	{
		bool vertexFound = false;
		MPoint vertexPos = allPos[vIter.element(x)];
		double smallestdelta = 10000.0;
		unsigned int smallestIndex = 0;
		vertexPos[lAxis] *= -1;

		for (unsigned int v = 0; v < verticesCount; v++)
		{
			//MPoint pos = allPos[v];
			double dist = vertexPos.distanceTo(allPos[v]);
			if (dist <= smallnumber)
			{
				mirrorvertComp.addElement(v);
				sourceVertComp.addElement(vIter.element(x));
				targetArr.append(v);
				vertexFound = true;
				break;
			}
			if (dist < smallestdelta)
			{
				smallestdelta = dist;
				smallestIndex = v;
			}
		}

		if (!vertexFound)
		{
			if (smallestdelta <= mBias)
			{
				mirrorvertComp.addElement(smallestIndex);
				sourceVertComp.addElement(vIter.element(x));
				targetArr.append(smallestIndex);
			}
			else
			{
				/*std::string tmp = "No Mirror match for for vertex: ";
				tmp += std::to_string(vIter.index());
				displayWarning(tmp.c_str());*/
				unfoundVertex++;
			}
		}
	}

	if (sourceVertComp.elementCount() == 0 || mirrorvertComp.elementCount() == 0)
	{
		displayError("No Component matched to mirror.");
		return MS::kFailure;
	}

	if (unfoundVertex > 0)
	{
		std::string message(std::to_string(unfoundVertex));
		message += " unmatched vertices that won't be mirrored.";
		displayWarning(message.c_str());
	}

	MFnSkinCluster lSkinCluster(mSkinCluster);
	unsigned int influenceCount;
	lSkinCluster.getWeights(mGeoDagPath, sourceComponent, mOldValues, influenceCount);

	//Vertex order issue
	// force to sort the weight for the mirror vertex order.
	MIntArray sortedTargetArr(1, 0);
	for (unsigned int x = 1; x < targetArr.length(); x++)
	{
		bool bigger = true;
		for (unsigned int y = 0; y < sortedTargetArr.length(); y++)
		{
			if (targetArr[x] < targetArr[sortedTargetArr[y]])
			{
				sortedTargetArr.insert(x, y);
				bigger = false;
				break;
			}
		}
		if (bigger)
		{
			sortedTargetArr.append(x);
		}
	}


	mNewValues.setLength(mOldValues.length());
	unsigned int si = 0;
	unsigned int ti = 0;
	for (unsigned int x = 0; x < sortedTargetArr.length(); x++)
	{
		for (unsigned int i = 0; i < influenceCount; i++)
		{
			si = x * influenceCount + i;
			ti = sortedTargetArr[x] * influenceCount + i;
			mNewValues[si] = mOldValues[ti];
		}
	}
	mOldValues.clear();
	return MS::kSuccess;
}
