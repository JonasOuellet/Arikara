#pragma once
#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MDoubleArray.h>
#include <maya/MDGModifier.h>
#include <maya//MIntArray.h>

#define kUseTargetFlag               "-ut"
#define kUseTargetFlagLong           "-useTarget"


class MFnMesh;

class ArikaraAttachSkinTargetCmd : public MPxCommand
{
public:
    MStatus     doIt(const MArgList&);
    bool		isUndoable() const { return false; }
    static		void* creator() { return new ArikaraAttachSkinTargetCmd; }

    static MSyntax newSyntax();
    static bool isTargetSet;
    static bool isUsingComponent;
    static MObject TargetComponent;
    static MDagPath TargetGeoDagPath;
    static MObject TargetSkinClusterObject;
};


class ArikaraAttachSkinCmd : public MPxCommand
{
public:
    ArikaraAttachSkinCmd();
    virtual ~ArikaraAttachSkinCmd();

    MStatus     doIt(const MArgList&);
    MStatus		redoIt();
    MStatus		undoIt();
    bool		isUndoable() const { return true; }

    static		void* creator();
    static MSyntax newSyntax();

private:

    bool m_UseTargetFlag;

    /*Target Variable.
    */
    bool m_UseDifferentTarget;
    bool m_UseTargetComponent;

    MObject m_TargetComponent;
    MDagPath m_TargetGeoDagPath;
    MObject m_TargetSkinClusterObj;

    /*Source Variable
    */
    bool m_UseSourceComponent;
    bool m_SourceHasSkinCluster;

    MObject m_SourceComponent;
    MDagPath m_SourceGeoDagPath;
    MObject m_SourceSkinClusterObj;

    MDoubleArray m_oldWeights;
    MIntArray m_influencesIndex;

    MDGModifier m_MDGModifer;

    int rbfType;

    bool createANewGeo(MObject& sourceGeo, MFnMesh& targetMFn, MSpace::Space pSpace = MSpace::kObject);
};
