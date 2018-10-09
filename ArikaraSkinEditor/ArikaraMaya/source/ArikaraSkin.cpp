#include "../include/ArikaraSkin.h"
#include "../include/MayaSkinFunction.h"
#include <maya/MFnDagNode.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MSelectionList.h>
#include <maya/MObjectHandle.h>
#include <maya/MColorArray.h>

#include <string>
#include "../command/ArikaraSkinEditorSetWeightCmd.h"


using namespace ArikaraSkinUtils;

ArikaraSkin::ArikaraSkin() : iEdit(this), m_GeoDisplay(this)
{
	mVertexCount = 0;
	mLockedVertexCount = 0;
	currentInfluence = -1;
	useSoftSelection = false;
	useLockVertex = false;
}

ArikaraSkin::~ArikaraSkin()
{

}

void ArikaraSkin::selectVertexAffectedByCurrentInfluences()
{
	if (isSkinValid() && currentInfluence != -1)
	{
		MStatus status;
		MFnSkinCluster skin(skinClusterMObject);
		MDagPathArray influences;
		int influCount = static_cast<int>(skin.influenceObjects(influences, &status));
		if (currentInfluence < influCount)
		{
			MDagPath influ = influences[currentInfluence];
			MSelectionList sel;
			MDoubleArray weight;
			skin.getPointsAffectedByInfluence(influ, sel, weight);
			for (unsigned int x = 0; x < sel.length(); x++)
			{
				MDagPath objDag;
				MObject comp;
				sel.getDagPath(x, objDag, comp);
				if (geoDagPath == objDag)
				{
					MGlobal::select(objDag, comp, MGlobal::kReplaceList);
					return;
				}
			}
		}
	}
}

void ArikaraSkin::SetVertexWeight(float value)
{
	if (isSkinValid() && mVertexCount > 0 && currentInfluence != -1)
	{
		MGlobal::executeCommand("arikaraSkinEditorSetWeight", false, true);
		if (!ArikaraSkinEditorSetWeightCmd::initCommand(geoDagPath, skinClusterMObject, mVertexSelection))
			return;
		MFnSkinCluster skin(skinClusterMObject);
		MDoubleArray weights;
		skin.getWeights(geoDagPath, mVertexSelection, currentInfluence, weights);
		for (unsigned int x = 0; x < weights.length(); x++)
		{
			weights[x] = value * mSoftSelectionWeight[x];
		}
		MIntArray influencesArr;
		influencesArr.append(currentInfluence);
		skin.setWeights(geoDagPath, mVertexSelection, influencesArr, weights, true);
		ArikaraSkinEditorSetWeightCmd::closeCommand();
	}
}

void ArikaraSkin::AddVertexWeight(float value)
{
	if (isSkinValid() && mVertexCount > 0 && currentInfluence != -1)
	{
		MGlobal::executeCommand("arikaraSkinEditorSetWeight", false, true);
		if (!ArikaraSkinEditorSetWeightCmd::initCommand(geoDagPath, skinClusterMObject, mVertexSelection))
			return;

		MFnSkinCluster skin(skinClusterMObject);
		MDoubleArray weights;
		skin.getWeights(geoDagPath, mVertexSelection, currentInfluence, weights);
		for (unsigned int x = 0; x < weights.length(); x++)
		{
			weights[x] += value * mSoftSelectionWeight[x];
			if (weights[x] > 1.0)
				weights[x] = 1.0;
			else if (weights[x] < 0.0)
				weights[x] = 0.0;
		}
		MIntArray influencesArr;
		influencesArr.append(currentInfluence);
		skin.setWeights(geoDagPath, mVertexSelection, influencesArr, weights, true);
		ArikaraSkinEditorSetWeightCmd::closeCommand();
	}
}

void ArikaraSkin::SetObject(MDagPath pDagPath, MObject pSkin)
{

}

bool ArikaraSkin::SetObjectFromSelection()
{
    MDagPath tmpDagPath = transformDagPath;
	if (GetFirstObjectWithSkinFromSelection(transformDagPath, skinClusterMObject))
	{
        geoDagPath = transformDagPath;
        geoDagPath.extendToShape();
        if (!(tmpDagPath == transformDagPath))
        {
            currentInfluence = 0;
            OnGeometryChanded(geoDagPath);
            OnInfluenceChanged(currentInfluence);
            clean();
            OnInfluenceListChanged();
        }
		return true;
	}
	else
	{
		return false;
	}
}


