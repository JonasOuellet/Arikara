#pragma once
#include <maya/MDoubleArray.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MIntArray.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>

#include "VertexInfluenceData.h"
#include "SkinModeBase.h"

#include <vector>

class ArikaraSkin;

class InteractiveEdit
{
public:
    InteractiveEdit(ArikaraSkin* pArikara);
    ~InteractiveEdit();

    // slider Weights;
    void Init(double);
    void Edit(double);
    void Close();

    MDoubleArray   baseWeights;
    MDoubleArray   newWeights;
    MIntArray      influencesArray;
    MFnSkinCluster mfnSkin;
    MObject        skinClusterObj;
    MDagPath       geoDagPath;

    VertexInfluenceData vData;
    
    unsigned int influenceCount;
    unsigned int vertexCount;
    unsigned int currentInfluence;

    SkinModeBase* currentSkinMode;
    
    double baseValue;
    double newValue;
    double delta;

    const std::vector<float>& softSelection;

    bool isValid() {
        return m_isInteractiveValid;
    };

    static bool RegisterMode(SkinModeBase* (*func)());
    static void ClearMode();
    static void GetModesNames(std::vector<const char*>& p_container);
    
    bool SetCurrentMode(int p_index);
    bool SetCurrentMode(const char* p_ModeName);

    int GetCurrentModeIndex();

    void clear();

private:
    ArikaraSkin* m_Arikara;

    bool m_isInteractiveValid;


    static std::vector<SkinModeBase*> modes;
};