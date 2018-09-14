#include "ArikaraAttachSkinCmd.h"
#include "../../GlobalDefine.h"

#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMeshIntersector.h>
#include <maya/MPointArray.h>
#include <maya/MPoint.h>
#include <maya/MItMeshPolygon.h>

#include "../include/MayaSkinFunction.h"
#include "../include/ArikaraMath.h"
#include "../include/ArikaraSkinEditorCmd.h"

#include <vector>
#include <algorithm>

double clamp(double x, double lower, double upper)
{
    return std::min(upper, std::max(x, lower));
}


struct joMeshTest
{
    joMeshTest (MFnMesh& mfnMesh){
        mfnMesh.getTriangles(triangleCount, triangleVertices);
        unsigned int numface = triangleCount.length();
        offset.setLength(numface);
        int off = 0;
        offset[0] = 0;
        for (unsigned int x = 1; x < numface; x++)
        {
            off += triangleCount[x - 1] * 3;
            offset[x] = off;
        }
    }
    MIntArray triangleCount;
    MIntArray triangleVertices;
    MIntArray offset;
};




struct triangeData {
    unsigned int id0;
    unsigned int id1;
    unsigned int id2;

    MVector normal;
};

struct faceWeightInfo
{
    int numPoint;
    std::vector<double> pointWeights;
    std::vector<int> pointIndex;
};

struct faceData {
    faceData(MFnMesh& p_MfnMesh, int pFaceIdx, MPointArray& p_ptsData):
        r(0.0), points(p_ptsData)
    {
        p_MfnMesh.getPolygonVertices(pFaceIdx, pI);
        numPoint = pI.length();
        /*Calculate the face center.
        */
        for (unsigned int x = 0; x < pI.length(); x++)
        {
            faceCenter += p_ptsData[pI[x]];
        }
        faceCenter = faceCenter / pI.length();
        maxDist = 0.0;
        int count = 0;
        for (unsigned int x = 0; x < numPoint; x++)
        {
            for (unsigned int y = 0; y < numPoint; y++)
            {
                if (x != y)
                {
                    count++;
                    maxDist += points[pI[x]].distanceTo(points[pI[y]]);
                }
            }
        }
        maxDist /= count;

        for (unsigned int x = 0; x < pI.length(); x++)
        {
            r += faceCenter.distanceTo(p_ptsData[pI[x]]);
        }
        r /= pI.length();
        r /= 2;

        //int counter = 0;
        /*for (int x = 0; x < n; x++)
        {
        for (int y = 0; y < n; y++)
        {
        if (x != y)
        {
        r0 += testPtsArray[pts[x]].distanceTo(testPtsArray[pts[y]]);
        counter++;
        }
        }
        }*/

        //r0 /= counter;


        tri.clear();
        /*build the triangle list.
        */
        if (numPoint > 3)
        {
            tri.resize(numPoint);
            numTri = numPoint;
            for (unsigned int x = 0; x < numPoint; x++)
            {
                unsigned int id0 = x;
                unsigned int id1 = x + 1;
                if (id1 >= numPoint)
                {
                    id1 = id1 - numPoint;
                }
                unsigned int id2 = x + 2;
                if (id2 >= numPoint)
                {
                    id2 = id2 - numPoint;
                }

                MPoint& p0 = points[pI[id0]];
                MPoint& p1 = points[pI[id1]];
                MPoint& p2 = points[pI[id2]];

                tri[x].normal = ((p1 - p0) ^ (p2 - p0)).normal();

                tri[x].id0 = id0; tri[x].id1 = id1; tri[x].id2 = id2;

                /*double scaleParam = 10.0;
                tri[x].p0n = ((p0 - p1) + (p0 - p2)).normal() * scaleParam;
                tri[x].p1n = ((p1 - p2) + (p1 - p0)).normal() * scaleParam;
                tri[x].p2n = ((p2 - p1) + (p2 - p0)).normal() * scaleParam;*/
            }
        }
        else if (numPoint == 3)
        {
            tri.resize(1);
            numTri = 1;
            tri[0].id0 = 0; tri[0].id1 = 1; tri[0].id2 = 2;

            tri[0].normal = ((points[pI[1]] - points[pI[0]])
                ^ (points[pI[2]] - points[pI[0]])).normal();
        }
    }