bool ArikaraSkin::SetVertexFromSelection(unsigned int* vertexCount /*= nullptr*/)
{
	if (geoDagPath.isValid())
	{
		if (useSoftSelection)
		{
			if (ArikaraSelection::GetSoftSelectedVertexForObject(geoDagPath, mVertexSelection, &mVertexCount, &mSoftSelectionWeight))
			{
				if (vertexCount)
				    *vertexCount = mVertexCount;

				updateVertexSelectionWeightWithLock();
				return true;
			}
		}
		else
		{
			if (ArikaraSelection::GetSelectedVertexForObject(geoDagPath, mVertexSelection, &mVertexCount))
			{
				if (vertexCount)
					*vertexCount = mVertexCount;

				mSoftSelectionWeight.clear();
				mSoftSelectionWeight.resize(mVertexCount, 1.0f);
				updateVertexSelectionWeightWithLock();
				return true;
			}
		}
	}
	mVertexCount = 0;
	return false;
}

bool ArikaraSkin::GetAverageWeightForAllInfluences(std::vector<double> &avgWeight, unsigned int *pInfluencesCount)
{
	if (!skinClusterMObject.isNull() && mVertexCount > 0)
	{
		//MGlobal::displayInfo("Getting Average Weight");
		MFnSkinCluster skinClust;
		skinClust.setObject(skinClusterMObject);

		unsigned int influencesCount = 0;
		MDoubleArray weights;
		skinClust.getWeights(geoDagPath, mVertexSelection, weights, influencesCount);

		avgWeight.resize(influencesCount);
		
        /* Try for getting the average with softselection weight;
        */
        double wcount = 0.0;
        for (unsigned int x = 0; x < mVertexCount; x++)
        {
            wcount += mSoftSelectionWeight[x];
        }
        wcount = 1.0 / wcount;

        double w;
        for (unsigned int influence = 0; influence < influencesCount; influence++)
		{
            w = 0.0;
			for (unsigned int vert = 0; vert < mVertexCount; vert++)
			{
				w += weights[vert * influencesCount + influence] * mSoftSelectionWeight[vert];
			}
			//avgWeight[influence] = w * mVertexCount;
            avgWeight[influence] = w * wcount;
		}

		if (pInfluencesCount != nullptr)
		{
			*pInfluencesCount = influencesCount;
		}

		return true;
	}
	return false;
}

bool ArikaraSkin::GetAverageWeightForInfluences(MIntArray& influences, std::vector<double> &avgWeight)
{
    if (!skinClusterMObject.isNull() && mVertexCount > 0)
    {
        MFnSkinCluster skinClust;
        skinClust.setObject(skinClusterMObject);
        
        unsigned int influencesCount = influences.length();
        MDoubleArray weights;
        skinClust.getWeights(geoDagPath, mVertexSelection, influences, weights);

        avgWeight.resize(influencesCount);

        /* Try for getting the average with softselection weight;
        */
        double wcount = 0.0;
        for (unsigned int x = 0; x < mVertexCount; x++)
        {
            wcount += mSoftSelectionWeight[x];
        }
        wcount = 1.0 / wcount;

        double w;
        for (unsigned int influence = 0; influence < influencesCount; influence++)
        {
            w = 0.0;
            for (unsigned int vert = 0; vert < mVertexCount; vert++)
            {
                w += weights[vert * influencesCount + influence] * mSoftSelectionWeight[vert];
            }
            //avgWeight[influence] = w * mVertexCount;
            avgWeight[influence] = w * wcount;
        }

        return true;
    }
    return false;
}

