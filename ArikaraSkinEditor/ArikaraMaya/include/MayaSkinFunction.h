#pragma once

#include <vector>
#include <string>

#include <maya/MDagPathArray.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MMatrixArray.h>
#include <maya/MDagPath.h>
#include <maya/MItDependencyGraph.h>

class MDGModifier;

namespace ArikaraSkinUtils
{
	bool hasSkinCluster(MDagPath& pObject, MObject& pSkin);

	bool getObjectWithSkinFromSelection(MDagPathArray& pDagPathArray);

	bool GetFirstObjectWithSkinFromSelection(MDagPath& pObject, MObject& pSkin);

	void SaveSkin(MDagPath& pGeo);

	void LoadSkin();

	unsigned int getInfluencesNames(const MObject &pSkinCluster, std::vector<std::string>&);

	/*Return either the influence is locked or not for all the influence.*/
	void isInfluencesLocked(const MObject &pSkinCluster, std::vector<bool>&);

	MStatus getInfluenceIndex(const MObject &pSkinCluster, const MString&, unsigned int*);

	void lockInfluence(const MDagPath& pInfluences, bool lockStatus = true);

	void lockInfluence(const MObject &pSkinCluster, unsigned int index, bool lockStatus = true);

	void lockInfluence(const MObject &pSkinCluster, const std::vector<unsigned int> &index, bool lockStatus = true);

	int getMaxInfluence(const MObject &pSkinCluster);

	void setMaxInfluence(const MObject &pSkinCluster, int maxInfluence);

	void resetBindPose(const MObject &pSkinCluster, MMatrixArray* oldPose = nullptr);

	bool transferWeight(const MDagPath &pObject, const MObject &pComp, const MObject &pSkinCluster,
		const MDagPath &pSourceInfluence, const MDagPath &pTargetInfluence, MDoubleArray *pOldWeight = nullptr, double pValue = 1.0f);

	bool transferWeightForSelectedObject(const char* pSourceInfluenceName, const char* pTargetInfluenceName, double pValue = 1.0f);

    /*Return an array of int representing the logical index of the influences.
    */
    bool getInfluenceLogicalIndex(const MObject& p_Skincluster, MIntArray& p_LogicalIndex);

    /*Set weights for all vertex and influences.
    The lengths of p_weights must be (vertexCount * influencesCount) and aligned by components.
    Must call MDGModifier.doit() after this function call. otherwise nothing will happen.
    */
    bool setWeights(const MObject& p_Skincluster, const MDoubleArray& p_Weights, MDGModifier& p_DGModifier);

    /* Get the specified skin cluster object maxInfluences.
     * @params skinObject (MObject)  the skin cluster mobject to check.
     * @params maxInfluence (int) -out- the max Influences count;
     * @return (bool)  true if maintaint max influence is set false otherwise.
    */
    bool getSkinMaxInfluence(const MObject& skinObject, int& maxInfluence);
}

namespace ArikaraSelection
{
	bool GetSoftSelectedVertexForObject(const MDagPath &pObject, MObject &pComponent, unsigned int *vertexCount = nullptr, std::vector<float> *weights = nullptr);

	/*Get the selected Vertex Component for the specified Dagpath.  If face or edge component are selected
	conversion will be done to return a vertex Selection.
	*/
	bool GetSelectedVertexForObject(const MDagPath &pObject, MObject &pComponent, unsigned int *vertexCount = nullptr);

	/*Get the selected Vertex Component and its object dagpath given the index of the selection.
	If face or edge component are selected conversion will be done to return a vertex Selection.
	*/
	bool GetSelectedObjectAndVertex(unsigned int index, const MSelectionList &sel, MDagPath& pObject, MObject &pComponent);

	bool GetSelectedFacesForObject(const MDagPath &pObject, MObject &pComponent);

	unsigned int ConvertFacesToVertex(const MDagPath &pObject, const MObject &pFaceComponent, MObject &pVertexComponent);
    unsigned int ConvertEdgesToVertex(const MDagPath &pObject, const MObject &pEdgeComponent, MObject &pVertexComponent);

	std::string GetSelectedObjectName();

    bool ConvertComponentToVertex(const MDagPath &pObject, const MObject &pCurrentComponent, MObject &pVertexComponent,
        unsigned int * vertexCount = nullptr);
}
