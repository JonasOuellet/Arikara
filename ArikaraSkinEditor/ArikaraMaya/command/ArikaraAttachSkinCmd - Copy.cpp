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

#include "../include/MayaSkinFunction.h"
#include "../include/ArikaraMath.h"

#include <vector>

struct triangeData {
    int id0;
    int id1;
    int id2;
    MVector normal;

    MVector p0n;
    MVector p1n;
    MVector p2n;

   // bool set
};

struct faceData {
    faceData(MFnMesh& p_MfnMesh, int pFaceIdx, MPointArray& p_ptsData) 
        : faceCenter(0.0, 0.0, 0.0), ptsData(p_ptsData), numTri(0)
    {
        p_MfnMesh.getPolygonVertices(pFaceIdx, pI);
        pointCount = pI.length();

        /*Calculate the face center.
        */
        for (int x = 0; x < pointCount; x++)
        {
            faceCenter += ptsData[pI[x]];
        }
        faceCenter = faceCenter / pointCount;

        tri.clear();

        /*build the triangle list.
        */
        if (pointCount > 3)
        {
            tri.resize(pointCount);
            numTri = pointCount;
            for (int x = 0; x < pointCount; x++)
            {
                int id0 = x;
                int id1 = x + 1;
                if (id1 >= pointCount)
                {
                    id1 = id1 - pointCount;
                }
                int id2 = x + 2;
                if (id2 >= pointCount)
                {
                    id2 = id2 - pointCount;
                }

                MPoint& p0 = ptsData[pI[id0]];
                MPoint& p1 = ptsData[pI[id1]];
                MPoint& p2 = ptsData[pI[id2]];

                tri[x].normal = ((p1 - p0) ^ (p2 - p0)).normal();

                tri[x].id0 = id0; tri[x].id1 = id1; tri[x].id2 = id2;

                /*double scaleParam = 10.0;
                tri[x].p0n = ((p0 - p1) + (p0 - p2)).normal() * scaleParam;
                tri[x].p1n = ((p1 - p2) + (p1 - p0)).normal() * scaleParam;
                tri[x].p2n = ((p2 - p1) + (p2 - p0)).normal() * scaleParam;*/
            }
        }
        else if (pointCount == 3)
        {
            tri.resize(1);
            numTri = 1;
            tri[0].id0 = 0; tri[0].id1 = 1; tri[0].id2 = 2;

            tri[0].normal = ((ptsData[pI[1]] - ptsData[pI[0]])
                ^ (ptsData[pI[2]] - ptsData[pI[0]])).normal();
        }
    };

    void getPointsWeights(MPoint& p, std::vector<double>& weights)
    {
        weights.clear();
        weights.resize(pointCount, 0.0);

        unsigned int count = 0;

        for (unsigned int x = 0; x < numTri; x++)
        {
            MPoint A = ptsData[pI[tri[x].id0]];
            MPoint B = ptsData[pI[tri[x].id1]];
            MPoint C = ptsData[pI[tri[x].id2]];

            MPoint p2 = p + (tri[x].normal * 10.0);

            MPoint inter = ArikaraMayaMath::LinePlaneIntersection(p, p2, A, B, C);

            double u, v, w;
            if (ArikaraMayaMath::BarycentricCoordinate(A, B, C, inter, u, v, w))
            {
                count++;
                weights[tri[x].id0] += u;
                weights[tri[x].id1] += v;
                weights[tri[x].id2] += w;
            }
        }

        for (int x = 0; x < pointCount; x++)
        {
            weights[x] /= count;
        }
    }

    MPointArray& ptsData;
    MPoint faceCenter;
    MIntArray pI;
    int pointCount;
    std::vector<triangeData> tri;
    unsigned int numTri;
};

struct faceWeightInfo
{
    int numPoint;
    std::vector<double> pointWeights;
    std::vector<int> pointIndex;
};

struct meshData {
    meshData(MFnMesh& mesh) 
    {
        mesh.getPoints(points/*, MSpace::kWorld*/);
        numPolygon = mesh.numPolygons();
        face.reserve(numPolygon);

        for (int x = 0; x < numPolygon; x++)
        {
            face.push_back(faceData(mesh, x, points));
        }
    };