double ArikaraSkin::GetAverageWeightForCurrentInfluences()
{
	if (!skinClusterMObject.isNull() && mVertexCount > 0 && currentInfluence >= 0)
	{
		MFnSkinCluster skinClust;
		skinClust.setObject(skinClusterMObject);

		MDoubleArray weights;
		MStatus status = skinClust.getWeights(geoDagPath, mVertexSelection, currentInfluence, weights);
		if (status != MS::kSuccess)
			return 0.0;

		double averageWeight = 0.0;
        double vWeights = 0.0;
		for (unsigned int x = 0; x < weights.length(); x++)
		{
			averageWeight += weights[x] * mSoftSelectionWeight[x];
            vWeights += mSoftSelectionWeight[x];
		}
		//averageWeight /= weights.length();
        averageWeight /= vWeights;
		return averageWeight;
	}
	return 0.0;
}

bool ArikaraSkin::isSkinValid()
{
    MObjectHandle skinHandle(skinClusterMObject);
    return skinHandle.isValid();
}

bool ArikaraSkin::LockSelectedVerts(unsigned int* vertexCount /*= nullptr*/)
{
	if (geoDagPath.isValid())
	{
		/*if (useSoftSelection)
		{
            MObject lTempVert;
			if (ArikaraSelection::GetSoftSelectedVertexForObject(geoDagPath, lTempVert))
			{
				MStatus status;

                MSelectionList sel;
                if (!mLockedVertex.isNull())
                    sel.add(geoDagPath, mLockedVertex);
                sel.add(geoDagPath, lTempVert);

                sel.getDagPath(0, geoDagPath, mLockedVertex);
                MFnComponent mfnComp(mLockedVertex);
				mLockedVertexCount = static_cast<unsigned int>(mfnComp.elementCount());
                mLockedVertexWeight.resize(mLockedVertexCount);
                for (unsigned int x = 0; x < mLockedVertexCount; x++)
                    mLockedVertexWeight[x] = mfnComp.weight(x).influence();

				if (vertexCount != nullptr)
				{
					*vertexCount = mLockedVertexCount;
				}
				return true;
			}
		}
		else
		{*/
            MObject lTempVert;
            unsigned int tmpCount;
			if (ArikaraSelection::GetSelectedVertexForObject(geoDagPath, lTempVert, &tmpCount))
			{
                MStatus status;

                MFnSingleIndexedComponent mfnLockComp;
                if (mLockedVertexCount == 0)
                    mLockedVertex = mfnLockComp.create(MFn::Type::kMeshVertComponent);
                else
                    mfnLockComp.setObject(mLockedVertex);
                
                MFnSingleIndexedComponent mfnTmpComp(lTempVert);
                MIntArray elem;
                mfnTmpComp.getElements(elem);
                mfnLockComp.addElements(elem);
                mLockedVertexCount = static_cast<unsigned int>(mfnLockComp.elementCount());
                mLockedVertexWeight.clear();
                mLockedVertexWeight.resize(mLockedVertexCount, 1.0f);
                if (vertexCount != nullptr)
                {
                    *vertexCount = mLockedVertexCount;
                }

				return true;
			}
		//}
	}
	mLockedVertexCount = 0;
	return false;
}


bool ArikaraSkin::UnlockSelectedVerts(unsigned int* vertexCount /*= nullptr*/)
{ 
	if (geoDagPath.isValid() && !mLockedVertex.isNull() && mVertexCount > 0)
	{
		MObject tmpSel;
        unsigned int tmpSelCount = 0;
		if (ArikaraSelection::GetSelectedVertexForObject(geoDagPath, tmpSel, &tmpSelCount))
        {
			MFnSingleIndexedComponent mfnLockedVerts(mLockedVertex);
			MFnSingleIndexedComponent MfncurSel(tmpSel);
			
			std::vector<float> newWeights;
			MFnSingleIndexedComponent newIndex;
			MObject newIndexObject = newIndex.create(MFn::Type::kMeshVertComponent);
            
            int selIdx = MfncurSel.element(0);
            int locIdx = mfnLockedVerts.element(0);
            unsigned int s = 0;
            unsigned int l = 0;
            while (s < tmpSelCount && l < mLockedVertexCount)
            {
                if (locIdx < selIdx)
                {
                    newIndex.addElement(locIdx);
                    newWeights.push_back(mLockedVertexWeight[l]);
                    l++;
                    locIdx = mfnLockedVerts.element(l);
                }
                else if (locIdx > selIdx)
                {
                    s++;
                    selIdx = MfncurSel.element(s);
                }
                else
                {
                    s++;
                    l++;
                    selIdx = MfncurSel.element(s);
                    locIdx = mfnLockedVerts.element(l);
                }
            }
            for (; l < mLockedVertexCount; l++)
            {
                newIndex.addElement(mfnLockedVerts.element(l));
                newWeights.push_back(mLockedVertexWeight[l]);
            }

			/*for (unsigned int y = 0; y < lockVertIndex.length(); y++)
			{
				bool found = false;
				int curIndex = lockVertIndex[y];
				for (int x = 0; x < MfncurSel.elementCount(); x++)
				{
					if (curIndex == MfncurSel.element(x))
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					newIndex.addElement(curIndex);
					newWeights.push_back(mLockedVertexWeight[y]);
				}
			}
            */
			mLockedVertex = newIndex.object();
			mLockedVertexWeight = newWeights;
			mLockedVertexCount = newIndex.elementCount();
			if (vertexCount)
				*vertexCount = mLockedVertexCount;
		}
        return true;
	}
	return false;
}

