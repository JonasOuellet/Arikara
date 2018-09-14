#include "../include/MayaSkinFunction.h"
#include <maya/MFnSkinCluster.h>
#include <maya/MRichSelection.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MDGModifier.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include <maya/MDataHandle.h>
#include <maya/MDGContext.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MDGModifier.h>
#include <maya/MItMeshEdge.h>

#include "../../GlobalDefine.h"

bool ArikaraSkinUtils::hasSkinCluster(MDagPath& pObject, MObject &pSkin)
{
	MStatus status;
	MObject geomNode = pObject.node();
	MItDependencyGraph dgIt(geomNode, MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
	if (!dgIt.isDone())
	{
		pSkin = dgIt.currentItem();
		return true;
	}
	return false;
}


bool ArikaraSkinUtils::getObjectWithSkinFromSelection(MDagPathArray& pDagPathArray)
{
	MSelectionList sel;
	MGlobal::getActiveSelectionList(sel);

	pDagPathArray.clear();

	MStatus status;
	for (unsigned int x = 0; x < sel.length(); x++)
	{
		MDagPath dagPath;
		MObject  component;
		status = sel.getDagPath(x, dagPath, component);
		MDagPath transDag = dagPath;
		if (component.isNull())
		{
			dagPath.extendToShape();
		}
		MObject tmpSkin;
		if (hasSkinCluster(dagPath, tmpSkin))
		{
			pDagPathArray.append(transDag);
		}
	}

	return pDagPathArray.length() > 0;
}

bool ArikaraSkinUtils::GetFirstObjectWithSkinFromSelection(MDagPath& pObject, MObject &pSkin)
{
	MSelectionList sel;
	MGlobal::getActiveSelectionList(sel);

	MStatus status;
	for (unsigned int x = 0; x < sel.length(); x++)
	{
		MDagPath dagPath;
		MObject  component;
		status = sel.getDagPath(x, dagPath, component);
		MDagPath transDag = dagPath;
		if (component.isNull())
		{
			dagPath.extendToShape();
		}
		else
		{
			transDag.pop();
		}

		if (hasSkinCluster(dagPath, pSkin))
		{
			pObject = transDag;
			return true;
		}
	}
	return false;
}

void ArikaraSkinUtils::SaveSkin(MDagPath& pGeo)
{


}

void ArikaraSkinUtils::LoadSkin()
{

}

unsigned int ArikaraSkinUtils::getInfluencesNames(const MObject &pSkinCluster, std::vector<std::string> &influenceNames)
{
	MFnSkinCluster mfnSkin;
	MStatus status = mfnSkin.setObject(pSkinCluster);
	if (status != MStatus::kSuccess)
	{
		MGlobal::displayWarning("invalid mobject");
		return 0;
	}

	MDagPathArray influences;
	unsigned int influencesCount = mfnSkin.influenceObjects(influences);

	influenceNames.resize(influencesCount);

	for (unsigned int x = 0; x < influencesCount; x++)
	{
		MGlobal::displayInfo(influences[x].partialPathName().asChar());
		influenceNames[x] = influences[x].partialPathName().asChar();
	}

	return influencesCount;
}

void ArikaraSkinUtils::isInfluencesLocked(const MObject &pSkinCluster, std::vector<bool>& locked)
{
	MStatus status;

	MFnSkinCluster mfnSkin(pSkinCluster);
	MDagPathArray influences;
	unsigned int influencesCount = mfnSkin.influenceObjects(influences);

	locked.resize(influencesCount, false);
	for (unsigned int x = 0; x < influencesCount; x++)
	{
		MFnDependencyNode mfnDep(influences[x].node());
		MObject lockAtt = mfnDep.attribute("liw", &status);
		if (!lockAtt.isNull() && status == MS::kSuccess)
		{
			MPlug attrPlug(influences[x].node(), lockAtt);
			locked[x] = attrPlug.asBool();
		}
	}
}

MStatus ArikaraSkinUtils::getInfluenceIndex(const MObject &pSkinCluster, const MString& name, unsigned int* index)
{
	//get the index of the joint
	MDagPath jointDagPath;
	MObject jointObj;
	MSelectionList selList;
	selList.clear(); selList.add(name);
	selList.getDagPath(0, jointDagPath, jointObj);

	// Check in all the influence object in the skin cluster to find the same dagpath
	MDagPathArray influences;
	MFnSkinCluster pMFnSkinCluster(pSkinCluster);
	pMFnSkinCluster.influenceObjects(influences);
	for (unsigned int x = 0; x < influences.length(); x++)
	{
		if (jointDagPath == influences[x])
		{
			*index = x;
			return MStatus::kSuccess;
		}
	}

	return MStatus::kFailure;
}


void ArikaraSkinUtils::lockInfluence(const MDagPath& pInfluences, bool lockStatus /*= true*/)
{
	MStatus status;
	MFnDependencyNode mfnDep(pInfluences.node());
	MObject lockAtt = mfnDep.attribute("liw", &status);
	if (!lockAtt.isNull() && status == MS::kSuccess)
	{
		MPlug attrPlug(pInfluences.node(), lockAtt);
		if (!attrPlug.isNull())
		{
			attrPlug.setBool(lockStatus);
		}
	}
}

int ArikaraSkinUtils::getMaxInfluence(const MObject &pSkinCluster)
{
	MStatus status;
	MFnDependencyNode mfnDep(pSkinCluster);
	MObject maxInflu = mfnDep.attribute("mi", &status);
	if (!maxInflu.isNull() && status == MS::kSuccess)
	{
		MPlug attrPlug(pSkinCluster, maxInflu);
		if (!attrPlug.isNull())
		{
			return attrPlug.asInt();
		}
	}
	return 0;
}

void ArikaraSkinUtils::setMaxInfluence(const MObject &pSkinCluster, int maxInfluence)
{
	/*MStatus status;
	MFnDependencyNode mfnDep(pSkinCluster);
	MObject maxInflu = mfnDep.attribute("mi", &status);
	if (!maxInflu.isNull() && status == MS::kSuccess)
	{
		MPlug attrPlug(pSkinCluster, maxInflu);
		if (!attrPlug.isNull())
		{
			attrPlug.setInt(maxInfluence);
		}
	}*/
	MFnDependencyNode mfnDep(pSkinCluster);
	MString maxInf(std::to_string(maxInfluence).c_str());
	MString command = "arikaraInfluence - mi " + maxInf + " " + mfnDep.name();
	MGlobal::executeCommand(command, true, true);
}

void ArikaraSkinUtils::resetBindPose(const MObject &pSkinCluster, MMatrixArray* oldPose /*= nullptr*/)
{
	if (pSkinCluster.isNull())
		return;

	MStatus status;

	MFnSkinCluster mfnSkin(pSkinCluster);
	MDagPathArray influences;
	unsigned int influencesCount = mfnSkin.influenceObjects(influences);
	
	MFnDependencyNode mfnDep(pSkinCluster);
	MPlug bindMatPlug = mfnDep.findPlug("bindPreMatrix", &status);

	if (status != MS::kSuccess)
		return;

	if (oldPose != nullptr)
	{
		oldPose->setLength(influencesCount);
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
				oldPose->set(mat, x);
				/*std::string message;
				for (unsigned int i = 0; i < 4; i++)
				{
					for (unsigned int j = 0; j < 4; j++)
					{
						message += std::to_string(mat(i, j)) + " ";
					}
				}
				MGlobal::displayInfo(message.c_str());*/
			}
		}
	}
	//MGlobal::displayInfo("---------------------------------------------");
	//MDGContext context(MAnimControl::currentTime());
	for (unsigned int x = 0; x < influencesCount; x++)
	{
		MDagPath curInfl = influences[x];
		MFnDependencyNode mfnDep(curInfl.node());

		//MGlobal::displayInfo(curInfl.partialPathName());

		MPlug bindMat = mfnDep.findPlug("worldInverseMatrix", &status);
		MMatrix bindInvMat;
		MFnMatrixData matData;
		if (status != MS::kSuccess)
		{
			//MGlobal::displayInfo("Cannot Find world InverseMatrix");
			continue;
		}
		MPlug matplug = bindMat.elementByLogicalIndex(0, &status);
		if (status == MS::kSuccess)
		{
			/*MDataHandle datahandle = matplug.asMDataHandle();
			bindInvMat = datahandle.asMatrix();
			matData.set(bindInvMat);*/
			MObject val;
			matplug.getValue(val);
			matData.setObject(val);
			bindInvMat = matData.matrix();
			//MGlobal::displayInfo("PhysicalIndex");
		}
		else
		{
			MObject val;
			bindMat.getValue(val);
			matData.setObject(val);
			bindInvMat = matData.matrix();
		}
		unsigned int physicIndex = mfnSkin.indexForInfluenceObject(curInfl);
		bindMatPlug.elementByPhysicalIndex(physicIndex, &status).setValue(matData.object());

		/*std::string message;
		for (unsigned int i = 0; i < 4; i++)
		{
			for (unsigned int j = 0; j < 4; j++)
			{
				message += std::to_string(bindInvMat(i, j)) + " ";
			}
		}
		MGlobal::displayInfo(message.c_str());*/
	}
}