    void getPointsWeightsTest2(MPoint& p1, faceWeightInfo& fInfo)
    {
        fInfo.pointWeights.clear();
        fInfo.pointWeights.resize(numPoint, 0.0);
        fInfo.numPoint = numPoint;
        fInfo.pointIndex.resize(numPoint);

        for (unsigned int x = 0; x < numPoint; x++)
        {
            fInfo.pointIndex[x] = pI[x];
        }

        if (numPoint == 3)
        {
            MPoint& A = points[pI[tri[0].id0]];
            MPoint& B = points[pI[tri[0].id1]];
            MPoint& C = points[pI[tri[0].id2]];
            double u, v, w;
            if (ArikaraMayaMath::BarycentricCoordinate(A, B, C, p1, u, v, w))
            {
                fInfo.pointWeights[0] = u;
                fInfo.pointWeights[1] = v;
                fInfo.pointWeights[2] = w;
            }
        }
        else
        {
            unsigned int count = 0;
            double totalWeights = 0.0;
            for (unsigned int i = 0; i < numPoint; i++)
            {
                unsigned int id0 = i;
                unsigned int id1 = i + 1;
                if (id1 >= numPoint)
                    id1 = id1 - numPoint;

                unsigned int id2_0 = i + 2;
                if (id2_0 >= numPoint)
                    id2_0 = id2_0 - numPoint;

                unsigned int id2_1 = i + 3;
                if (id2_1 >= numPoint)
                    id2_1 = id2_1 - numPoint;

                MPoint& A = points[pI[id0]];
                MPoint& B = points[pI[id1]];
                MPoint C = (points[pI[id2_0]] + points[pI[id2_1]]) / 2;

                MVector normal = ((B - A) ^ (C - A)).normal();

                MPoint inter = ArikaraMayaMath::LinePlaneIntersection(p1, normal, A, B, C);
                double u, v, w;
                if (ArikaraMayaMath::BarycentricCoordinate(A, B, C, inter, u, v, w))
                {
                    count++;
                    totalWeights += u + v + w;
                    fInfo.pointWeights[id0] += u;
                    fInfo.pointWeights[id1] += v;
                    fInfo.pointWeights[id2_0] += w * 0.5;
                    fInfo.pointWeights[id2_1] += w * 0.5;
                }
            }
            for (unsigned int i = 0; i < numPoint; i++)
            {
                fInfo.pointWeights[i] /= totalWeights;
            }
        }
    }

    void getPointsWeightsTest(MPoint& p1, faceWeightInfo& fInfo)
    {
        fInfo.pointWeights.clear();
        fInfo.pointWeights.resize(numPoint, 0.0);
        fInfo.numPoint = numPoint;
        fInfo.pointIndex.resize(numPoint);
        for (unsigned int x = 0; x < numPoint; x++)
        {
            fInfo.pointIndex[x] = pI[x];
        }

        if (numPoint == 3)
        {
            MPoint& A = points[pI[tri[0].id0]];
            MPoint& B = points[pI[tri[0].id1]];
            MPoint& C = points[pI[tri[0].id2]];
            double u, v, w;
            if (ArikaraMayaMath::BarycentricCoordinate(A, B, C, p1, u, v, w))
            {
                fInfo.pointWeights[0] = u;
                fInfo.pointWeights[1] = v;
                fInfo.pointWeights[2] = w;
            }
        }
        else
        {
            double totalWeight = 0.0;
            for (unsigned int x = 0; x < numPoint; x++)
            {
                double w = 1.0 - (clamp(points[pI[x]].distanceTo(p1), 0.0, maxDist) / maxDist);
                fInfo.pointWeights[x] = w;
                totalWeight += w;
            }
            for (unsigned int x = 0; x < numPoint; x++)
                fInfo.pointWeights[x] /= totalWeight;

        }
    }

    void getPointsWeights(MPoint& p1, faceWeightInfo& fInfo)
    {
        fInfo.pointWeights.clear();
        fInfo.pointWeights.resize(numPoint, 0.0);

        unsigned int count = 0;

        for (unsigned int x = 0; x < numTri; x++)
        {
            MPoint& A = points[pI[tri[x].id0]];
            MPoint& B = points[pI[tri[x].id1]];
            MPoint& C = points[pI[tri[x].id2]];

            MPoint inter = ArikaraMayaMath::LinePlaneIntersection(p1, tri[x].normal, A, B, C);

            double u, v, w;
            if (ArikaraMayaMath::BarycentricCoordinate(A, B, C, inter, u, v, w))
            {
                count++;
                fInfo.pointWeights[tri[x].id0] += u;
                fInfo.pointWeights[tri[x].id1] += v;
                fInfo.pointWeights[tri[x].id2] += w;
            }
        }

        for (unsigned int x = 0; x < numPoint; x++)
        {
            fInfo.pointWeights[x] /= count;
        }

        fInfo.numPoint = numPoint;
        fInfo.pointIndex.resize(numPoint);
        for (unsigned int x = 0; x < numPoint; x++)
        {
            fInfo.pointIndex[x] = pI[x];
        }
    }