bool ArikaraSkin::ClearLockedVertex()
{
	mLockedVertexCount = 0;
	mLockedVertex = MObject();
	mLockedVertexWeight.clear();
	return true;
}

unsigned int ArikaraSkin::GetLockedVertexCount()
{
	return mLockedVertexCount;
}

unsigned int ArikaraSkin::GetVertexSelectedCount()
{
	return mVertexCount;
}

void ArikaraSkin::selectLockedVert()
{
	if (geoDagPath.isValid() && mLockedVertexCount > 0)
	{
		MGlobal::select(geoDagPath, mLockedVertex, MGlobal::kReplaceList);
	}
}

std::string ArikaraSkin::GetCurrentInfluenceName()
{
	if (isSkinValid() && currentInfluence != -1)
	{
		MString name = InfluencesList[currentInfluence].partialPathName();
		std::string out(name.asChar());
		return out;
	}
	return "";
}

void ArikaraSkin::setInfluenceFromName(const MString &influenceName)
{
	if (!isSkinValid())
		return;

	MSelectionList sel;
	MStatus status = sel.add(influenceName); ReturnOnErrorVoid(status);
	MDagPath selectedInfluence;
	status = sel.getDagPath(0, selectedInfluence); ReturnOnErrorVoid(status);

	unsigned int influences;
	status = ArikaraSkinUtils::getInfluenceIndex(skinClusterMObject, influenceName, &influences); ReturnOnErrorVoid(status);
	currentInfluence = static_cast<int>(influences);
    OnInfluenceChanged(currentInfluence);
}

void ArikaraSkin::isInfluencesLocked(std::vector<bool>& locked)
{
    locked.resize(InfluencesCount, false);
    MFnDependencyNode mfnDep;
    for (unsigned int x = 0; x < InfluencesCount; x++)
    {
        mfnDep.setObject(InfluencesList[x].node());
        locked[x] = mfnDep.findPlug("liw").asBool();
    }
}

bool ArikaraSkin::addSelectedInfluences()
{
	if (isSkinValid())
	{
		MSelectionList sel;
		MGlobal::getActiveSelectionList(sel);
		if (sel.length() > 0)
		{
			MStringArray influenceName;
			sel.getSelectionStrings(influenceName);
			MString Command("arikaraInfluence -add");
			for (unsigned int x = 0; x < influenceName.length(); x++)
			{
				Command += " ";
				Command += influenceName[x];
			}
			Command += " ";
			Command += transformDagPath.partialPathName();
			Command += ";";
			MGlobal::executeCommand(Command, true, true);

            //OnInfluenceListChanged();
		    
            return true;
		}
	}
	return false;
}

bool ArikaraSkin::removeInfluences(std::vector<unsigned int>& pInfluencesIndex)
{
	if (isSkinValid())
	{
		MFnSkinCluster skin(skinClusterMObject);
		MDagPathArray inf;
		skin.influenceObjects(inf);
		MString Command("arikaraInfluence -rem");
		for (unsigned int x = 0; x < pInfluencesIndex.size(); x++)
		{
			Command += " ";
			Command += inf[pInfluencesIndex[x]].partialPathName();
		}
		Command += " ";
		Command += transformDagPath.partialPathName();
		Command += ";";
		MGlobal::executeCommand(Command, true, true);

        //OnInfluenceListChanged();

		return true;
	}
	return false;
}