bool ArikaraSkinUtils::transferWeight(const MDagPath &pObject, const MObject &pComp, const MObject &pSkinCluster, const MDagPath &pSourceInfluence, const MDagPath &pTargetInfluence, MDoubleArray *pOldWeight /*= nullptr*/, double pValue /*= 1.0f*/)
{
	MFnSkinCluster mfnSkin(pSkinCluster);

	MStatus status;
	unsigned int sourceIndex = mfnSkin.indexForInfluenceObject(pSourceInfluence, &status); if (status != MS::kSuccess) return false;
	unsigned int targetIndex = mfnSkin.indexForInfluenceObject(pTargetInfluence, &status); if (status != MS::kSuccess) return false;
	MIntArray influencesArr;
	influencesArr.append(sourceIndex);
	influencesArr.append(targetIndex);

	MDoubleArray weights;
	unsigned int influenceCount;
	status = mfnSkin.getWeights(pObject, pComp, weights, influenceCount); if (status != MS::kSuccess) return false;

	unsigned int vertCount = weights.length() / influenceCount;
	MDoubleArray newWeight(vertCount * 2, 0.0);
	for (unsigned int x = 0; x < vertCount; x++)
	{
		int sI = x * influenceCount + sourceIndex;
		int tI = x * influenceCount + targetIndex;
		double sourceWeight = weights[sI] * pValue;
		newWeight[x * 2] = weights[sI] - sourceWeight;
		newWeight[x * 2 + 1] = weights[tI] + sourceWeight;
	}


	status = mfnSkin.setWeights(pObject, pComp, influencesArr, newWeight, false, pOldWeight);


	return true;
}

