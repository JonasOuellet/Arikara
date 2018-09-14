#include "SkinModeBase.h"
#include "InteractiveEdit.h"
#include "../ArikaraMaya/include/ArikaraSkin.h"
#include "../ArikaraMaya/include/MayaSkinFunction.h"

#include <map>

#include <qtwidgets/qslider>


SkinModeBase::SkinModeBase(const char* modeName) : 
    m_modeName(modeName)
{

}

const char* SkinModeBase::getModeName()
{
    return m_modeName;
}

void SkinModeBase::init(InteractiveEdit& d)
{
}

void SkinModeBase::edit(InteractiveEdit& d)
{

   for (unsigned int v = 0; v < d.vertexCount; v++)
    {
        if (d.vData[v].isValid)
        {
            double lockedWeight = d.vData[v].lockedWeight;
            unsigned int index = v * d.influenceCount + d.currentInfluence;
            double newVal = ((d.newValue - d.baseWeights[index]) * d.softSelection[v]) + d.baseWeights[index];
            
            if (newVal + lockedWeight > 1.0)
                newVal = 1.0 - lockedWeight;
            else if (newVal < 0.0)
                newVal = 0.0;

            d.newWeights[index] = newVal;
            //double ratio = 1.0 - newVal - d.vData[v].lockedWeight;
            //double rw = 1.0 / d.vData[v].remainingWeight;
            double mult = (1.0 - newVal - lockedWeight) * (1.0 / d.vData[v].remainingWeight);
            for (unsigned int i = 0; i < d.vData[v].influencesCount; i++)
            {
                int idx = v * d.influenceCount + d.vData[v].i[i];
                d.newWeights[idx] = d.baseWeights[idx] * mult;
            }
        }
    }
}

void SkinModeBase::end(InteractiveEdit&)
{

}

void SkinModeBase::setWeight(double pVal)
{

}

void SkinModeBase::updateSlider(QSlider* pSlider, ArikaraSkin* pArikara)
{
    int avgWeight = static_cast<int>(pArikara->GetAverageWeightForCurrentInfluences() * 100.0);
    pSlider->setRange(0, 100);
    pSlider->setValue(avgWeight);
}

SkinModeBase* AbsoluteSkinMode::creator()
{
    return new AbsoluteSkinMode;
}

void RelativeSkinMode::edit(InteractiveEdit& d)
{
    for (unsigned int v = 0; v < d.vertexCount; v++)
    {
        if (d.vData[v].isValid)
        {
            double lockedWeight = d.vData[v].lockedWeight;
            unsigned int index = v * d.influenceCount + d.currentInfluence;
            double newVal = d.baseWeights[index] + (d.delta * d.softSelection[v]);

            if (newVal + lockedWeight > 1.0)
                newVal = 1.0 - lockedWeight;
            else if (newVal < 0.0)
                newVal = 0.0;

            d.newWeights[index] = newVal;
            //double ratio = 1.0 - newVal;
            //double rw = d.vData[v].remainingWeight;
            double mult = (1.0 - newVal - lockedWeight) * (1.0 / d.vData[v].remainingWeight);
            for (unsigned int i = 0; i < d.vData[v].influencesCount; i++)
            {
                int idx = v * d.influenceCount + d.vData[v].i[i];
                d.newWeights[idx] = d.baseWeights[idx] * mult;
            }
        }
    }
}

void RelativeSkinMode::updateSlider(QSlider* pSlider, ArikaraSkin* pArikara)
{
    int avgWeight = static_cast<int>(pArikara->GetAverageWeightForCurrentInfluences() * 100.0);
    pSlider->setRange(0, 100);
    pSlider->setValue(avgWeight);
}

SkinModeBase* RelativeSkinMode::creator()
{
    return new RelativeSkinMode;
}

void ScaleSkinMode::edit(InteractiveEdit& d)
{
    for (unsigned int v = 0; v < d.vertexCount; v++)
    {
        if (d.vData[v].isValid)
        {
            double lockedWeight = d.vData[v].lockedWeight;
            unsigned int index = v * d.influenceCount + d.currentInfluence;
            double newVal = d.baseWeights[index] * d.newValue * d.softSelection[v];

            if (newVal + lockedWeight > 1.0)
                newVal = 1.0 - lockedWeight;
            else if (newVal < 0.0)
                newVal = 0.0;

            d.newWeights[index] = newVal;

            //double ratio = 1.0 - newVal;
            //double rw = d.vData[v].remainingWeight;

            double mult = (1.0 - newVal - lockedWeight) * (1.0 / d.vData[v].remainingWeight);

            for (unsigned int i = 0; i < d.vData[v].influencesCount; i++)
            {
                int idx = v * d.influenceCount + d.vData[v].i[i];
                d.newWeights[idx] = d.baseWeights[idx] * mult;
            }
        }
    }
}

void ScaleSkinMode::updateSlider(QSlider* pSlider, ArikaraSkin* pArikara)
{
    pSlider->setRange(0, 200);
    pSlider->setValue(100);
}

SkinModeBase* ScaleSkinMode::creator()
{
    return new ScaleSkinMode;
}

void RigidSkinMode::init(InteractiveEdit& d)
{
    unsigned int influCount = d.influenceCount;
    unsigned int vertCount = d.vertexCount;
    m_TargetWeights = MDoubleArray(influCount, 0.0);


    double val = 1.0 / vertCount;
    for (unsigned int v = 0; v < d.vertexCount; v++)
    {
        for (unsigned int i = 0; i < influCount; i++)
        {
            m_TargetWeights[i] += d.baseWeights[v * influCount + i];
        }
    }

    for (unsigned int i = 0; i < influCount; i++)
    {
        m_TargetWeights[i] *= val;
    }

    int maxInfluences;
    if (ArikaraSkinUtils::getSkinMaxInfluence(d.skinClusterObj, maxInfluences))
    {
        int weightCount = 0;
        std::multimap<double, unsigned int> weightMap;
        for (unsigned int i = 0; i < influCount; i++)
        {
            double curWeight = m_TargetWeights[i];
            if (curWeight > 0.0)
            {
                weightCount++;
                weightMap.insert(std::pair<double, unsigned int>(curWeight, i));
            }
        }

        if (weightCount > maxInfluences)
        {
            int x = weightCount;
            double w = 1.0;
            std::multimap<double, unsigned int>::reverse_iterator rIter = weightMap.rbegin();
            for (; x > maxInfluences; x--, rIter++)
            {
                w -= rIter->first;
                m_TargetWeights[rIter->second] = 0.0;
            }
            for (; x > 0; x--, rIter++)
            {
                m_TargetWeights[rIter->second] /= w;
            }
        }
    }
}

void RigidSkinMode::edit(InteractiveEdit& d)
{
    unsigned int influCount = d.influenceCount;
    double ratio = 1.0 - d.newValue;
    double val = d.newValue;
    for (unsigned int v = 0; v < d.vertexCount; v++)
    {
        for (unsigned int i = 0; i < d.influenceCount; i++)
        {
            unsigned int index = v * influCount + i;
            double baseWeight = d.baseWeights[index];
            d.newWeights[index] = (baseWeight * ratio) + (m_TargetWeights[i] * val);
        }
    }
}

void RigidSkinMode::end(InteractiveEdit&)
{
    m_TargetWeights.clear();
}

void RigidSkinMode::updateSlider(QSlider* pSlider, ArikaraSkin* pArikara)
{
    pSlider->setRange(0, 100);
    pSlider->setValue(0);
}

SkinModeBase* RigidSkinMode::creator()
{
    return new RigidSkinMode;
}