bool ArikaraSkin::removeUnusedInfluence(double minVal)
{
	//MGlobal::displayInfo(std::to_string(minVal).c_str());

	if (isSkinValid())
	{
		MFnSkinCluster skin(skinClusterMObject);
		MDagPathArray inf;
		skin.influenceObjects(inf);
		MString Command("arikaraInfluence -ru ");
		Command += std::to_string(minVal).c_str();
		Command += " ";
		Command += transformDagPath.partialPathName();
		Command += ";";
		MGlobal::executeCommand(Command, true, true);

        //OnInfluenceListChanged();

		return true;
	}
	return false;
}

void ArikaraSkin::clean()
{
    mVertexSelection = MObject();
    mVertexCount = 0;
    mSoftSelectionWeight.clear();

    mLockedVertex = MObject();
    mLockedVertexCount = 0;
    mLockedVertexWeight.clear();
}

void ArikaraSkin::clear()
{
    mVertexSelection = MObject::kNullObj;
    mVertexCount = 0;
    mSoftSelectionWeight.clear();

    mLockedVertex = MObject::kNullObj;
    mLockedVertexCount = 0;
    mLockedVertexWeight.clear();

    currentInfluence = -1;
    skinClusterMObject = MObject::kNullObj;

    transformDagPath = MDagPath();
    geoDagPath = MDagPath();

    m_AriJointDisplay.clear();
    m_GeoDisplay.clear();
    iEdit.clear();
}

void ArikaraSkin::OnInfluenceChanged(int pInfluenceIndex)
{
    if (currentInfluence >= 0)
    {
        MFnSkinCluster skin(skinClusterMObject);
        MDagPathArray influences;
        skin.influenceObjects(influences);

        m_AriJointDisplay.setNewObject(influences[currentInfluence].node());
        //m_GeoDisplay.setBaseColor();
    }
}

void ArikaraSkin::OnGeometryChanded(const MDagPath& pNewGeo)
{
    //m_GeoDisplay.setNewGeometry(pNewGeo.node());
}

void ArikaraSkin::OnToolShow()
{
    OnInfluenceListChanged();
    m_AriJointDisplay.restoreArikaraDisplay();
    //m_GeoDisplay.restoreArikaraDisplay();
}

void ArikaraSkin::OnToolClose()
{
    m_AriJointDisplay.resetSceneState();
    //m_GeoDisplay.resetSceneState();
}

void ArikaraSkin::OnSceneNew()
{
    clear();
}

void ArikaraSkin::OnSceneClose()
{

}

void ArikaraSkin::OnSceneSavedBefore()
{
    m_AriJointDisplay.resetSceneState();
    //m_GeoDisplay.resetSceneState();
}

void ArikaraSkin::OnSceneSavedAfter()
{
    m_AriJointDisplay.restoreArikaraDisplay();
    //m_GeoDisplay.restoreArikaraDisplay();
}

void ArikaraSkin::OnInfluenceListChanged()
{
    MFnSkinCluster mfnSkin(skinClusterMObject);
    InfluencesCount = mfnSkin.influenceObjects(InfluencesList);
}

void ArikaraSkin::operator=(const ArikaraSkin& data)
{

}

void ArikaraSkin::updateVertexSelectionWeightWithLock()
{
	if (mVertexCount == 0 || mLockedVertexCount == 0)
	{
		return;
	}
	if (useLockVertex)
	{
		MStatus status;
		MFnSingleIndexedComponent mfnSelectedVert(mVertexSelection, &status);
		if (status != MS::kSuccess) return;
        MFnSingleIndexedComponent mfnLockedVert(mLockedVertex, &status);
		if (status != MS::kSuccess) return;

        //Test to use int array too if its faster.
        //test to see if looping from the bigger array to the lowest if faster.

        //MIntArray selElem; mfnSelectedVert.getElements(selElem);
        //MIntArray lockElem; mfnLockedVert.getElements(lockElem);

        int selIdx = mfnSelectedVert.element(0);
        int locIdx = mfnLockedVert.element(0);
        unsigned int s = 0;
        unsigned int l = 0;
        while (s < mVertexCount && l < mLockedVertexCount)
        {
            if (selIdx < locIdx)
            {
                s++;
                selIdx = mfnSelectedVert.element(s);
            }
            else if (selIdx > locIdx)
            {
                l++;
                locIdx = mfnLockedVert.element(l);
            }
            else
            {
                //are equal
                mSoftSelectionWeight[s] *= 1.0f - mLockedVertexWeight[l];
                s++; l++;
                selIdx = mfnSelectedVert.element(s);
                locIdx = mfnLockedVert.element(l);
            }
        }
	}
}

