#include "ArikaraTransferWeightCmd.h"
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MDagPathArray.h>

#include "../../GlobalDefine.h"

MString ArikaraTransferWeightCmd::m_LastSource = "";
MString ArikaraTransferWeightCmd::m_LastTarget = "";
double ArikaraTransferWeightCmd::m_LastValue = 1.0;


ArikaraTransferWeightCmd::ArikaraTransferWeightCmd()
{
}


ArikaraTransferWeightCmd::~ArikaraTransferWeightCmd()
{
}

MStatus ArikaraTransferWeightCmd::doIt(const MArgList& args)
{
	MStatus status;
	MSyntax mySyntax = newSyntax();
	MArgDatabase parsed(mySyntax, args);

	MSelectionList skinNodeSel;
	parsed.getObjects(skinNodeSel);

	status = skinNodeSel.getDagPath(0, m_geoDagPath, m_component);
	if (status != MS::kSuccess)
	{
		status = skinNodeSel.getDependNode(0, m_skinCluster); ReturnOnError(status);
		if (!m_skinCluster.hasFn(MFn::kSkinClusterFilter))
			return MS::kInvalidParameter;

		MItDependencyGraph dgIt(m_skinCluster, MFn::Type::kMesh, MItDependencyGraph::kDownstream);
		if (!dgIt.isDone())
		{
			MFnDagNode(dgIt.currentItem()).getPath(m_geoDagPath);
			MFnSingleIndexedComponent comp;
			comp.setComplete(true);
			m_component = comp.create(MFn::kMeshVertComponent, &status); ReturnOnError(status);
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
		if (m_component.isNull())
		{
			m_geoDagPath.extendToShape();

			MFnSingleIndexedComponent comp;
			comp.setComplete(true);
			m_component = comp.create(MFn::kMeshVertComponent, &status); ReturnOnError(status);
		}

		MItDependencyGraph dgIt(m_geoDagPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
		if (!dgIt.isDone())
		{
			m_skinCluster = dgIt.currentItem();
		}
		else
		{
			return MS::kInvalidParameter;
		}
	}

	if (parsed.isFlagSet(kATWValueFlag))
	{
		m_value = parsed.flagArgumentDouble(kATWValueFlag, 0);
		m_LastValue = m_value;
	}
	else
	{
		m_LastValue = m_value;
	}

	MString source(m_LastSource);
	MString target(m_LastTarget);
	if (parsed.isFlagSet(kATWSourceFlag))
	{
		source = parsed.flagArgumentString(kATWSourceFlag, 0);
	}
	if (parsed.isFlagSet(kATWTargetFlag))
	{
		target = parsed.flagArgumentString(kATWTargetFlag, 0);
	}

	MSelectionList influences;
	status = influences.add(source); if (status != MS::kSuccess) { displayError("Invalid Object: " + source); return MS::kInvalidParameter; };
	status = influences.add(target); if (status != MS::kSuccess) { displayError("Invalid Object: " + target); return MS::kInvalidParameter; };

	MDagPath sourceDagPath, targetDagPath;
	status = influences.getDagPath(0, sourceDagPath); if (status != MS::kSuccess) { displayError("Invalid Object: " + source); return MS::kInvalidParameter; };
	status = influences.getDagPath(1, targetDagPath); if (status != MS::kSuccess) { displayError("Invalid Object: " + target); return MS::kInvalidParameter; };

	MFnSkinCluster lskin(m_skinCluster);
	MDagPathArray influencesDagPath;
	unsigned int influencesCount = lskin.influenceObjects(influencesDagPath);

	m_influenceIndex.setLength(2);
	bool targetFound, sourceFound = false;
	for (unsigned int x = 0; x < influencesCount; x++)
	{
		if (sourceDagPath == influencesDagPath[x])
		{
			m_influenceIndex[atwSource] = x;
			sourceFound = true;
		}
		else if (targetDagPath == influencesDagPath[x])
		{
			m_influenceIndex[atwTarget] = x;
			targetFound = true;
		}

		if (targetFound && sourceFound)
			break;
	}

	if (!sourceFound)
	{
		displayError("Influence object: \"" + source + "\" is not influencing current skinCluster");
		return MS::kInvalidParameter;
	}
	if (!targetFound)
	{
		displayError("Influence object: \"" + target + "\" is not influencing current skinCluster");
		return MS::kInvalidParameter;
	}

	unsigned int influenceCount;
	lskin.getWeights(m_geoDagPath, m_component, m_oldWeight, influenceCount);
	m_influenceIndexRedo.setLength(influenceCount);
	for (unsigned int x = 0; x < influenceCount; x++)
		m_influenceIndexRedo[x] = x;

	return redoIt();
}

MStatus ArikaraTransferWeightCmd::redoIt()
{
	MFnSkinCluster lskin(m_skinCluster);
	
	MDoubleArray tmpWeights;
	lskin.getWeights(m_geoDagPath, m_component, m_influenceIndex, tmpWeights);

	unsigned int vertCount = tmpWeights.length() / 2;
	MDoubleArray newWeights(tmpWeights.length(), 0.0);

	for (unsigned int v = 0; v < vertCount; v++)
	{
		double sourceWeight = tmpWeights[v * 2 + atwSource] * m_value;
		newWeights[v * 2 + atwSource] = tmpWeights[v * 2 + atwSource] - sourceWeight;
		newWeights[v * 2 + atwTarget] = tmpWeights[v * 2 + atwTarget] + sourceWeight;
	}

	//lskin.setWeights(m_geoDagPath, m_component, m_influenceIndex, newWeights, false, &m_oldWeight);
	lskin.setWeights(m_geoDagPath, m_component, m_influenceIndex, newWeights, false);
	
	return MS::kSuccess;
}

MStatus ArikaraTransferWeightCmd::undoIt()
{
	MFnSkinCluster lskin(m_skinCluster);
	lskin.setWeights(m_geoDagPath, m_component, m_influenceIndexRedo, m_oldWeight, false);
	return MS::kSuccess;
}

void* ArikaraTransferWeightCmd::creator()
{
	return new ArikaraTransferWeightCmd;
}

MSyntax ArikaraTransferWeightCmd::newSyntax()
{
	MSyntax syntax;

	syntax.setObjectType(MSyntax::MObjectFormat::kSelectionList, 1);
	syntax.useSelectionAsDefault(true);

	syntax.addFlag(kATWSourceFlag, kATWSourceFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kATWTargetFlag, kATWTargetFlagLong, MSyntax::MArgType::kString);
	syntax.addFlag(kATWValueFlag, kATWValueFlagLong,   MSyntax::MArgType::kDouble);

	return syntax;
}
