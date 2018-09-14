#pragma once
#include <maya/MDagPath.h>
#include <maya/MColor.h>
#include <maya/MFnMesh.h>
#include <maya/MString.h>
#include <maya/MDGModifier.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDagPathArray.h>
#include <maya/MPlug.h>

#include "../../skinMode/InteractiveEdit.h"
#include "GlobalDefine.h"

class ArikaraSkin;

class ArikaraGeometryDisplay
{
public:
    ArikaraGeometryDisplay(ArikaraSkin* pArikara) :
        m_ArikaraSkin(pArikara)
    {
        m_geometry = MObject::kNullObj;
        m_MDGModifier_ColorSet = nullptr;
        m_MDGModifier_PolyColor = nullptr;
    };
    ~ArikaraGeometryDisplay() 
    {
        if (m_MDGModifier_ColorSet)
            delete m_MDGModifier_ColorSet;
        if (m_MDGModifier_PolyColor)
            delete m_MDGModifier_PolyColor;
    };

    enum DisplayMode
    {
        kSingleInfluenceWeight = 0,
        kStretching            = 1
    };

    void setNewGeometry(const MObject& pGeo);
    void resetSceneState();
    void restoreArikaraDisplay();

    void updateColor();
    void setBaseColor();

    static MString colorSetName;

    void clear();

private:
    ArikaraSkin* m_ArikaraSkin;

    MObject m_geometry;
    MFnMesh m_MfnMesh;
    //MFnMesh m_MfnMeshTest;

    MString m_baseColorSetName;

    bool m_IsDisplayingColor;

    bool isArikaraColorSetExist();

    MDGModifier* m_MDGModifier_ColorSet;
    MDGModifier* m_MDGModifier_PolyColor;
};

class ArikaraJointDisplay
{
public:
    ArikaraJointDisplay();
    ~ArikaraJointDisplay() {};

    static MColor selectedColor;
   
    void setNewObject(MObject& newSelectedJoint);
    void resetSceneState();
    void restoreArikaraDisplay();

    void clear();

private:
    int    m_useObjectColor;

    MColor  m_oldColor;
    int     m_objectColor;

    MObject m_SelectedJoint;

    /*Plug and dep.
    */
    MFnDependencyNode m_MfnDep;
    MPlug m_PlugWireColor;
    MPlug m_PlugUseObjectColor;
    MPlug m_PlugObjectColor;
};

class ArikaraSkin
{
public:
	ArikaraSkin();
	~ArikaraSkin();

	MDagPath transformDagPath;
	MDagPath geoDagPath;
	MObject  skinClusterMObject;

	bool useSoftSelection;
	bool useLockVertex;

	int currentInfluence;

	void selectVertexAffectedByCurrentInfluences();

	void SetVertexWeight(float value);
	void AddVertexWeight(float value);


	/*Set the object to edit from the specified MDagPath*/
	void SetObject(MDagPath pDagPath, MObject pSkin);
	
	/*Set the object to edit from the maya viewport selection.*/
	bool SetObjectFromSelection();

	/*Set the vertex to edit from the maya viewport selection.*/
	bool SetVertexFromSelection(unsigned int* vertexCount = nullptr);

	/*Get the average weights for the current vertex that was set previously with SetVertex.*/
	bool GetAverageWeightForAllInfluences(std::vector<double> &avgWeight, unsigned int *pInfluencesCount = nullptr);
    bool GetAverageWeightForInfluences(MIntArray& influences, std::vector<double> &avgWeight);

	double GetAverageWeightForCurrentInfluences();

	bool isSkinValid();

	bool LockSelectedVerts(unsigned int* vertexCount = nullptr);
    bool UnlockSelectedVerts(unsigned int* vertexCount = nullptr);
	bool ClearLockedVertex();

	unsigned int GetLockedVertexCount();
	unsigned int GetVertexSelectedCount();

	void selectLockedVert();

	const char* GetCurrentInfluenceName();

	void setInfluenceFromName(const MString &name);

    void isInfluencesLocked(std::vector<bool>& locked);

    /* 
     * Method that modify the influence list.
    */
	bool addSelectedInfluences();
	bool removeInfluences(std::vector<unsigned int>& pInfluencesIndex);
	bool removeUnusedInfluence(double minVal);

    void clear();

    void operator=(const ArikaraSkin&);

    friend class InteractiveEdit;
    //friend class SkinModeBase;
    friend class VertexInfluenceData;

    InteractiveEdit iEdit;

    /*
    * Callback function.
    */
    void OnInfluenceChanged(int pInfluenceIndex);
    void OnGeometryChanded(const MDagPath& pNewGeo);
    void OnToolShow();
    void OnToolClose();
    void OnSceneNew();
    void OnSceneClose();
    void OnSceneSavedBefore();
    void OnSceneSavedAfter();

    void OnInfluenceListChanged();

    friend class ArikaraGeometryDisplay;

    MDagPathArray InfluencesList;
    unsigned int  InfluencesCount;

private:
	MObject mVertexSelection;
	unsigned int mVertexCount;
	std::vector<float> mSoftSelectionWeight;

	MObject mLockedVertex;
	unsigned int mLockedVertexCount;
	std::vector<float> mLockedVertexWeight;

	void updateVertexSelectionWeightWithLock();

    void clean();

    ArikaraJointDisplay m_AriJointDisplay;
    ArikaraGeometryDisplay m_GeoDisplay;
};