ArikaraJointDisplay::ArikaraJointDisplay()
{
    m_SelectedJoint = MObject::kNullObj;
    m_useObjectColor = 1;
    m_objectColor = 1;
}

MColor ArikaraJointDisplay::selectedColor = MColor(1.0f, 1.0f, 1.0f);

void ArikaraJointDisplay::setNewObject(MObject& newSelectedJoint)
{
    MStatus status;
    resetSceneState();

    m_SelectedJoint = newSelectedJoint;
    m_MfnDep.setObject(m_SelectedJoint);
    m_PlugUseObjectColor = m_MfnDep.findPlug("useObjectColor", &status); ReturnOnErrorVoid(status);
    m_PlugObjectColor = m_MfnDep.findPlug("objectColor", &status); ReturnOnErrorVoid(status);
    m_PlugWireColor = m_MfnDep.findPlug("wireColorRGB", &status); ReturnOnErrorVoid(status);

    /*
     * Get color info.
    */
    m_useObjectColor = m_PlugUseObjectColor.asInt();
    if (m_useObjectColor == 1)
    {
        m_objectColor = m_PlugObjectColor.asInt();
    }
    else if (m_useObjectColor == 2)
    {
        m_oldColor.r = m_PlugWireColor.child(0).asFloat();
        m_oldColor.g = m_PlugWireColor.child(1).asFloat();
        m_oldColor.b = m_PlugWireColor.child(2).asFloat();
    }

    restoreArikaraDisplay();
}

void ArikaraJointDisplay::resetSceneState()
{
    MStatus status;
    if (!m_SelectedJoint.isNull())
    {
        m_PlugUseObjectColor.setInt(m_useObjectColor);
        if (m_useObjectColor == 1)
        {
            m_PlugObjectColor.setInt(m_objectColor);
        }
        else if (m_useObjectColor == 2)
        {
            m_PlugWireColor.child(0).setFloat(m_oldColor.r);
            m_PlugWireColor.child(1).setFloat(m_oldColor.g);
            m_PlugWireColor.child(2).setFloat(m_oldColor.b);
        }
    }
}

void ArikaraJointDisplay::restoreArikaraDisplay()
{
    MStatus status;
    if (!m_SelectedJoint.isNull())
    {
        m_PlugUseObjectColor.setInt(2);
        m_PlugWireColor.child(0).setFloat(selectedColor.r);
        m_PlugWireColor.child(1).setFloat(selectedColor.g);
        m_PlugWireColor.child(2).setFloat(selectedColor.b);
    }
}

void ArikaraJointDisplay::clear()
{
    m_SelectedJoint = MObject::kNullObj;
    m_MfnDep.setObject(MObject::kNullObj);
}

void ArikaraGeometryDisplay::setNewGeometry(const MObject& pGeo)
{
    resetSceneState();

    m_geometry = pGeo;
    m_MfnMesh.setObject(m_geometry);

    m_MfnMesh.getCurrentColorSetName(m_baseColorSetName);
    m_IsDisplayingColor = m_MfnMesh.displayColors();

    restoreArikaraDisplay();
}