bool ArikaraSkinUtils::transferWeightForSelectedObject(const char* pSourceInfluenceName, const char* pTargetInfluenceName, double pValue /*= 1.0f*/)
{
	MSelectionList sel;
	MGlobal::getActiveSelectionList(sel);

	MStatus status;

	MDagPath sourceInfluence;
	MDagPath targetInfluence;

	MSelectionList influenceSel;
	status = influenceSel.add(pSourceInfluenceName); if (status != MS::kSuccess) return false;
	status = influenceSel.add(pTargetInfluenceName); if (status != MS::kSuccess) return false;

	status = influenceSel.getDagPath(0, sourceInfluence); if (status != MS::kSuccess) return false;
	status = influenceSel.getDagPath(1, targetInfluence); if (status != MS::kSuccess) return false;

	for (unsigned int x = 0; x < sel.length(); x++)
	{
		MDagPath dagPath;
		MObject  component;
		MObject vertComp;
		MObject skinCluster;
		status = sel.getDagPath(x, dagPath, component);
		// If component is null get the shape
		if (component.isNull())
			dagPath.extendToShape();

		if (!hasSkinCluster(dagPath, skinCluster))
			continue;
		
		if (component.isNull())
		{
			//set the complete vertex data to transfer the weights
			// on all vertices.
			MFnSingleIndexedComponent mfncomp;
			vertComp = mfncomp.create(MFn::Type::kMeshVertComponent);
			mfncomp.setComplete(true);
			
		}
		else
		{
			// if selected component is face
			// convert it to vertex;
			MFnComponent mfncomp(component);
			if (mfncomp.componentType() == MFn::Type::kMeshPolygonComponent)
			{
				ArikaraSelection::ConvertFacesToVertex(dagPath, component, vertComp);
			}
			else if (mfncomp.componentType() == MFn::Type::kMeshEdgeComponent)
			{
				continue;
			}
			else if (mfncomp.componentType() == MFn::Type::kMeshVertComponent)
			{
				vertComp = component;
			}
		}

		transferWeight(dagPath, vertComp, skinCluster, sourceInfluence, targetInfluence, nullptr, pValue);
	}
	return true;
}


