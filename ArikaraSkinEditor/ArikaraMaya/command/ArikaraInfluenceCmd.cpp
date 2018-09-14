#include "ArikaraInfluenceCmd.h"
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnSkinCluster.h>

#include "../../GlobalDefine.h"
#include <string>
#include "../include/ArikaraSkinEditorCmd.h"

#include <map>

#include "../include/MayaSkinFunction.h"

ArikaraInfluenceCmd::ArikaraInfluenceCmd()
{
	m_AddInfluencesFlag = false;
	m_RemoveInfluencesFlag = false;
	m_MaxInfluencesFlag = false;
	m_RemoveUnusedFlag = false;
	m_ResetBindPoseFlag = false;
}

ArikaraInfluenceCmd::~ArikaraInfluenceCmd()
{

}

MStatus ArikaraInfluenceCmd::doIt(const MArgList& args)
{
	MStatus status;
	MSyntax mySyntax = newSyntax();
	MArgDatabase parsed(mySyntax, args);

	MSelectionList skinNodeSel;
	parsed.getObjects(skinNodeSel);
	unsigned int lastSelIndex = skinNodeSel.length() - 1;
	MDagPath tmp;
	status = skinNodeSel.getDagPath(lastSelIndex, tmp);
	if (status != MS::kSuccess)
	{
		status = skinNodeSel.getDependNode(lastSelIndex, m_SkinCluster); ReturnOnError(status);
		if (!m_SkinCluster.hasFn(MFn::kSkinClusterFilter))
			return MS::kInvalidParameter;

		MItDependencyGraph dgIt(m_SkinCluster, MFn::Type::kMesh, MItDependencyGraph::kDownstream);
		if (!dgIt.isDone())
		{
			MObject geoObject = dgIt.currentItem();
			MFnDagNode dagNode(geoObject);
			dagNode.getPath(m_geo);
		}
		else
		{
			displayError("Couldn't find geometry mesh");
			return MS::kFailure;
		}
	}
	else
	{
		//Try to get the skin from the sel;
		MStatus status;
		tmp.extendToShape();
		MItDependencyGraph dgIt(tmp.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
		if (!dgIt.isDone())
		{
			m_SkinCluster = dgIt.currentItem();
			m_geo = tmp;
		}
		else
		{
			return MS::kInvalidParameter;
		}
	}

	if (parsed.isFlagSet(kAIAddFlag))
	{
		m_AddInfluencesFlag = true;

		if (lastSelIndex == 0)
		{
			return MS::kInvalidParameter;
		}

		m_InfluencesList.clear();
		for (unsigned int x = 0; x < lastSelIndex; x++)
		{
			MDagPath influ;
			status = skinNodeSel.getDagPath(x, influ);
			if (status == MS::kSuccess)
			{
				m_InfluencesList.add(influ);
			}
			
		}
	}
	else if (parsed.isFlagSet(kAIRemoveFlag))
	{
		m_RemoveInfluencesFlag = true;

		if (lastSelIndex == 0)
		{
			return MS::kInvalidParameter;
		}

		m_InfluencesList.clear();
		for (unsigned int x = 0; x < lastSelIndex; x++)
		{
			MDagPath influ;
			status = skinNodeSel.getDagPath(x, influ);
			if (status == MS::kSuccess)
			{
				m_InfluencesList.add(influ);
			}
		}
	}
	else if (parsed.isFlagSet(kAIRemoveUnusedFlag))
	{
		m_RemoveUnusedFlag = true;
		m_RemoveInfluenceValue = parsed.flagArgumentDouble(kAIRemoveUnusedFlag, 0, &status); ReturnOnError(status);
	}
	else if (parsed.isFlagSet(kARIMaxInfluenceFlag))
	{
		m_MaxInfluencesFlag = true;
		m_MaxInfluenceCount = parsed.flagArgumentInt(kARIMaxInfluenceFlag, 0, &status); ReturnOnError(status);
	}
	else if (parsed.isFlagSet(kARIResetBindPoseFlag))
	{
		m_ResetBindPoseFlag = true;
		resetBindPoseInit();
	}
	else
	{
		displayError("No flag specified");
		return MS::kInvalidParameter;
	}

	return redoIt();
}

MStatus ArikaraInfluenceCmd::redoIt()
{
	MFnDependencyNode skinNode(m_SkinCluster);
	MString skinName(skinNode.name());
	if (m_AddInfluencesFlag)
	{
		MString baseCommand("skinCluster -e -wt 0.0 -lw 1 -dr 10.0 -ai ");
		MString unlockCommand("skinCluster -e -lw 0 -inf ");
		for (unsigned int x = 0; x < m_InfluencesList.length(); x++)
		{
			MDagPath curInflu;
			m_InfluencesList.getDagPath(x, curInflu);
			MString com(baseCommand + curInflu.partialPathName() + " " + skinName + ";");
			m_MDGModifier.commandToExecute(com);
			MString com2(unlockCommand + curInflu.partialPathName() + " " + skinName + ";");
			m_MDGModifier.commandToExecute(com2);
		}
		m_MDGModifier.doIt();
        ArikaraSkinEditorCmd::onInfluenceListChanged();
	}
	else if (m_RemoveInfluencesFlag)
	{
		MFnDependencyNode skinNode(m_SkinCluster);
		MString skinName(skinNode.name());
		MString baseCommand("skinCluster -e -ri ");
		for (unsigned int x = 0; x < m_InfluencesList.length(); x++)
		{
			MDagPath curInflu;
			m_InfluencesList.getDagPath(x, curInflu);
			MString com(baseCommand + curInflu.partialPathName() + " " + skinName + ";");
			m_MDGModifier.commandToExecute(com);
		}
		m_MDGModifier.doIt();

        ArikaraSkinEditorCmd::onInfluenceListChanged();
	}
	else if (m_RemoveUnusedFlag)
	{
		MStatus sta = removeUnusedInfluences();
        if (sta == MS::kSuccess)
            ArikaraSkinEditorCmd::onInfluenceListChanged();
		return sta;
	}
	else if (m_MaxInfluencesFlag)
	{
		/*MString maxInf(std::to_string(m_MaxInfluenceCount).c_str());
		MString command = "skinCluster -e -mi " + maxInf + " " + skinName + ";";
		m_MDGModifier.commandToExecute(command);
		m_MDGModifier.doIt();
		ArikaraSkinEditorCmd::updateToolUI();*/
        MStatus status =  setMaxInfluences();
        if (status == MS::kSuccess)
            ArikaraSkinEditorCmd::updateToolUI();
        return status;
	}
	else if (m_ResetBindPoseFlag)
	{
		return resetbindPose(m_newMatrix);
	}

	return MS::kSuccess;
}

MStatus ArikaraInfluenceCmd::undoIt()
{
	if (m_AddInfluencesFlag)
	{
		m_MDGModifier.undoIt();
        ArikaraSkinEditorCmd::onInfluenceListChanged();
	}
	else if (m_RemoveInfluencesFlag)
	{
		m_MDGModifier.undoIt();
        ArikaraSkinEditorCmd::onInfluenceListChanged();
	}
	else if (m_RemoveUnusedFlag)
	{
		m_MDGModifier.undoIt();
        ArikaraSkinEditorCmd::onInfluenceListChanged();
	}
	else if (m_MaxInfluencesFlag)
	{
		m_MDGModifier.undoIt();
        //ArikaraSkinUtils::setWeights(m_SkinCluster, m_OldWeights, m_MDGModifier);
        
        MFnSkinCluster clust(m_SkinCluster);
        MIntArray infl(m_influenceCount);
        for (unsigned int x = 0; x < m_influenceCount; x++)
            infl[x] = x;

        clust.setWeights(m_geo, m_component, infl, m_OldWeights, false);

		ArikaraSkinEditorCmd::updateToolUI();
	}
	else if (m_ResetBindPoseFlag)
	{
		return resetbindPose(m_oldMatrix);
	}
	return MS::kSuccess;
}

void* ArikaraInfluenceCmd::creator()
{
	return new ArikaraInfluenceCmd;
}

MSyntax ArikaraInfluenceCmd::newSyntax()
{
	MSyntax syntax;

	syntax.setObjectType(MSyntax::MObjectFormat::kSelectionList, 1);
	syntax.useSelectionAsDefault(true);
	syntax.addFlag(kAIRemoveFlag, kAIRemoveFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kAIAddFlag, kAIAddFlagLong, MSyntax::MArgType::kNoArg);
	syntax.addFlag(kAIRemoveUnusedFlag, kAIRemoveUnusedFlagLong, MSyntax::MArgType::kDouble);
	syntax.addFlag(kARIMaxInfluenceFlag, kARIMaxInfluenceFlagLong, MSyntax::MArgType::kUnsigned);
	syntax.addFlag(kARIResetBindPoseFlag, kARIResetBindPoseFlagLong, MSyntax::MArgType::kNoArg);

	return syntax;
}

MStatus ArikaraInfluenceCmd::removeUnusedInfluences()
{
	MDagPathArray influenceList;
	MDagPathArray influenceToRemove;

	MFnSkinCluster mfnSkin(m_SkinCluster);

	MFnSingleIndexedComponent MfnVertComp;
	MObject vertComp = MfnVertComp.create(MFn::kMeshVertComponent);
	MfnVertComp.setComplete(true);

	mfnSkin.influenceObjects(influenceList);
	MDoubleArray weight;
	unsigned int influencesCount;
	mfnSkin.getWeights(m_geo, vertComp, weight, influencesCount);

	unsigned int vertexCount = weight.length() / influencesCount;
	for (unsigned int i = 0; i < influencesCount; i++)
	{
		bool keep = false;
		for (unsigned int v = 0; v < vertexCount; v++)
		{
			if (weight[v * influencesCount + i] > m_RemoveInfluenceValue)
			{
				keep = true;
				break;
			}
		}

		if (!keep)
		{
			influenceToRemove.append(influenceList[i]);
		}
	}

	if (influenceToRemove.length() == 0)
	{
		displayError("No Influences found to remove");
		return MS::kNotFound;
	}
	else
	{
		MFnDependencyNode skinNode(m_SkinCluster);
		MString skinName(skinNode.name());
		MString baseCommand("skinCluster -e -ri ");
		MString infoStr = std::to_string(influenceToRemove.length()).c_str();
		infoStr += " Removed Influences [ ";
		for (unsigned int x = 0; x < influenceToRemove.length(); x++)
		{
			MDagPath curInflu = influenceToRemove[x];
			MString com(baseCommand + curInflu.partialPathName() + " " + skinName + ";");
			m_MDGModifier.commandToExecute(com);

			infoStr += curInflu.partialPathName();
			if (x != influenceToRemove.length() - 1)
			{
				infoStr += ", ";
			}
		}
		m_MDGModifier.doIt();

		infoStr += "]";
		displayInfo(infoStr);
	}

	return MS::kSuccess;
}

MStatus ArikaraInfluenceCmd::setMaxInfluences()
{
    MStatus status;

    MFnDependencyNode skiNode(m_SkinCluster);
    MPlug maxPlug = skiNode.findPlug("mi", &status); ReturnOnError(status);
    int maxInf = static_cast<int>(m_MaxInfluenceCount);
    m_MDGModifier.newPlugValueInt(maxPlug, maxInf);
    

    MFnSingleIndexedComponent mfnComp;
    m_component = mfnComp.create(MFn::Type::kMeshVertComponent);
    mfnComp.setComplete(true);

    MFnSkinCluster mfnSkin(m_SkinCluster);
    mfnSkin.getWeights(m_geo, m_component, m_OldWeights, m_influenceCount);

    MDoubleArray newWeights(m_OldWeights.length(), 0.0);
    unsigned int vertexCount = m_OldWeights.length() / m_influenceCount;

    for (unsigned int v = 0; v < vertexCount; v++)
    {
        std::multimap<double, unsigned int> weights;
        for (unsigned int i = 0; i < m_influenceCount; i++)
        {
            double w = m_OldWeights[v * m_influenceCount + i];
            if (w > 0.0)
            {
                weights.emplace(std::pair<double, unsigned int>(w, i));
            }
        }
        double totalWeights = 1.0;
        unsigned int size = static_cast<unsigned int>(weights.size());
        unsigned int x = 0;
        std::multimap<double, unsigned int>::iterator it = weights.begin();
        if (size > m_MaxInfluenceCount)
        {
            for (; x < size - m_MaxInfluenceCount; x++, it++)
            {
                totalWeights -= (*it).first;
            }

            for (; x < size; x++, it++)
            {
                newWeights[v * m_influenceCount + (*it).second] = (*it).first / totalWeights;
            }
        }
        else
        {
            for (; x < size; x++, it++)
            {
                newWeights[v * m_influenceCount + (*it).second] = (*it).first;
            }
        }
    }

    //ArikaraSkinUtils::setWeights(m_SkinCluster, newWeights, m_MDGModifier);
    MIntArray influ(m_influenceCount);
    for (unsigned int i = 0; i < m_influenceCount; i++)
        influ[i] = i;

    mfnSkin.setWeights(m_geo, m_component, influ, newWeights, false);

    m_MDGModifier.doIt();
    //m_OldWeights.clear();

    return MS::kSuccess;
}

MStatus ArikaraInfluenceCmd::resetBindPoseInit()
{
	MStatus status;

	MFnSkinCluster mfnSkin(m_SkinCluster);
	MDagPathArray influences;
	unsigned int influencesCount = mfnSkin.influenceObjects(influences);

	MFnDependencyNode mfnDep(m_SkinCluster);
	MPlug bindMatPlug = mfnDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);

	m_oldMatrix.setLength(influencesCount);
	for (unsigned int x = 0; x < influencesCount; x++)
	{
		unsigned int physicIndex = mfnSkin.indexForInfluenceObject(influences[x]);
		MObject val;
		MPlug curMat = bindMatPlug.elementByPhysicalIndex(physicIndex, &status); ReturnOnError(status);
		curMat.getValue(val);
		m_oldMatrix.set(val, x);
	}

	m_newMatrix.setLength(influencesCount);
	for (unsigned int x = 0; x < influencesCount; x++)
	{
		MFnDependencyNode mfnDep(influences[x].node());
		MPlug bindMat = mfnDep.findPlug("worldInverseMatrix", &status); ReturnOnError(status);
		MPlug matplug = bindMat.elementByLogicalIndex(0, &status); ReturnOnError(status);
		MObject val;
		matplug.getValue(val);
		m_newMatrix.set(val, x);
	}
	return MS::kSuccess;
}

MStatus ArikaraInfluenceCmd::resetbindPose(MObjectArray& mat)
{
	MStatus status;
	MFnSkinCluster mfnSkin(m_SkinCluster);
	MDagPathArray influences;
	unsigned int influencesCount = mfnSkin.influenceObjects(influences);

	MFnDependencyNode mfnDep(m_SkinCluster);
	MPlug bindMatPlug = mfnDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);

	for (unsigned int x = 0; x < influencesCount; x++)
	{
		unsigned int physicIndex = mfnSkin.indexForInfluenceObject(influences[x]);
		bindMatPlug.elementByPhysicalIndex(physicIndex, &status).setValue(mat[x]);
	}

	return MS::kSuccess;
}
