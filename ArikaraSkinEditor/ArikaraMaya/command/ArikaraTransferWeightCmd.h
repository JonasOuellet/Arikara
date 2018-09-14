#pragma once
#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>

#define kATWSourceFlag       "-s"
#define kATWSourceFlagLong   "-source"
#define kATWTargetFlag       "-t"
#define kATWTargetFlagLong   "-target"
#define kATWValueFlag        "-v"
#define kATWValueFlagLong    "-value"

#define atwSource 0
#define atwTarget 1

class ArikaraTransferWeightCmd : public MPxCommand
{
public:
	ArikaraTransferWeightCmd();
	~ArikaraTransferWeightCmd();

	MStatus doIt(const MArgList&);
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const { return true; }

	static		void* creator();
	static MSyntax newSyntax();

private:
	static MString m_LastSource;
	static MString m_LastTarget;

	static double m_LastValue;

	MObject m_skinCluster;
	MDagPath m_geoDagPath;

	MIntArray m_influenceIndex;
	MIntArray m_influenceIndexRedo;
	MObject m_component;
	double m_value;

	MDoubleArray m_oldWeight;
};