bool ArikaraSkinUtils::getInfluenceLogicalIndex(const MObject& p_Skincluster, MIntArray& p_LogicalIndex)
{
    MStatus status;
    MFnDependencyNode mfnSkin(p_Skincluster);
    MPlug matrixPlug = mfnSkin.findPlug("matrix", &status); ReturnOnErrorBool(status);

    unsigned int numElem = matrixPlug.numElements(&status); ReturnOnErrorBool(status);
    p_LogicalIndex.setLength(numElem);
    for (unsigned int x = 0; x < numElem; x++)
    {
        p_LogicalIndex[x] = matrixPlug[x].logicalIndex();
    }

    return true;
}

bool ArikaraSkinUtils::setWeights(const MObject& p_Skincluster, const MDoubleArray& p_Weights, MDGModifier& p_DGModifier)
{
    MStatus status;
    MFnDependencyNode mfnSkin(p_Skincluster);

    MPlug matrixPlug = mfnSkin.findPlug("matrix", &status); ReturnOnErrorBool(status);
    unsigned int numInfluences = matrixPlug.numElements(&status); ReturnOnErrorBool(status);

    MPlug verticesWeightPlug = mfnSkin.findPlug("weightList", &status); ReturnOnErrorBool(status);
    unsigned int verticesCount = verticesWeightPlug.numElements();
    if (p_Weights.length() != numInfluences * verticesCount)
        return false;
    
    MIntArray logicalIndex;
    getInfluenceLogicalIndex(p_Skincluster, logicalIndex);
    
    for (unsigned int v = 0; v < verticesCount; v++)
    {
        MPlug weightPlug = verticesWeightPlug[v].child(0);
        //MPlugArray arr = weightPlug.array()
        //weightPlug.
        for (unsigned int i = 0; i < numInfluences; i++)
        {
            MPlug cur = weightPlug.elementByLogicalIndex(logicalIndex[i]);
            cur.setDouble(p_Weights[v * numInfluences + i]);
            /*double nw = p_Weights[v * numInfluences + i];
            double ow = cur.asDouble();
            if (nw != ow)
            {
                p_DGModifier.newPlugValueDouble(cur, p_Weights[v * numInfluences + i]);
            }*/
        }
    }
    return true;
}

bool ArikaraSkinUtils::getSkinMaxInfluence(const MObject& skinObject, int& maxInfluence)
{
    MFnDependencyNode mfnDep(skinObject);
    maxInfluence = mfnDep.findPlug("mi").asInt();
    return mfnDep.findPlug("maintainMaxInfluences").asBool();
}

void ArikaraSkinUtils::lockInfluence(const MObject &pSkinCluster, const std::vector<unsigned int> &index, bool lockStatus /*= true*/)
{
	MFnSkinCluster mfnSkin(pSkinCluster);
	MDagPathArray influences;
	unsigned int influencesCount = mfnSkin.influenceObjects(influences);
	for (unsigned int i = 0; i < index.size(); i++)
	{
		if (index[i] < influencesCount)
		{
			lockInfluence(influences[index[i]], lockStatus);
		}
	}
}

void ArikaraSkinUtils::lockInfluence(const MObject &pSkinCluster, unsigned int index, bool lockStatus /*= true*/)
{
	MFnSkinCluster mfnSkin(pSkinCluster);
	MDagPathArray influences;
	unsigned int influencesCount = mfnSkin.influenceObjects(influences);
	if (index < influencesCount)
	{
		lockInfluence(influences[index], lockStatus);
	}
}

