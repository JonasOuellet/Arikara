#pragma once
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MObject.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MObjectArray.h>

/*
How to use the command:
	arikaraInfluence [flags] skinclusterName
	[flags] :
		-add -a     (selection list) influences list
		-remove -r  (selection list) influences list
		-removeUnused -ru  (double) min value to remove.
		-maxInfluence -mi  (int) max influences.
*/

#define kAIRemoveFlag             "-rem"
#define kAIRemoveFlagLong         "-remove"
#define kAIAddFlag                "-add"
#define kAIAddFlagLong			  "-addInfluences"
#define kAIRemoveUnusedFlag		  "-ru"
#define kAIRemoveUnusedFlagLong   "-removeUnused"
#define kARIMaxInfluenceFlag      "-mi"
#define kARIMaxInfluenceFlagLong  "-maxInfluence"
#define kARIResetBindPoseFlag     "-rbp"
#define kARIResetBindPoseFlagLong "-resetBindPose"


class ArikaraInfluenceCmd : public MPxCommand
{
public:
	ArikaraInfluenceCmd();
	virtual ~ArikaraInfluenceCmd();

	MStatus doIt(const MArgList&);
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const { return true; }

	static		void* creator();
	static MSyntax newSyntax();

private:
	bool m_AddInfluencesFlag;
	bool m_RemoveInfluencesFlag;
	bool m_MaxInfluencesFlag;
	bool m_RemoveUnusedFlag;
	bool m_ResetBindPoseFlag;

	MSelectionList m_InfluencesList;
	double m_RemoveInfluenceValue;
	unsigned int m_MaxInfluenceCount;
    unsigned int m_influenceCount;
    MObject m_component;

	MDGModifier m_MDGModifier;

	MDoubleArray m_OldWeights;
	MObject m_SkinCluster;

	MStatus removeUnusedInfluences();
    MStatus setMaxInfluences();

	MDagPath m_geo;

	MObjectArray m_oldMatrix;
	MObjectArray m_newMatrix;

	MStatus resetBindPoseInit();
	MStatus resetbindPose(MObjectArray& mat);
};