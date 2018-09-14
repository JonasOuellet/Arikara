#pragma once
#include <maya/MPxCommand.h>
#include <maya/MDoubleArray.h>
#include <maya/MDagPath.h>
#include <maya/MIntArray.h>

/*
Command only used by the Arikara Skin editor for undo purpose.
*/


class ArikaraSkinEditorSetWeightCmd : public MPxCommand
{
public:
	ArikaraSkinEditorSetWeightCmd();
	virtual ~ArikaraSkinEditorSetWeightCmd();

	MStatus		doIt(const MArgList&);
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const { return true; }

	static		void* creator();

	MDoubleArray newWeights;
	MDoubleArray oldWeights;
	MObject component;
	MObject skinCluter;
	MDagPath geoDagPath;
	MIntArray influences;

	static bool initCommand(const MDagPath& obj, const MObject &skin, const MObject& comp);
	static bool closeCommand();
	static ArikaraSkinEditorSetWeightCmd* lastCreatedCommand;
};
