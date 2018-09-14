#pragma once
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MDoubleArray.h>
#include <maya/MDagPath.h>
#include <maya/MIntArray.h>

/*
Command to mirror the selected or specified vertex component of a skincluster deformed object.

how to use the command:
	arikaraMirrorWeight flags [objects]
if the components are selected in the scenes:
	'arikaraMirrorWeight flags' will work.
flags:
	-find    -f (string) to find when matching influences.	  Default = "Left"
	-replace -r (string) to replace when matching influences. Default = "Right"
	-axis    -a (int)    0 = X, 1 = Y, 2 = Z Mirror Axis      Default = 0
	-bias    -b (double) bias for finding matching verts      Default = 0.05
*/


#define kAMWFindFlag		"-f"
#define kAMWFindFlagLong	"-find"
#define kAMWReplaceFlag		"-r"
#define kAMWReplaceFlagLong "-replace"
#define kAMWAxisFlag		"-a"
#define kAMWAxisFlagLong	"-axis"
#define kAMWBiasFlag		"-b"
#define kAMWBiasFlagLong	"-bias"

class ArikaraMirrorWeightCmd : public MPxCommand
{
public:
	ArikaraMirrorWeightCmd();
	virtual ~ArikaraMirrorWeightCmd();

	MStatus		doIt(const MArgList&);
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const {return true;}

	static		void* creator();
	static MSyntax newSyntax();

    static void Init();

    static unsigned int m_defaultAxis;
    static MString m_defaultFind;
    static MString m_defaultReplace;
    static double m_defaultBias;

private:
	void buildMirrorInfluencesArray();
	MStatus findMatchingComponent();

	MDoubleArray mNewValues;
	MDoubleArray mOldValues;
	MObject mComponent;
	MObject mMirrorComponent;
	MDagPath mGeoDagPath;
	MIntArray mInfluences;
	MObject mSkinCluster;

	MString mFind;
	MString mReplace;
	double mBias;
	unsigned int mAxis;
};