bool ArikaraSelection::GetSoftSelectedVertexForObject(const MDagPath &pObject, MObject &pComponent, unsigned int *vertexCount, std::vector<float> *weights)
{
	//https://groups.google.com/forum/#!topic/python_inside_maya/q1JlddKybyM

	MRichSelection richSelection;
	MSelectionList selection;
	MGlobal::getRichSelection(richSelection);
    richSelection.getSelection(selection);

	MStatus status;

    for (unsigned int x = 0; x < selection.length(); x++)
    {
        MDagPath tmpDag;
        MObject tmpComp;

        selection.getDagPath(x, tmpDag, tmpComp);

        if (tmpDag == pObject)
        {
            MFnComponent mfnComp(tmpComp);
            MFn::Type type = mfnComp.type();

            int numComp = mfnComp.elementCount();
            
            if (numComp <= 0)
                return false;

            if (type == MFn::Type::kMeshVertComponent)
            {
                pComponent = tmpComp;
                if (vertexCount)
                    *vertexCount = numComp;
                if (weights)
                {
                    if (mfnComp.hasWeights())
                    {
                        weights->resize(numComp);
                        for (int v = 0; v < numComp; v++)
                        {
                            (*weights)[v] = mfnComp.weight(v).influence();
                        }
                    }
                    else
                    {
                        if (weights)
                        {
                            weights->clear();
                            weights->resize(numComp, 1.0f);
                        }
                    }
                }
                return true;
            }
            else if (type == MFn::Type::kMeshEdgeComponent)
            {
                unsigned int tmpver = ArikaraSelection::ConvertEdgesToVertex(tmpDag, tmpComp, pComponent);
                if (vertexCount)
                    *vertexCount = tmpver;
                if (weights)
                {
                    weights->clear();
                    weights->resize(tmpver, 1.0f);
                }
                return true;
            }
            else if (type == MFn::Type::kMeshPolygonComponent)
            {
                unsigned int tmpver = ArikaraSelection::ConvertFacesToVertex(tmpDag, tmpComp, pComponent);
                if (vertexCount)
                    *vertexCount = tmpver;
                if (weights)
                {
                    weights->clear();
                    weights->resize(tmpver, 1.0f);
                }
                return true;
            }
            return false;
        }
    }
    return false;
}

bool ArikaraSelection::GetSelectedVertexForObject(const MDagPath &pObject, MObject &pComponent, unsigned int *vertexCount)
{
	MSelectionList sel;
	MGlobal::getActiveSelectionList(sel);

	for (unsigned int x = 0; x < sel.length(); x++)
	{
		MDagPath tmpDag;
		MObject  tmpComp;
		sel.getDagPath(x, tmpDag, tmpComp);

		if (pObject == tmpDag)
		{
			MFnComponent mfncomp(tmpComp);
			if (mfncomp.elementCount() > 0)
            {
				if (mfncomp.componentType() == MFn::Type::kMeshPolygonComponent)
				{
                    unsigned int tmpver = ArikaraSelection::ConvertFacesToVertex(tmpDag, tmpComp, pComponent);
                    if (vertexCount)
                        *vertexCount = tmpver;
                    return true;
				}
				else if (mfncomp.componentType() == MFn::Type::kMeshEdgeComponent)
				{
                    unsigned int tmpver = ArikaraSelection::ConvertEdgesToVertex(tmpDag, tmpComp, pComponent);
                    if (vertexCount)
                        *vertexCount = tmpver;
					return true;
				}
				else if (mfncomp.componentType() == MFn::Type::kMeshVertComponent)
				{
                    pComponent = tmpComp;
                    if (vertexCount)
                        *vertexCount = mfncomp.elementCount();
                    return true;
				}
                else
                {
                    return false;
                }
			}
			return false;
		}
	}
	return false;
}