    faceData& findClosestFace(MPoint& pPoint, double bias = 0.005)
    {
        double closestDistance = 1000000.0;
        int closestIndex = -1;

        for (int x = 0; x < numPolygon; x++)
        {
            double dist = pPoint.distanceTo(face[x].faceCenter);

            if (dist < bias)
            {
                return face[x];
            }
            else if (dist < closestDistance)
            {
                closestDistance = dist;
                closestIndex = x;
            }
        }

        return face[closestIndex];
    }

    void getClosestFacePointsWeights(MPoint& pPoints, faceWeightInfo& pInfo) 
    {
        faceData& face = findClosestFace(pPoints);

        int ptsCount = face.pointCount;
        pInfo.numPoint = ptsCount;
        pInfo.pointIndex.resize(ptsCount);
        for (int x = 0; x < ptsCount; x++)
        {
            pInfo.pointIndex[x] = face.pI[x];
        }
        face.getPointsWeights(pPoints, pInfo.pointWeights);
    }

    std::vector<faceData> face;
    MPointArray points;
    int numPolygon;
};

void calculateFaceCenterPos(MObject mesh, MPointArray& averagePos)
{
    MFnMesh l_MfnMesh(mesh);
    int numPoly = l_MfnMesh.numPolygons();
    averagePos.setLength(numPoly);
    for (int x = 0; x < numPoly; x++)
    {
        
    }
}