    void getDistance(MPoint& pts, std::vector<double>& dist)
    {
        dist.resize(numPoint);

        for (unsigned int x = 0; x < numPoint; x++)
        {
            dist[x] = pts.distanceTo(points[pI[x]]);
        }
    }

    void normalize(std::vector<double>& value)
    {
        double totalVal = 0.0;
        for (unsigned int x = 0; x < numPoint; x++)
        {
            totalVal += value[x];
        }
        for (unsigned int x = 0; x < numPoint; x++)
        {
            value[x] /= totalVal;
        }
    }

    void getValue1(MPoint& pts, std::vector<double>& value)
    {
        std::vector<double> dist;
        getDistance(pts, dist);

        unsigned int i;
        for (i = 0; i < numPoint; i++)
        {
            value[i] = sqrt(dist[i] * dist[i] + r * r);
        }

        normalize(value);
    }

    void getValue2(MPoint& pts, std::vector<double>& value)
    {
        std::vector<double> dist;
        getDistance(pts, dist);

        unsigned int i;
        for (i = 0; i < numPoint; i++)
        {
            value[i] = 1.0 / sqrt(dist[i] * dist[i] + r * r);
        }

        normalize(value);
    }

    void getValue3(MPoint& pts, std::vector<double>& value)
    {
        std::vector<double> dist;
        getDistance(pts, dist);

        unsigned int i;
        for (i = 0; i < numPoint; i++)
        {
            if (dist[i] <= 0.0)
            {
                value[i] = 0.0;
            }
            else
            {
                value[i] = dist[i] * dist[i] * log(dist[i] / r);
            }
        }

        normalize(value);
    }

    void getValue4(MPoint& pts, std::vector<double>& value)
    {
        value.clear();
        value.resize(pI.length());

        std::vector<double> dist;
        getDistance(pts, dist);

        unsigned int i;
        for (i = 0; i < numPoint; i++)
        {
            value[i] = exp(-0.5 * dist[i] * dist[i] / r / r);
        }

        normalize(value);
    }

    void getValue(MPoint& pts, faceWeightInfo& pInfo, int pRbfType = 1)
    {
        pInfo.numPoint = numPoint;
        pInfo.pointIndex.resize(numPoint);
        for (unsigned int x = 0; x < numPoint; x++)
            pInfo.pointIndex[x] = pI[x];

        pInfo.pointWeights.resize(numPoint);
        
        switch (pRbfType)
        {
        case 1:
            getValue1(pts, pInfo.pointWeights);
            break;
        case 2:
            getValue2(pts, pInfo.pointWeights);
            break;
        case 3:
            getValue3(pts, pInfo.pointWeights);
            break;
        case 4:
            getValue4(pts, pInfo.pointWeights);
            break;
        default:
            break;
        }
    }

    unsigned int numPoint;
    MIntArray pI;
    MPoint faceCenter;
    double r;
    MPointArray &points;

    std::vector<triangeData> tri;
    unsigned int numTri;

    double maxDist;
};

struct meshData {
    meshData(MFnMesh& mesh, MPointArray& pts):
        points(pts)
    {
        int numPolygon = mesh.numPolygons();
        face.clear();
        face.reserve(numPolygon);
        for (int x = 0; x < numPolygon; x++)
        {
            face.push_back(faceData(mesh, x, pts));
        }
    };

    std::vector<faceData> face;
    MPointArray& points;
};


ArikaraAttachSkinCmd::ArikaraAttachSkinCmd() :
    m_UseDifferentTarget(false),
    m_UseTargetComponent(false),
    m_UseSourceComponent(false),
    m_SourceHasSkinCluster(false),
    m_UseTargetFlag(false),
    rbfType(-1)
{

}

ArikaraAttachSkinCmd::~ArikaraAttachSkinCmd()
{

}