void ArikaraGeometryDisplay::resetSceneState()
{
    if (!m_geometry.isNull())
    {
        MSelectionList curSel;
        MGlobal::getActiveSelectionList(curSel);

        m_MfnMesh.setCurrentColorSetName(m_baseColorSetName);
        m_MfnMesh.setDisplayColors(m_IsDisplayingColor);

        /*
         * Deleting arikara Color Set;
        */
        if (m_MDGModifier_PolyColor)
        {
            m_MDGModifier_PolyColor->undoIt();
            delete m_MDGModifier_PolyColor;
            m_MDGModifier_PolyColor = nullptr;
        }
        if (m_MDGModifier_ColorSet)
        {
            m_MDGModifier_ColorSet->undoIt();
            delete m_MDGModifier_ColorSet;
            m_MDGModifier_ColorSet = nullptr;
        }
        //m_MfnMesh.deleteColorSet(colorSetName);


        MGlobal::setActiveSelectionList(curSel);
    }
}

void ArikaraGeometryDisplay::restoreArikaraDisplay()
{
    if (!m_geometry.isNull())
    {
        if (!isArikaraColorSetExist())
        {
            if (m_MDGModifier_ColorSet)
            {
                delete m_MDGModifier_ColorSet;
            }
            m_MDGModifier_ColorSet = new MDGModifier();
            m_MfnMesh.createColorSet(colorSetName, m_MDGModifier_ColorSet, true, MFnMesh::MColorRepresentation::kRGB);
        }
        m_MfnMesh.setCurrentColorSetName(colorSetName);
        m_MfnMesh.setDisplayColors(true);

        setBaseColor();
    }
}

void ArikaraGeometryDisplay::updateColor()
{
    MDoubleArray weight;
    MFnSkinCluster mfnskin(m_ArikaraSkin->skinClusterMObject);
    mfnskin.getWeights(m_ArikaraSkin->geoDagPath, m_ArikaraSkin->mVertexSelection, m_ArikaraSkin->currentInfluence, weight);

    MFnSingleIndexedComponent mfnVert(m_ArikaraSkin->mVertexSelection);

    int numVert = mfnVert.elementCount();
    MColorArray colorArr(numVert);
    MIntArray colorIDs(numVert);

    float col;
    for (int x = 0; x < numVert; x++)
    {
        col = static_cast<float>(weight[x]);
        colorArr[x].set(MColor::MColorType::kRGB, col, col, col);
        colorIDs[x] = mfnVert.element(x);
    }

    m_MfnMesh.setVertexColors(colorArr, colorIDs);

}

void ArikaraGeometryDisplay::setBaseColor()
{
    if (m_ArikaraSkin->currentInfluence >= 0)
    {
        MDoubleArray weight;
        MFnSkinCluster mfnskin(m_ArikaraSkin->skinClusterMObject);
        MFnSingleIndexedComponent mfnVert;
        MObject fullSel = mfnVert.create(MFn::Type::kMeshVertComponent);
        mfnVert.setComplete(true);
        mfnskin.getWeights(m_ArikaraSkin->geoDagPath, fullSel, m_ArikaraSkin->currentInfluence, weight);

        int numVert = m_MfnMesh.numVertices();
        float co = 0.0;
        MColorArray colorArr(numVert);
        MIntArray colorIDs(numVert);
        for (int x = 0; x < numVert; x++)
        {
            co = static_cast<float>(weight[x]);
            colorArr[x].set(MColor::MColorType::kRGB, co, co, co);
            colorIDs[x] = x;
        }
        if (m_MDGModifier_PolyColor)
            m_MfnMesh.setVertexColors(colorArr, colorIDs);
        else
        {
            m_MDGModifier_PolyColor = new MDGModifier();
            m_MfnMesh.setVertexColors(colorArr, colorIDs, m_MDGModifier_PolyColor);
        }

    }
}

MString ArikaraGeometryDisplay::colorSetName = "ArikaraColorDisplay";

void ArikaraGeometryDisplay::clear()
{
    m_geometry = MObject::kNullObj;
    m_MfnMesh.setObject(m_geometry);
}

bool ArikaraGeometryDisplay::isArikaraColorSetExist()
{
    int numColor = m_MfnMesh.numColorSets();
    if (numColor == 0)
        return false;

    MStringArray meshColorSet;
    m_MfnMesh.getColorSetNames(meshColorSet);
    for (int x = 0; x < numColor; x++)
    {
        if (colorSetName == meshColorSet[x])
        {
            return true;
        }
    }
    return false;
}