bool ArikaraSelection::GetSelectedObjectAndVertex(unsigned int index, const MSelectionList &sel, MDagPath& pObject, MObject &pComponent)
{
	MStatus status;
	MObject  tmpComp;
	status = sel.getDagPath(index, pObject, tmpComp); ReturnOnErrorBool(status);

	MFnComponent mfnComp(tmpComp);
	if (mfnComp.elementCount() > 0)
	{
		if (mfnComp.componentType() == MFn::Type::kMeshPolygonComponent)
		{
			ArikaraSelection::ConvertFacesToVertex(pObject, tmpComp, pComponent);
			return true;
		}
		else if (mfnComp.componentType() == MFn::Type::kMeshEdgeComponent)
		{
            ArikaraSelection::ConvertEdgesToVertex(pObject, tmpComp, pComponent);

		}
		else if (mfnComp.componentType() == MFn::Type::kMeshVertComponent)
		{
			pComponent = tmpComp;
			return true;
		}
	}
	return false;
}

bool ArikaraSelection::GetSelectedFacesForObject(const MDagPath &pObject, MObject &pComponent)
{
	MSelectionList sel;
	MGlobal::getActiveSelectionList(sel);

	for (unsigned int x = 0; x < sel.length(); x++)
	{
		MDagPath tmpDag;
		MObject  tmpComp;
		sel.getDagPath(x, tmpDag, tmpComp);

		if (pObject == tmpDag)
		{
			MFnComponent mfnComp(tmpComp);
			if (mfnComp.elementCount() > 0)
			{
				pComponent = tmpComp;
				return true;
			}
			return false;
		}
	}
	return false;
}

unsigned int ArikaraSelection::ConvertFacesToVertex(const MDagPath &pObject, const MObject &pFaceComponent, MObject &pVertexComponent)
{
	MObject tmp = pFaceComponent;
	MItMeshPolygon faceIter(pObject, tmp);

	MFnSingleIndexedComponent newIndex;
	pVertexComponent = newIndex.create(MFn::Type::kMeshVertComponent);
	for (; !faceIter.isDone(); faceIter.next())
	{
		MIntArray vertIndex;
		MStatus status = faceIter.getVertices(vertIndex);
		newIndex.addElements(vertIndex);
	}
	return newIndex.elementCount();
}

unsigned int ArikaraSelection::ConvertEdgesToVertex(const MDagPath &pObject, const MObject &pEdgeComponent, MObject &pVertexComponent)
{
    MObject tmp = pEdgeComponent;
    MItMeshEdge edgeIter(pObject, tmp);

    MFnSingleIndexedComponent newIndex;
    pVertexComponent = newIndex.create(MFn::Type::kMeshVertComponent);
    for (; !edgeIter.isDone(); edgeIter.next())
    {
        newIndex.addElement(edgeIter.index(0));
        newIndex.addElement(edgeIter.index(1));
    }
    return newIndex.elementCount();
}

const char* ArikaraSelection::GetSelectedObjectName()
{
	MSelectionList sel;
	MGlobal::getActiveSelectionList(sel);

	MDagPath obj;
	if (sel.length() > 0)
	{
		sel.getDagPath(0, obj);
		return obj.partialPathName().asChar();
	}
	return "";
}

bool ArikaraSelection::ConvertComponentToVertex(const MDagPath &pObject, const MObject &pCurrentComponent, MObject &pVertexComponent, unsigned int * vertexCount /*= nullptr*/)
{
    MFnComponent mfnComp(pCurrentComponent);
    if (mfnComp.elementCount() > 0)
    {
        if (mfnComp.componentType() == MFn::Type::kMeshPolygonComponent)
        {
            unsigned int vertCount =  ArikaraSelection::ConvertFacesToVertex(pObject, pCurrentComponent, pVertexComponent);
            if (vertexCount)
                *vertexCount = vertCount;
            return true;
        }
        else if (mfnComp.componentType() == MFn::Type::kMeshEdgeComponent)
        {
            unsigned int vertCount = ArikaraSelection::ConvertEdgesToVertex(pObject, pCurrentComponent, pVertexComponent);
            if (vertexCount)
                *vertexCount = vertCount;
            return true;
        }
        else if (mfnComp.componentType() == MFn::Type::kMeshVertComponent)
        {
            pVertexComponent = pCurrentComponent;
            if (vertexCount)
                *vertexCount = mfnComp.elementCount();
            return true;
        }
    }
    return false;
}