MStatus ArikaraAttachSkinCmd::doIt(const MArgList& args)
{
    MStatus status;
    MSyntax mySyntax = newSyntax();
    MArgDatabase parsed(mySyntax, args);

    m_UseTargetFlag = parsed.isFlagSet(kUseTargetFlag);

    MSelectionList commandSelection;
    parsed.getObjects(commandSelection);

    status = commandSelection.getDagPath(0, m_SourceGeoDagPath, m_SourceComponent); ReturnOnError(status);
    if (m_SourceComponent.isNull())
    {
        m_SourceGeoDagPath.extendToShape();
    }
    /*Check source object is affected by a skinCluster.
    */
    MItDependencyGraph dgIt(m_SourceGeoDagPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
    if (!dgIt.isDone())
    {
        m_SourceSkinClusterObj = dgIt.currentItem();
        m_SourceHasSkinCluster = true;
    }

    if (m_SourceComponent.isNull())
    {
        /*No vertex Selected so we can't attach to its own surface.
        Check if there is a target set.
        */
        if (ArikaraAttachSkinTargetCmd::isTargetSet && 
            !(m_SourceGeoDagPath == ArikaraAttachSkinTargetCmd::TargetGeoDagPath))
        {
            m_UseTargetComponent = ArikaraAttachSkinTargetCmd::isUsingComponent;
            m_TargetComponent = ArikaraAttachSkinTargetCmd::TargetComponent;
            m_TargetSkinClusterObj = ArikaraAttachSkinTargetCmd::TargetSkinClusterObject;
            m_TargetGeoDagPath = ArikaraAttachSkinTargetCmd::TargetGeoDagPath;
            m_UseDifferentTarget = true;
            m_UseSourceComponent = false;
        }
        else
        {
            displayError("No target specified, please use command \"arikaraAttachSkinTarget\" to set a target.");
            return MS::kInvalidParameter;
        }
    }
    else
    {
        if (m_SourceComponent.apiType() != MFn::Type::kMeshPolygonComponent)
        {
            displayError("Must make polygon component selection");
            return MS::kInvalidParameter;
        }

        m_UseSourceComponent = true;
        /* If there is component selected, the source object must originally have a skinCluster.
        */
        if (!m_SourceHasSkinCluster)
        {
            displayError("Must select an object with a skinCluster.");
            return MS::kInvalidParameter;
        }

        if (m_UseTargetFlag)
        {
            if (ArikaraAttachSkinTargetCmd::isTargetSet)
            {
                m_UseDifferentTarget = !(m_SourceGeoDagPath == ArikaraAttachSkinTargetCmd::TargetGeoDagPath);

                m_TargetGeoDagPath = ArikaraAttachSkinTargetCmd::TargetGeoDagPath;
                m_UseTargetComponent = ArikaraAttachSkinTargetCmd::isUsingComponent;
                m_TargetSkinClusterObj = ArikaraAttachSkinTargetCmd::TargetSkinClusterObject;
                m_TargetComponent = ArikaraAttachSkinTargetCmd::TargetComponent;
            }
            else
            {
                displayError("You must specify a target first with command: \"arikaraAttachSkin\".");
                return MS::kInvalidParameter;
            }
        }
        else
        {
            m_TargetGeoDagPath = m_SourceGeoDagPath;
            m_TargetSkinClusterObj = m_SourceSkinClusterObj;
            m_UseDifferentTarget = false;
        }
    }

    if (parsed.isFlagSet("rbf"))
    {
        rbfType = parsed.flagArgumentInt("rbf", 0);
    }

    return redoIt();
}

MStatus ArikaraAttachSkinCmd::redoIt()
{
    MStatus status;

    MFnDependencyNode mfnSourceSkinDep;

    /*Get the target mesh in the skinCluster.
    and create a copy.
    */
    MFnDependencyNode mfnTargetSkinDep(m_TargetSkinClusterObj);
    MPlug inputPlug = mfnTargetSkinDep.findPlug("input", &status); ReturnOnError(status);
    inputPlug = inputPlug.elementByLogicalIndex(0, &status); ReturnOnError(status);
    MPlug targetGeoPlug = inputPlug.child(0, &status); ReturnOnError(status);

    /*Get the source mesh.
    */
    MFnMesh sourceMfnMesh;
    if (m_SourceHasSkinCluster)
    {
        MFnDependencyNode mfnSourceSkinDep(m_SourceSkinClusterObj);
        MPlug inplug = mfnSourceSkinDep.findPlug("input", &status); ReturnOnError(status);
        inplug = inplug.elementByLogicalIndex(0, &status); ReturnOnError(status);
        inplug = inplug.child(0, &status); ReturnOnError(status);
        sourceMfnMesh.setObject(inplug.asMObject());
    }
    else
    {
        sourceMfnMesh.setObject(m_SourceGeoDagPath);
    }
    bool isNewGeo = false;
    MObject l_NewGeoObj;
    MFnMesh targetMfnMesh;
    MObject targetObjectMesh;
    MObject l_SourceVertexComponent;
    MSpace::Space vertexSpace = MSpace::kObject;

    if (m_UseSourceComponent) {
        ArikaraSelection::ConvertComponentToVertex(m_SourceGeoDagPath, m_SourceComponent, l_SourceVertexComponent);
    }
    else {
        MFnSingleIndexedComponent comp;
        l_SourceVertexComponent = comp.create(MFn::Type::kMeshVertComponent);
        comp.setCompleteData(sourceMfnMesh.numVertices());
    }

    if (m_UseDifferentTarget)
    {
        vertexSpace = MSpace::kWorld;
        if (!m_UseTargetComponent)
        {
            targetMfnMesh.setObject(targetGeoPlug.asMObject());
            l_NewGeoObj = targetGeoPlug.asMObject();

            MFnSingleIndexedComponent comp;
            m_TargetComponent = comp.create(MFn::Type::kMeshPolygonComponent);
            comp.setCompleteData(targetMfnMesh.numPolygons());
        }
        else
        {
            if (createANewGeo(targetGeoPlug.asMObject(), targetMfnMesh, vertexSpace))
            {
                isNewGeo = true;
                l_NewGeoObj = targetMfnMesh.object();
            }
            else
            {
                displayError("Error initializing geometry.");
                return status;
            }
        }
    }
    else
    {
        targetMfnMesh.setObject(targetGeoPlug.asMObject());

        MFnSingleIndexedComponent comp;
        MObject newTargetCompObject = comp.create(MFn::Type::kMeshPolygonComponent);

        MFnSingleIndexedComponent sourceComp(m_SourceComponent);
        MFnSingleIndexedComponent targetComp;
        if (!m_UseTargetComponent)
        {
            m_TargetComponent = targetComp.create(MFn::Type::kMeshPolygonComponent);
            targetComp.setCompleteData(targetMfnMesh.numPolygons());
        }
        else
        {
            targetComp.setObject(m_TargetComponent);
        }

        /* Remove Component that are present in the source selection.
        */
        int s = 0;
        int t = 0;
        int tComp = targetComp.element(t);
        int sComp = sourceComp.element(s);
        int numComp = sourceComp.elementCount();
        for (; t < targetComp.elementCount();)
        {
            if (tComp < sComp){
                comp.addElement(tComp);
                t++;
                tComp = targetComp.element(t);
            }
            else if (tComp == sComp){
                t++; s++; sComp = sourceComp.element(s); tComp = targetComp.element(t);
            }
            else if (tComp > sComp && s < numComp) {
                s++; sComp = sourceComp.element(s);
            }
            else{
                comp.addElement(tComp);
                t++;
                tComp = targetComp.element(t);
            }
        }
        m_TargetComponent = newTargetCompObject;
        if (createANewGeo(targetGeoPlug.asMObject(), targetMfnMesh, vertexSpace))
        {
            isNewGeo = true;
            l_NewGeoObj = targetMfnMesh.object();
        }
        else
        {
            displayError("Error initializing geometry.");
            return status;
        }
    }

    /*Initialize skin
    */
    MFnSkinCluster targetSkin(m_TargetSkinClusterObj);
    MFnSkinCluster sourceSkin;

    MDagPathArray l_TargetInfluenceDag;
    unsigned int l_TargetInfluenceCount = targetSkin.influenceObjects(l_TargetInfluenceDag);
    unsigned int l_sourceInfuenceCount = 0;
    MIntArray matchingIndex(l_TargetInfluenceCount);
    if (m_SourceHasSkinCluster)
    {
        sourceSkin.setObject(m_SourceSkinClusterObj);
        mfnSourceSkinDep.setObject(m_SourceSkinClusterObj);
        MDagPathArray sourceInflu;
        l_sourceInfuenceCount = sourceSkin.influenceObjects(sourceInflu);

        mfnSourceSkinDep.findPlug("maintainMaxInfluences").setBool(false);
        if (m_UseDifferentTarget)
        {
            /*@TODO: To do in MDG Modifier
            1.find matching infuences index.
            2.Add influence if it is not in the skin cluster.
            3.Set same matrix;
            4.Set maxinfluences.
            */
            
            //1.find matching infuences index.
            MDagPathArray influenceToAdd;
            
            unsigned int maxIter = l_sourceInfuenceCount;
            for (unsigned int i = 0; i < l_TargetInfluenceCount; i++)
            {
                MDagPath& curDag(l_TargetInfluenceDag[i]);
                bool found = false;
                for (unsigned int x = 0; x < maxIter; x++)
                {
                    if (curDag == sourceInflu[x])
                    {
                        matchingIndex[i] = x;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    influenceToAdd.append(curDag);
                    matchingIndex[i] = l_sourceInfuenceCount;
                    l_sourceInfuenceCount++;
                }
            }

            if (influenceToAdd.length() > 0)
            {
                MString command = "arikaraInfluence -add ";
                for (unsigned int x = 0; x < influenceToAdd.length(); x++)
                {
                    command += influenceToAdd[x].partialPathName();
                    command += " ";
                }
                MDagPath trans = m_SourceGeoDagPath;
                trans.pop();
                command += trans.partialPathName(); 
                m_MDGModifer.commandToExecute(command);
                m_MDGModifer.doIt();

                /*Set the same bindMatrix.
                */
                MPlug sourceBindMatPlug = mfnSourceSkinDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);
                MPlug targetBindMatPlug = mfnTargetSkinDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);
                MFnMatrixData matData;

                for (unsigned int x = 0; x < influenceToAdd.length(); x++)
                {
                    unsigned int targetIndex = targetSkin.indexForInfluenceObject(influenceToAdd[x]);
                    unsigned int sourceIndex = sourceSkin.indexForInfluenceObject(influenceToAdd[x]);
                    MObject mat = targetBindMatPlug.elementByLogicalIndex(targetIndex).asMObject();

                    sourceBindMatPlug.elementByLogicalIndex(sourceIndex).setValue(mat);
                }
            }
        }
        else
        {
            matchingIndex.setLength(l_sourceInfuenceCount);
            for (unsigned int x = 0; x < l_sourceInfuenceCount; x++)
            {
                matchingIndex[x] = x;
            }
        }
    }
    else
    {
        /*Create a new skin cluster with the influence of the target skincluster.
        */
        MDagPath trans = m_SourceGeoDagPath;
        trans.pop();

        MString command = "skinCluster -tsb " + trans.partialPathName();
        for (unsigned int i = 0; i < l_TargetInfluenceCount; i++)
        {
            command += " " + l_TargetInfluenceDag[i].partialPathName();
            matchingIndex[i] = i;
        }

        status = m_MDGModifer.commandToExecute(command);
        m_MDGModifer.doIt();

        //get the skinCluster;
        MItDependencyGraph dgIt(m_SourceGeoDagPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
        if (!dgIt.isDone())
        {
            m_SourceSkinClusterObj = dgIt.currentItem();
            mfnSourceSkinDep.setObject(m_SourceSkinClusterObj);
            sourceSkin.setObject(m_SourceSkinClusterObj);
        }

        /*Set the same bindMatrix.
        */
        MPlug sourceBindMatPlug = mfnSourceSkinDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);
        MPlug targetBindMatPlug = mfnTargetSkinDep.findPlug("bindPreMatrix", &status); ReturnOnError(status);
        MFnMatrixData matData;
        for (unsigned int x = 0; x < l_TargetInfluenceCount; x++)
        {
            
            unsigned int targetIndex = targetSkin.indexForInfluenceObject(l_TargetInfluenceDag[x]);
            MObject mat = targetBindMatPlug.elementByLogicalIndex(targetIndex).asMObject();

            sourceBindMatPlug.elementByPhysicalIndex(x).setValue(mat);
        }

        /*Set max influences.
        */
        mfnSourceSkinDep.findPlug("mi").setInt(mfnTargetSkinDep.findPlug("mi").asInt());
        /*mfnSourceSkinDep.findPlug("maintainMaxInfluences").setBool(
            mfnTargetSkinDep.findPlug("maintainMaxInfluences").asBool());*/
        mfnSourceSkinDep.findPlug("maintainMaxInfluences").setBool(false);
        l_sourceInfuenceCount = l_TargetInfluenceCount;
    }

    MFnSingleIndexedComponent targetCompMfnWeight;
    MObject l_targetWeightComp = targetCompMfnWeight.create(MFn::Type::kMeshVertComponent);
    targetCompMfnWeight.setComplete(true);

    MDoubleArray targetSkinWeights;
    targetSkin.getWeights(m_TargetGeoDagPath, l_targetWeightComp, targetSkinWeights, l_TargetInfluenceCount);

    MFnSingleIndexedComponent sourceCompMfn(l_SourceVertexComponent);
    MDoubleArray sourceSkinWeights(sourceCompMfn.elementCount() * l_sourceInfuenceCount, 0.0);

    joMeshTest l_MeshData(targetMfnMesh);

    MMeshIntersector meshInter;
    if (vertexSpace == MSpace::kWorld && !isNewGeo)
    {
        MDagPath trans = m_TargetGeoDagPath;
        trans.pop();
        MFnDependencyNode targetDep(trans.node());
        MObject mat;
        mat = targetDep.findPlug("worldMatrix").elementByLogicalIndex(0).asMObject();
        //mat = targetDep.findPlug("worldInverseMatrix").elementByLogicalIndex(0).asMObject();
        MFnMatrixData matData;
        matData.setObject(mat);
        MMatrix matrix = matData.matrix();

        status = meshInter.create(l_NewGeoObj, matrix);
        if (status != MS::kSuccess)
        {
            MGlobal::displayError("Error while init MMeshIntersector");
            return status;
        }
    }
    else
    {
        status = meshInter.create(l_NewGeoObj);
        if (status != MS::kSuccess)
        {
            MGlobal::displayError("Error while init MMeshIntersector");
            return status;
        }
    }

    faceWeightInfo fInfo;
    fInfo.numPoint = 3;
    fInfo.pointIndex.resize(3);
    fInfo.pointWeights.resize(3);

    for (int v = 0; v < sourceCompMfn.elementCount(); v++)
    {
        int curVertId = sourceCompMfn.element(v);
        MPoint p;
        sourceMfnMesh.getPoint(curVertId, p, vertexSpace);

        //faceWeightInfo faceWeigh;
        //l_MeshData.getClosestFacePointsWeights(p, faceWeigh);

        /*MPoint test2222;
        int closestPolygon;
        targetMfnMesh.getClosestPoint(p, test2222, MSpace::kObject, &closestPolygon);*/

        MPointOnMesh pMesh;
        meshInter.getClosestPoint(p, pMesh);

        int fIdx = pMesh.faceIndex();
        int tI = pMesh.triangleIndex();
        int off = l_MeshData.offset[fIdx] + tI * 3;
        fInfo.pointIndex[0] = l_MeshData.triangleVertices[off];
        fInfo.pointIndex[1] = l_MeshData.triangleVertices[off + 1];
        fInfo.pointIndex[2] = l_MeshData.triangleVertices[off + 2];

        float bu;
        float bv;
        pMesh.getBarycentricCoords(bu, bv);
        fInfo.pointWeights[0] = bu;
        fInfo.pointWeights[1] = bv;
        fInfo.pointWeights[2] = 1.0 - bu - bv;


        //faceWeightInfo fInfo;

        //closestPolygon = pMesh.faceIndex();
        //test2222 = pMesh.getPoint();
        /*if (rbfType == -1)
        {
            l_MeshData.face[closestPolygon].getPointsWeightsTest2(test2222, fInfo);
        }
        else if (rbfType == 0)
        {
            l_MeshData.face[closestPolygon].getPointsWeightsTest(test2222, fInfo);
        }
        else
        {
            l_MeshData.face[closestPolygon].getValue(test2222, fInfo, rbfType);
        }*/


        for (unsigned int i = 0; i < l_TargetInfluenceCount; i++)
        {
            double w = 0.0;
            for (int x = 0; x < fInfo.numPoint; x++)
            {
                w += targetSkinWeights[fInfo.pointIndex[x] * l_TargetInfluenceCount + i]
                    * fInfo.pointWeights[x];
            }
            sourceSkinWeights[v * l_sourceInfuenceCount + matchingIndex[i]] = w;
        }

    }

    m_influencesIndex.setLength(l_sourceInfuenceCount);
    for (unsigned int i = 0; i < l_sourceInfuenceCount; i++)
        m_influencesIndex[i] = i;

    sourceSkin.setWeights(m_SourceGeoDagPath, m_SourceComponent, m_influencesIndex, sourceSkinWeights, true, &m_oldWeights);
    
    if (isNewGeo)
    {
        MFnDagNode tmpDag(l_NewGeoObj);
        MObject tras = tmpDag.parent(0);
        status = MGlobal::deleteNode(l_NewGeoObj);
        status = MGlobal::deleteNode(tras);
    }

    l_NewGeoObj = MObject::kNullObj;
    targetObjectMesh = MObject::kNullObj;
    l_SourceVertexComponent = MObject::kNullObj;

    ArikaraSkinEditorCmd::updateToolUI();

    return MS::kSuccess;
}

