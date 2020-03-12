//
// Copyright (C) JonasOuel
// 
// File: pluginMain.cpp
//
// Author: Maya Plug-in Wizard 2.0
//
#include "ArikaraMaya/ArikaraOptions.h"
#include "ArikaraMaya/include/ArikaraSkinEditorCmd.h"
#include "ArikaraMaya/command/ArikaraMirrorWeightCmd.h"
#include "ArikaraMaya/command/ArikaraSkinEditorSetWeightCmd.h"
#include "ArikaraMaya/command/ArikaraInfluenceCmd.h"
#include "ArikaraMaya/command/ArikaraSkinDataCmd.h"
#include "ArikaraMaya/command/ArikaraTransferWeightCmd.h"
#include "ArikaraMaya/command/ArikaraAttachSkinCmd.h"
#include "ArikaraMaya/nodes/ArikaraWeightDisplayNode.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>


MStatus initializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is loaded into Maya.  It 
//		registers all of the services that this plug-in provides with 
//		Maya.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "JonasOuel", "2017", "Any");

    status = plugin.registerCommand("arikaraAttachTarget", ArikaraAttachSkinTargetCmd::creator, ArikaraAttachSkinTargetCmd::newSyntax);
    status = plugin.registerCommand("arikaraAttach", ArikaraAttachSkinCmd::creator, ArikaraAttachSkinCmd::newSyntax);

	status = plugin.registerCommand("arikaraTransferWeight", ArikaraTransferWeightCmd::creator, ArikaraTransferWeightCmd::newSyntax);
	status = plugin.registerCommand("arikaraSkinData", ArikaraSkinDataCmd::creator, ArikaraSkinDataCmd::newSyntax);
	status = plugin.registerCommand("arikaraInfluence", ArikaraInfluenceCmd::creator, ArikaraInfluenceCmd::newSyntax);
	status = plugin.registerCommand("arikaraMirrorWeight", ArikaraMirrorWeightCmd::creator, ArikaraMirrorWeightCmd::newSyntax);
	status = plugin.registerCommand("arikaraSkinEditorSetWeight", ArikaraSkinEditorSetWeightCmd::creator);
	status = plugin.registerCommand("arikaraSkinEditor", ArikaraSkinEditorCmd::creator, ArikaraSkinEditorCmd::newSyntax);
	status = plugin.registerNode("arikaraWeightDisplay", ArikaraWeightDisplayNode::id, ArikaraWeightDisplayNode::creator,
		ArikaraWeightDisplayNode::initialize, MPxNode::kDeformerNode);

#ifndef NLOADOPTION
    ArikaraSkinDataCmd::Init();
    ArikaraMirrorWeightCmd::Init();
    ArikaraOption::TheOne()->loadDataFromFile();
#endif // !NLOADOPTION

    MString plugPath = plugin.loadPath();
    ArikaraOption::TheOne()->SetPluginPath(plugPath);

	if (!status) {
		status.perror("registerCommand");
		return status;
	}

    MGlobal::displayInfo("ArikaraSkin Successfully loaded!");

	return status;
}

MStatus uninitializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is unloaded from Maya. It 
//		deregisters all of the services that it was providing.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MStatus   status;
	MFnPlugin plugin( obj );

#ifndef NLOADOPTION
	ArikaraOption::TheOne()->writeDataToFile();
	ArikaraOption::cleanup();
#endif // !NLOADOPTION

    status = plugin.deregisterCommand("arikaraAttach");
    status = plugin.deregisterCommand("arikaraAttachTarget");

	status = plugin.deregisterCommand("arikaraTransferWeight");
	status = plugin.deregisterCommand("arikaraSkinData"); 
	status = plugin.deregisterCommand("arikaraInfluence");
	status = plugin.deregisterCommand("arikaraMirrorWeight");
	status = plugin.deregisterCommand("arikaraSkinEditorSetWeight");

	status = plugin.deregisterNode(ArikaraWeightDisplayNode::id);

	ArikaraSkinEditorCmd::cleanup();

	status = plugin.deregisterCommand("arikaraSkinEditor");
	
	
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

    MGlobal::displayInfo("ArikaraSkin Successfully unloaded!");

	return status;
}
