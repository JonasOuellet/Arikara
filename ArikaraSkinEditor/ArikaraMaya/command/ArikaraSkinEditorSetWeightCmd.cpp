#include "ArikaraSkinEditorSetWeightCmd.h"
#include <maya/MGlobal.h>
#include <maya/MFnSkinCluster.h>

#include "../include/ArikaraSkinEditorCmd.h"

ArikaraSkinEditorSetWeightCmd* ArikaraSkinEditorSetWeightCmd::lastCreatedCommand = nullptr;

ArikaraSkinEditorSetWeightCmd::ArikaraSkinEditorSetWeightCmd()
{

}

ArikaraSkinEditorSetWeightCmd::~ArikaraSkinEditorSetWeightCmd()
{

}

MStatus ArikaraSkinEditorSetWeightCmd::doIt(const MArgList&)
{
	return MS::kSuccess;
}

MStatus ArikaraSkinEditorSetWeightCmd::redoIt()
{
	MFnSkinCluster mfnSkin(skinCluter);
	MStatus status = mfnSkin.setWeights(geoDagPath, component, influences, newWeights, false);
    ArikaraSkinEditorCmd::updateToolUI();
    return status;
}

MStatus ArikaraSkinEditorSetWeightCmd::undoIt()
{
	MFnSkinCluster mfnSkin(skinCluter);
	MStatus status = mfnSkin.setWeights(geoDagPath, component, influences, oldWeights, false);
    ArikaraSkinEditorCmd::updateToolUI();
    return status;
}

void* ArikaraSkinEditorSetWeightCmd::creator()
{
	lastCreatedCommand = new ArikaraSkinEditorSetWeightCmd;
	return lastCreatedCommand;
}

bool ArikaraSkinEditorSetWeightCmd::initCommand(const MDagPath& obj, const MObject &skin, const MObject& comp)
{
	if (lastCreatedCommand)
	{
		lastCreatedCommand->geoDagPath = obj;
		lastCreatedCommand->skinCluter = skin;
		lastCreatedCommand->component = comp;

		MFnSkinCluster Mfnskin(lastCreatedCommand->skinCluter);

		unsigned int influenceCount;
		Mfnskin.getWeights(obj, comp, lastCreatedCommand->oldWeights, influenceCount);

		lastCreatedCommand->influences.setLength(influenceCount);
		for (unsigned int x = 0; x < influenceCount; x++)
			lastCreatedCommand->influences[x] = x;

		return true;
	}
	return false;
}

bool ArikaraSkinEditorSetWeightCmd::closeCommand()
{
	unsigned int influenceCount;
	MFnSkinCluster Mfnskin(lastCreatedCommand->skinCluter);
	Mfnskin.getWeights(lastCreatedCommand->geoDagPath, 
		lastCreatedCommand->component, lastCreatedCommand->newWeights, influenceCount);
	return true;
}