MStatus ArikaraAttachSkinCmd::undoIt()
{
    MFnSkinCluster sourceSkin(m_SourceSkinClusterObj);
    sourceSkin.setWeights(m_SourceGeoDagPath, m_SourceComponent, m_influencesIndex, m_oldWeights);
   
    m_MDGModifer.undoIt();

    ArikaraSkinEditorCmd::updateToolUI();

    return MS::kSuccess;
}

void* ArikaraAttachSkinCmd::creator()
{
    return new ArikaraAttachSkinCmd;
}

MSyntax ArikaraAttachSkinCmd::newSyntax()
{
    MSyntax mySyntax;

    mySyntax.setObjectType(MSyntax::MObjectFormat::kSelectionList, 1);
    mySyntax.useSelectionAsDefault(true);
    
    mySyntax.addFlag(kUseTargetFlag, kUseTargetFlagLong, MSyntax::kNoArg);
    mySyntax.addFlag("rbf", "radialFunction", MSyntax::kLong);

    return mySyntax;
}


bool ArikaraAttachSkinCmd::createANewGeo(MObject& sourceGeo, MFnMesh& targetMFn, MSpace::Space pSpace)
{
    //create a new Geometry
    MStatus status;
    MFnMesh tmpMesh(sourceGeo, &status);
    if (status == MS::kSuccess)
    {
        MFnSingleIndexedComponent vertComp;
        MObject tmpvertComp = vertComp.create(MFn::Type::kMeshVertComponent);

        MFnSingleIndexedComponent targetFaceComp(m_TargetComponent);

        int numVertices = tmpMesh.numVertices();
        int numPolygon = targetFaceComp.elementCount();
        MPointArray pointArray;
        MIntArray polygonCount(numPolygon);
        MIntArray polygonConnect;

        status = tmpMesh.getPoints(pointArray, pSpace); ReturnOnErrorBool(status);
        for (int p = 0; p < targetFaceComp.elementCount(); p++)
        {
            MIntArray verts;
            tmpMesh.getPolygonVertices(targetFaceComp.element(p), verts);
            for (unsigned int x = 0; x < verts.length(); x++)
                polygonConnect.append(verts[x]);
            polygonCount[p] = verts.length();
        }

        targetMFn.create(numVertices, numPolygon, pointArray, polygonCount, polygonConnect);
        return true;
    }
    return false;
}

