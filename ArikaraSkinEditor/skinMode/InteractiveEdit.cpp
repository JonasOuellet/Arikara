#include "InteractiveEdit.h"

#include <maya/MGlobal.h>

#include "../ArikaraMaya/include/ArikaraSkin.h"
#include "../ArikaraMaya/command/ArikaraSkinEditorSetWeightCmd.h"

#include <string.h>

InteractiveEdit::InteractiveEdit(ArikaraSkin* pArikara) :
    m_Arikara(pArikara),
    m_isInteractiveValid(false),
    baseValue(0.0),
    newValue(0.0),
    delta(0.0),
    softSelection(pArikara->mSoftSelectionWeight)
{
    /*Register mode.
    */
    InteractiveEdit::RegisterMode(&(AbsoluteSkinMode::creator));
    InteractiveEdit::RegisterMode(&(RelativeSkinMode::creator));
    InteractiveEdit::RegisterMode(&(ScaleSkinMode::creator));
    InteractiveEdit::RegisterMode(&(RigidSkinMode::creator));
}


InteractiveEdit::~InteractiveEdit()
{
    ClearMode();
}

void InteractiveEdit::Init(double pVal)
{
    if (!(m_Arikara->isSkinValid() && m_Arikara->mVertexCount > 0 &&
            m_Arikara->currentInfluence != -1 && currentSkinMode != nullptr))
        return;

    geoDagPath = m_Arikara->geoDagPath;
    skinClusterObj = m_Arikara->skinClusterMObject;

    baseValue = pVal;
    currentInfluence = static_cast<unsigned int>(m_Arikara->currentInfluence);

    MGlobal::executeCommand("arikaraSkinEditorSetWeight", false, true);
    if (!ArikaraSkinEditorSetWeightCmd::initCommand(m_Arikara->geoDagPath, 
        m_Arikara->skinClusterMObject, m_Arikara->mVertexSelection))
        return;

    mfnSkin.setObject(m_Arikara->skinClusterMObject);
    mfnSkin.getWeights(m_Arikara->geoDagPath, m_Arikara->mVertexSelection,
        baseWeights, influenceCount);

    newWeights = baseWeights;

    vertexCount = m_Arikara->mVertexCount;

    if (currentSkinMode->needVertexData())
    {
        vData.GenerateData(m_Arikara);
    }

    influencesArray.setLength(influenceCount);
    for (unsigned int x = 0; x < influenceCount; x++)
    {
        influencesArray[x] = x;
    }

    currentSkinMode->init(*this);

    m_isInteractiveValid = true;
}

void InteractiveEdit::Edit(double value)
{
    newValue = value;
    delta = value - baseValue;
    currentSkinMode->edit(*this);
    mfnSkin.setWeights(m_Arikara->geoDagPath, m_Arikara->mVertexSelection, influencesArray, newWeights);

    /*MString message;
    double totalWeight = 0.0;
    for (unsigned int x = 0; x < newWeights.length(); x++)
    {
        totalWeight += newWeights[x];
        message += newWeights[x];
        message += "\n";
    }
    message += "Total Weights: ";
    message += totalWeight;
    message += "\n--------------------";
    MGlobal::displayInfo(message);
    */
    //m_Arikara->m_GeoDisplay.updateColor();
}

void InteractiveEdit::Close()
{
    if (m_isInteractiveValid)
    {
        newValue = 0.0;
        baseValue = 0.0;
        influenceCount = 0;
        vData.clear();
        influencesArray.clear();
        newWeights.clear();
        baseWeights.clear();

        vertexCount = 0;

        m_isInteractiveValid = false;
        ArikaraSkinEditorSetWeightCmd::closeCommand();

        currentSkinMode->end(*this);
    }
}

bool InteractiveEdit::RegisterMode(SkinModeBase* (*func)())
{

    SkinModeBase* newMode = (*func)();
    
    const char* newModeName = newMode->getModeName();
    bool found = false;
    for (SkinModeBase* existingMode : modes)
    {
        if (std::strcmp(existingMode->getModeName(), newModeName) == 0)
        {
            found = true;
            break;
        }
    }
    if (found)
    {
        delete newMode;
        return false;
    }

    modes.push_back(newMode);
    return true;
}

void InteractiveEdit::ClearMode()
{
    for (SkinModeBase* existingMode : modes)
    {
        delete existingMode;
    }

    modes.clear();
}

void InteractiveEdit::GetModesNames(std::vector<const char*>& p_container)
{
    p_container.clear();

    for (SkinModeBase* existingMode : modes)
    {
        p_container.push_back(existingMode->getModeName());
    }
}

bool InteractiveEdit::SetCurrentMode(int p_index)
{
    if (p_index >= 0 && p_index < static_cast<int>(modes.size()))
    {
        currentSkinMode = modes[p_index];
        return true;
    }
    return false;
}

bool InteractiveEdit::SetCurrentMode(const char* p_ModeName)
{
    for (SkinModeBase* existingMode : modes)
    {
        if (std::strcmp(existingMode->getModeName(), p_ModeName) == 0)
        {
            currentSkinMode = existingMode;
            return true;
        }
    }
    return false;
}

int InteractiveEdit::GetCurrentModeIndex()
{
    for (int x = 0; x < static_cast<int>(modes.size()); x++)
    {
        if (currentSkinMode == modes[x])
        {
            return x;
        }
    }
    return -1;
}

void InteractiveEdit::clear()
{
    mfnSkin.setObject(MObject::kNullObj);
    skinClusterObj = MObject::kNullObj;
    geoDagPath = MDagPath();
}

std::vector<SkinModeBase*> InteractiveEdit::modes;
