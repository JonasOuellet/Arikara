#pragma once

#include <vector>

class ArikaraSkin;

struct VertexInfluData
{
    double remainingWeight = 0.0;
    double lockedWeight = 0.0;
    unsigned int influencesCount = 0;
    std::vector<unsigned int> i;

    bool isValid;
};

class VertexInfluenceData 
{
public:
    VertexInfluenceData();

    void GenerateData(ArikaraSkin* pArikara);

    const VertexInfluData& operator[](unsigned int index);

    std::vector<VertexInfluData> vertData;

    void clear();

private:

};