ArikaraAttachSkinCmd::ArikaraAttachSkinCmd():
    m_UseDifferentTarget(false),
    m_UseTargetComponent(false),
    m_UseSourceComponent(false),
    m_SourceHasSkinCluster(false),
    m_UseTargetFlag(false),
    m_UseTargetComponentFlag(false)
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
    m_UseTargetComponentFlag = parsed.isFlagSet(kUseTargetComponentFlag);

    MSelectionList commandSelection;
    parsed.getObjects(commandSelection);

    MObject tmpComponent;
    status = commandSelection.getDagPath(0, m_SourceGeoDagPath, tmpComponent); ReturnOnError(status);
    if (tmpComponent.isNull())
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

    if (tmpComponent.isNull())
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
        m_UseSourceComponent = true;
        /* If there is component selected, the source object must originally have a skinCluster.
        */
        if (!m_SourceHasSkinCluster)
        {
            displayError("Must select an object with a skinCluster.");
            return MS::kInvalidParameter;
        }

        ArikaraSelection::ConvertComponentToVertex(m_SourceGeoDagPath, tmpComponent, m_SourceComponent);

        if (m_UseTargetFlag)
        {
            if (ArikaraAttachSkinTargetCmd::isTargetSet)
            {
                m_UseDifferentTarget = !(m_SourceGeoDagPath == ArikaraAttachSkinTargetCmd::TargetGeoDagPath);

                m_TargetGeoDagPath = ArikaraAttachSkinTargetCmd::TargetGeoDagPath;
                m_UseTargetComponent = ArikaraAttachSkinTargetCmd::isUsingComponent;
                m_TargetSkinClusterObj = ArikaraAttachSkinTargetCmd::TargetSkinClusterObject;
                if (m_UseTargetComponent && m_UseTargetComponentFlag)
                {
                    m_TargetComponent = ArikaraAttachSkinTargetCmd::TargetComponent;
                }
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
    inputPlug = inputPlug.child(0, &status); ReturnOnError(status);

    MFnMesh targetMfnMesh;
    MObject targetMeshObject = targetMfnMesh.copy(inputPlug.asMObject());

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

    /*remove component that are not specified for the computation.
    */
    if (m_UseTargetComponent)
    {
        MFnSingleIndexedComponent mfnComp(m_TargetComponent);
        for (int v = 0, c = 0;
            v < targetMfnMesh.numVertices() && c < mfnComp.elementCount();)
        {
            if (v < mfnComp.element(c))
            {
                /* current vertex is is smaller than the vertex in selection.
                   Delete the vertex.
                */
                targetMfnMesh.deleteVertex(v);
                v++;
            }
            else
            {
                /*They are equal so update iter.
                */
                c++;
                v++;
            }
        }
    }
    else
    {
        MFnSingleIndexedComponent comp;
        m_TargetComponent = comp.create(MFn::Type::kMeshVertComponent);
        comp.setCompleteData(targetMfnMesh.numVertices());
    }

    if (!m_UseDifferentTarget)
    {
        /*Remove source form the target component.
        */
        MFnSingleIndexedComponent mfnSourceComp(m_SourceComponent, &status);
        if (status == MS::kSuccess)
        {
            for (int x = 0; x < mfnSourceComp.elementCount(); x++)
            {
                targetMfnMesh.deleteVertex(mfnSourceComp.element(x));
            }
        }

        /* Remove selected component from target Comp.
        */
        MFnSingleIndexedComponent comp;
        MObject tmpComp = comp.create(MFn::Type::kMeshVertComponent);
        MFnSingleIndexedComponent mfnComp(m_TargetComponent);
        int t = 0;
        for (int s = 0; t < mfnComp.elementCount() &&
            s < mfnSourceComp.elementCount();)
        {
            int tI = mfnComp.element(t);
            int sI = mfnSourceComp.element(s);
            if (tI < sI)
            {
                t++;
                comp.addElement(tI);
            }
            else if (tI > sI)
            {
                s++;
            }
            else
            {
                //both are equal
                s++; t++;
            }
        }
        for (; t < mfnComp.elementCount(); t++)
            comp.addElement(mfnComp.element(t));

        m_TargetComponent = tmpComp;
    }
    else
    {
        /* Set complete data if not using source component;
        */
        if (!m_UseSourceComponent)
        {
            MFnSingleIndexedComponent comp;
            m_SourceComponent = comp.create(MFn::Type::kMeshVertComponent);
            comp.setCompleteData(sourceMfnMesh.numVertices());
        }
    }

    /*Initialize skin
    */
    MFnSkinCluster targetSkin(m_TargetSkinClusterObj);
    MFnSkinCluster sourceSkin;

    MDagPathArray l_TargetInfluenceDag;
    unsigned int l_TargetInfluenceCount = targetSkin.influenceObjects(l_TargetInfluenceDag);
    unsigned int l_sourceInfuenceCount = 0;
    MIntArray matchingIndex;
    if (m_SourceHasSkinCluster)
    {
        sourceSkin.setObject(m_SourceSkinClusterObj);
        mfnSourceSkinDep.setObject(m_SourceSkinClusterObj);
        
        if (m_UseDifferentTarget)
        {
            /*@TODO: To do in MDG Modifier
            1.find matching infuences index.
            2.Add influence if it is not in the skin cluster.
            3.Set same matrix;
            4.Set maxinfluences.
            */
        }
    }
    else
    {
        /*Create a new skin cluster with the influence of the target skincluster.
        */
        MDagPath trans = m_SourceGeoDagPath;
        trans.pop();

        matchingIndex.setLength(l_TargetInfluenceCount);

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
        mfnSourceSkinDep.findPlug("maintainMaxInfluences").setBool(
            mfnTargetSkinDep.findPlug("maintainMaxInfluences").asBool());

        l_sourceInfuenceCount = l_TargetInfluenceCount;
    }
    MDoubleArray targetSkinWeights;
    targetSkin.getWeights(m_TargetGeoDagPath, m_TargetComponent, targetSkinWeights, l_TargetInfluenceCount);

    MFnSingleIndexedComponent sourceCompMfn(m_SourceComponent);
    MDoubleArray sourceSkinWeights(sourceCompMfn.elementCount() * l_sourceInfuenceCount, 0.0);


    int test = targetMfnMesh.numPolygons();
    int test2 = targetMfnMesh.numVertices();
    MPointArray testPtsArray;
    targetMfnMesh.getPoints(testPtsArray);

    //meshData l_MeshData(targetMfnMesh);

    MMeshIntersector meshInter;
    meshInter.create(targetMfnMesh.object());


    for (int v = 0; v < sourceCompMfn.elementCount(); v++)
    {
        int curVertId = sourceCompMfn.element(v);
        MPoint p;
        sourceMfnMesh.getPoint(curVertId, p/*, MSpace::kWorld*/);

        //faceWeightInfo faceWeigh;
        //l_MeshData.getClosestFacePointsWeights(p, faceWeigh);

        MPoint test2222;
        int closestPolygon;
        targetMfnMesh.getClosestPoint(p, test2222, MSpace::kObject, &closestPolygon);

        MIntArray pts;
        targetMfnMesh.getPolygonVertices(closestPolygon, pts);

        MPointOnMesh pMesh;
        meshInter.getClosestPoint(p, pMesh);

        int n = pts.length();
        double* r = new double[n];
        double* val = new double[n];
        
        
        for (int i = 0; i < n; i++)
        {
            r[i] = test2222.distanceTo(testPtsArray[pts[i]]);
        }
        
        double r0 = 0.0;
        int counter = 0;
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

        r0 /= counter;
        double totalWeights = 0.0;
        for (int i = 0; i < n; i++)
        {
            val[i] = exp(-0.5 * r[i] * r[i] / r0 / r0);
            totalWeights += val[i];
        }

        for (int i = 0; i < n; i++)
        {
            val[i] /= totalWeights;
        }

        for (unsigned int i = 0; i < l_TargetInfluenceCount; i++)
        {
            double w = 0.0;
            for (int x = 0; x < n; x++)
            {
                w += targetSkinWeights[pts[x] * l_TargetInfluenceCount + i]
                    * val[x];
            }
            sourceSkinWeights[curVertId * l_sourceInfuenceCount + matchingIndex[i]] = w;
        }


        delete[] r;
        delete[] val;
        /*for (unsigned int i = 0; i < l_TargetInfluenceCount; i++)
        {
            double w = 0.0;
            for (int x = 0; x < faceWeigh.numPoint; x++)
            {
                w += targetSkinWeights[faceWeigh.pointIndex[x] * l_TargetInfluenceCount + i] 
                    * faceWeigh.pointWeights[x];
            }
            sourceSkinWeights[curVertId * l_sourceInfuenceCount + matchingIndex[i]] = w;
        }*/

        /*
        for (int x = 0; x < faceWeigh.numPoint; x++)
        {
            unsigned int curTargetVId = faceWeigh.pointIndex[x];
            double curWeight = faceWeigh.pointWeights[x];
            for (unsigned int i = 0; i < l_TargetInfluenceCount; i++)
            {
                sourceSkinWeights[curVertId * l_sourceInfuenceCount + matchingIndex[i]] =
                    targetSkinWeights[curTargetVId * l_TargetInfluenceCount + i] * curWeight;
            }
        }*/

    }

    m_influencesIndex.setLength(l_sourceInfuenceCount);
    for (unsigned int i = 0; i < l_sourceInfuenceCount; i++)
        m_influencesIndex[i] = i;

    sourceSkin.setWeights(m_SourceGeoDagPath, m_SourceComponent, m_influencesIndex, sourceSkinWeights, false, &m_oldWeights);
    MGlobal::deleteNode(targetMeshObject);

    return MS::kSuccess;
}

MStatus ArikaraAttachSkinCmd::undoIt()
{
    MFnSkinCluster sourceSkin(m_SourceSkinClusterObj);
    sourceSkin.setWeights(m_SourceGeoDagPath, m_SourceComponent, m_influencesIndex, m_oldWeights);
   
    m_MDGModifer.undoIt();

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
    mySyntax.addFlag(kUseTargetComponentFlag, kUseTargetComponentFlagLong, MSyntax::kNoArg);

    return mySyntax;
}


MStatus ArikaraAttachSkinTargetCmd::doIt(const MArgList& args)
{
    MStatus status;
    MSyntax mySyntax = newSyntax();
    MArgDatabase parsed(mySyntax, args);

    MSelectionList skinNodeSel;
    parsed.getObjects(skinNodeSel);
    MObject tmpComponent;
    status = skinNodeSel.getDagPath(0, TargetGeoDagPath, tmpComponent);
    if (status == MS::kSuccess)
    {
        if (TargetComponent.isNull())
        {
            TargetGeoDagPath.extendToShape();
            isUsingComponent = false;
        }
        else
        {
            isUsingComponent = ArikaraSelection::ConvertComponentToVertex(TargetGeoDagPath, tmpComponent, TargetComponent);
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
