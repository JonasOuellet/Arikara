#include "VertexInfluenceData.h"

#include "../ArikaraMaya/include/ArikaraSkin.h"

VertexInfluenceData::VertexInfluenceData()
{

}

void VertexInfluenceData::GenerateData(ArikaraSkin* pArikara)
{
    unsigned int vertexCount = pArikara->mVertexCount;
    unsigned int influenceCount = pArikara->iEdit.influenceCount;
    unsigned int currentInfluence = static_cast<unsigned int>(pArikara->currentInfluence);
    
    vertData.resize(vertexCount);

    std::vector<bool> lockedInfluence;
    pArikara->isInfluencesLocked(lockedInfluence);

    MDoubleArray& baseWeight = pArikara->iEdit.baseWeights;

    for (unsigned int v = 0; v < vertexCount; v++)
    {
        for (unsigned int i = 0; i < influenceCount; i++)
        {
            if (!lockedInfluence[i])
            {
                if (i != currentInfluence)
                {
                    if (baseWeight[v * influenceCount + i] > 0.0)
                    {
                        vertData[v].remainingWeight += baseWeight[v * influenceCount + i];
                        vertData[v].influencesCount++;
                        vertData[v].i.push_back(i);
                    }
                }
            }
            else
            {
                vertData[v].lockedWeight += baseWeight[v * influenceCount + i];
            }

        }
        vertData[v].isValid = vertData[v].influencesCount > 0;
    }
}

const VertexInfluData& VertexInfluenceData::operator[](unsigned int index)
{
    return vertData[index];
}

void VertexInfluenceData::clear()
{
    vertData.clear();
}
