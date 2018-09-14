//
// Copyright (C) JonasOuel
// 
// File: ArikaraSkinEditorCmd.cpp
//
// MEL Command: arikaraSkinEditor
//
// Author: Maya Plug-in Wizard 2.0
//

#include "../include/ArikaraSkinEditorCmd.h"

#include "../../Editor/arikaraSkinEditor.h"

#include <maya/MGlobal.h>
#include <maya/MArgDatabase.h>
#include <maya/MQtUtil.h>

QPointer<ArikaraSkinEditor>	ArikaraSkinEditorCmd::ArikaraWindow;
MCallbackId ArikaraSkinEditorCmd::onSelectionChangedID = 0;
MCallbackId ArikaraSkinEditorCmd::onSceneChangedID = 0;
MCallbackId ArikaraSkinEditorCmd::onSceneSavedBeforeID = 0;
MCallbackId ArikaraSkinEditorCmd::onSceneSavedAfterID = 0;

MSyntax ArikaraSkinEditorCmd::newSyntax()
{
	MSyntax syntax;

	syntax.addFlag(kAriShowToolFlag, kAriShowToolFlagLong, MSyntax::kNoArg);
	syntax.addFlag(kAriSetInfluenceFlag, kAriSetInfluenceFlagLong, MSyntax::kString);

	return syntax;
}



void ArikaraSkinEditorCmd::updateToolUI()
{
	if (!ArikaraWindow.isNull())
	{
		if (ArikaraWindow->isVisible())
		{
			ArikaraWindow->refreshUI();
		}
	}
}

void ArikaraSkinEditorCmd::onInfluenceListChanged()
{
    if (!ArikaraWindow.isNull())
    {
        if (ArikaraWindow->isVisible())
        {
            ArikaraWindow->arikaraSkin.OnInfluenceListChanged();
            ArikaraWindow->refreshUI();
        }
    }
}

void ArikaraSkinEditorCmd::onSelectionChanged(void* data)
{
	if (!ArikaraWindow.isNull())
	{
		if (ArikaraWindow->isVisible())
		{
			ArikaraWindow->arikaraSkin.SetVertexFromSelection();
			ArikaraWindow->refreshUI();
		}
	}
}

void ArikaraSkinEditorCmd::onSceneChanged(void* data)
{
    if (!ArikaraWindow.isNull())
    {
        ArikaraWindow->arikaraSkin.OnSceneNew();
        if (ArikaraWindow->isVisible())
            ArikaraWindow->refreshUI();
    }
}

void ArikaraSkinEditorCmd::onSceneSavedBefore(void* data)
{
    if (!ArikaraWindow.isNull())
    {
        if (ArikaraWindow->isVisible())
        {
            ArikaraWindow->arikaraSkin.OnSceneSavedBefore();
        }
    }
}

void ArikaraSkinEditorCmd::onSceneSavedAfter(void* data)
{
    if (!ArikaraWindow.isNull())
    {
        if (ArikaraWindow->isVisible())
        {
            ArikaraWindow->arikaraSkin.OnSceneSavedAfter();
        }
    }
}

MStatus ArikaraSkinEditorCmd::parseArgs(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;
	MSyntax syntax = newSyntax();
	MArgDatabase parse(syntax, args);

	if (parse.isFlagSet(kAriShowToolFlag))
	{
		createTool();
	}
	
	if (parse.isFlagSet(kAriSetInfluenceFlag))
	{
		MString influenceName = parse.flagArgumentString(kAriSetInfluenceFlag, 0, &status);
		if (status == MS::kSuccess)
			setInfluences(influenceName);
	}

	return status;
}

void ArikaraSkinEditorCmd::setInfluences(const MString &name)
{
	if (!ArikaraWindow.isNull())
	{
		ArikaraWindow->arikaraSkin.setInfluenceFromName(name);
		if (ArikaraWindow->isVisible())
		{
			ArikaraWindow->refreshUI();
		}
	}
}

MStatus ArikaraSkinEditorCmd::doIt( const MArgList& argsList)
{
	return parseArgs(argsList);
	//return redoIt();
}

MStatus ArikaraSkinEditorCmd::redoIt()
{
	return MS::kSuccess;
}

MStatus ArikaraSkinEditorCmd::undoIt()
{
	return MS::kSuccess;
}

void* ArikaraSkinEditorCmd::creator()
{
	return new ArikaraSkinEditorCmd();
}

void ArikaraSkinEditorCmd::cleanup()
{
	if (onSelectionChangedID != 0)
	{
		MEventMessage::removeCallback(onSelectionChangedID);
        onSelectionChangedID = 0;
	}
    if (onSceneSavedBeforeID != 0)
    {
        MSceneMessage::removeCallback(onSceneSavedBeforeID);
        onSceneSavedBeforeID = 0;
    }
    if (onSceneSavedAfterID != 0)
    {
        MSceneMessage::removeCallback(onSceneSavedAfterID);
        onSceneSavedAfterID = 0;
    }

	if (!ArikaraWindow.isNull())
	{
		delete ArikaraWindow;
		ArikaraWindow = nullptr;
	}
}

void ArikaraSkinEditorCmd::createTool()
{
	if (ArikaraWindow.isNull()) {
		
		ArikaraWindow = new ArikaraSkinEditor(MQtUtil::mainWindow());
		if (onSelectionChangedID == 0)
			onSelectionChangedID = MEventMessage::addEventCallback("SelectionChanged", ArikaraSkinEditorCmd::onSelectionChanged);
        if (onSceneChangedID == 0)
            onSceneChangedID = MEventMessage::addEventCallback("SceneOpened", ArikaraSkinEditorCmd::onSceneChanged);
        if (onSceneSavedBeforeID == 0)
            onSceneSavedBeforeID = MSceneMessage::addCallback(MSceneMessage::Message::kBeforeSave, onSceneSavedBefore);
        if (onSceneSavedAfterID == 0)
            onSceneSavedAfterID = MSceneMessage::addCallback(MSceneMessage::Message::kAfterSave, onSceneSavedAfter);

		ArikaraWindow->show();
	}
	else {
		ArikaraWindow->show();
		ArikaraWindow->activateWindow();
	}
}

ArikaraSkinEditorCmd::ArikaraSkinEditorCmd()
{}

ArikaraSkinEditorCmd::~ArikaraSkinEditorCmd()
{
}

bool ArikaraSkinEditorCmd::isUndoable() const
{
	return false;
}
