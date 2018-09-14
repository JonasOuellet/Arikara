#pragma once
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MDagPath.h>
#include <maya/MObject.h>
#include <maya/MMatrix.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>

#include <vector>

#define kARISDSaveFlag            "-s"
#define kARISDSaveFlagLong        "-save"
#define kARISDLoadFlag            "-l"
#define kARISDLoadFlagLong        "-load"
#define kARISDFileFlag            "-f"
#define kARISDFileFlagLong        "-file"
#define kARISDBindMatFlag         "-lbm"
#define kARISDBindMatFlagLong     "-loadBindMatrix"
#define kARISDCleanSkinFlag       "-c"
#define kARISDCleanSkinFlagLong   "-clean"

#include "../../json.hpp"
using json = nlohmann::json;

namespace ariJson
{
	class ariVertex
	{
	public:
		ariVertex();
		~ariVertex();

		double position[3];
		std::vector<unsigned int> influences;
		std::vector<double> weights;

		void setJsonData(json& pJson);
		void initFromData(json& pJson);
	};

	class ariInfluence
	{
	public:
		ariInfluence() {};
		ariInfluence(const MString& pName, const MMatrix &pBindMatrix);
		ariInfluence(const MObject&, const MDagPath&);
		~ariInfluence();

		std::string name;
		double bindMat[4][4];
		void setJsonData(json& pJson);
		void initFromData(const json& pJson);
	};

	class ariSkinCluster
	{
	public:
		ariSkinCluster() {};
		ariSkinCluster(MObject &pSkinCluster, MDagPath& pGeo);
		ariSkinCluster(const json& pJson);
		~ariSkinCluster() {};

		std::string name;
		unsigned int maxInfluences;

		unsigned int influencesCount;
		std::vector<ariInfluence> influences;

		unsigned int vertexCount;
		std::vector<ariVertex> vertices;

		void setJsonData(json& pJson);

		double geoMatrix[4][4];

		void initFromData(const json& pJson);

	private:
		MObject m_skinCluster;

	};
}

class ArikaraSkinData : public MPxCommand
{
public:
	ArikaraSkinData();
	virtual ~ArikaraSkinData();

	MStatus doIt(const MArgList&);
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const { return true; }

	static		void* creator();
	static MSyntax newSyntax();

	static MString pathName;
	static MString fileName;

	static MString DefaultPath();
private:

	MStatus saveSkinData();
	MStatus loadSkinData();

	void loadCleanSkinData(ariJson::ariSkinCluster& skinJson);
	void loadSkinData(ariJson::ariSkinCluster& skinJson, MIntArray &influIndex, unsigned int& totalNewInfluences);

	MDagPath m_GeoDagPath;
	MObject m_SkinCluster;

	bool m_SaveFlag;
	bool m_LoadFlag;
	bool m_LoadBindMatrixFlag;
	bool m_CleanFlag;


	bool m_isSkinClusterCreated;

	MDGModifier m_MDGModifier;
	MDagModifier m_MDagModifier;
	MDGModifier m_MDGModMatrix;

	MDoubleArray m_oldWeights;
	MIntArray m_InfluenceArr;

	MObject m_components;

};