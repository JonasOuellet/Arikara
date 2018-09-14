#ifndef _ArikaraSkinEditorCmd
#define _ArikaraSkinEditorCmd
//
// Copyright (C) JonasOuel
// 
// File: ArikaraSkinEditorCmd.h
//
// MEL Command: arikaraSkinEditor
//
// Author: Maya Plug-in Wizard 2.0
//

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MEventMessage.h>
#include <maya/MSceneMessage.h>

#include <QtCore/QPointer>

#define kAriShowToolFlag	 "-s"
#define kAriShowToolFlagLong "-show"

#define kAriSetInfluenceFlag      "-si"
#define kAriSetInfluenceFlagLong  "-setInfluence"

class ArikaraSkinEditor;

class ArikaraSkinEditorCmd : public MPxCommand
{

public:
	ArikaraSkinEditorCmd();
	virtual		~ArikaraSkinEditorCmd();

	MStatus		doIt( const MArgList& );
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const;

	static		void* creator();
	static void cleanup();
	static MSyntax newSyntax();

	static void updateToolUI();
    static void onInfluenceListChanged();

    /*
     *  Callback function.
    */
	static void onSelectionChanged(void* data);
    static void onSceneChanged(void* data);
    static void onSceneSavedBefore(void* data);
    static void onSceneSavedAfter(void* data);

private:
	void createTool();
	
	MStatus parseArgs(const MArgList&);

	void setInfluences(const MString &name);

	static QPointer<ArikaraSkinEditor> ArikaraWindow;

	static MCallbackId onSelectionChangedID;
    static MCallbackId onSceneChangedID;
    static MCallbackId onSceneSavedBeforeID;
    static MCallbackId onSceneSavedAfterID;
};

#endif