MStatus ArikaraAttachSkinTargetCmd::doIt(const MArgList& args)
{
    MStatus status;
    MSyntax mySyntax = newSyntax();
    MArgDatabase parsed(mySyntax, args);

    MSelectionList skinNodeSel;
    parsed.getObjects(skinNodeSel);
    status = skinNodeSel.getDagPath(0, TargetGeoDagPath, TargetComponent);
    if (status == MS::kSuccess)
    {
        if (TargetComponent.isNull())
        {
            TargetGeoDagPath.extendToShape();
            isUsingComponent = false;
        }
        else
        {
            if (TargetComponent.apiType() != MFn::Type::kMeshPolygonComponent)
            {
                isUsingComponent = false;
                isTargetSet = false;
                TargetSkinClusterObject = MObject();
                TargetGeoDagPath = MDagPath();
                TargetComponent = MObject();
                displayError("Must make polygon component selection");
                return MS::kInvalidParameter;
            }
            isUsingComponent = true;
        }

        //check if there is a skin cluster on the object.
        MItDependencyGraph dgIt(TargetGeoDagPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
		if (!dgIt.isDone())
		{
			TargetSkinClusterObject = dgIt.currentItem();
		}
		else
		{
            isUsingComponent = false;
            isTargetSet = false;
            TargetSkinClusterObject = MObject();
            TargetGeoDagPath = MDagPath();
            TargetComponent = MObject();

            displayError("Must specified a Target with a skinCluster.");
			return MS::kInvalidParameter;
		}

        isTargetSet = true;
    }
    return status;
}

MSyntax ArikaraAttachSkinTargetCmd::newSyntax()
{
    MSyntax mySyntax;

    mySyntax.setObjectType(MSyntax::MObjectFormat::kSelectionList, 1);
    mySyntax.useSelectionAsDefault(true);

    return mySyntax;
}

bool ArikaraAttachSkinTargetCmd::isTargetSet = false;
bool ArikaraAttachSkinTargetCmd::isUsingComponent = false;
MObject ArikaraAttachSkinTargetCmd::TargetComponent;
MDagPath ArikaraAttachSkinTargetCmd::TargetGeoDagPath;
MObject ArikaraAttachSkinTargetCmd::TargetSkinClusterObject;
