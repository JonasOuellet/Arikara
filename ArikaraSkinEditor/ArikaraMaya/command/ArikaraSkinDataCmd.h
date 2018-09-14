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
#define kARISDPathFlag            "-p"
#define kARISDPathFlagLong        "-path"
#define kARISDBindMatFlag         "-lbm"
#define kARISDBindMatFlagLong     "-loadBindMatrix"
#define kARISDCleanSkinFlag       "-c"
#define kARISDCleanSkinFlagLong   "-clean"
#define kARISDLoadByPosFlag       "-pos"
#define kARISDLoadByPosFlagLong   "-position"
#define kARISDWorldSpaceFlag      "-ws"
#define kARISDWorldSpaceFlagLong  "-worldSpace"
#define kARISDSuffixFlag          "-suf"
#define kARISDSuffixFlagLong      "-suffix"
#define kARISDPrefixFlag          "-pre"
#define kARISDPrefixFlagLong      "-prefix"
#define kARISDBiasFlag            "-b"
#define kARISDBiasFlagLong        "-bias"

class MFnMesh;

namespace ariSkinData
{
	class ariVertex
	{
	public:
		ariVertex() : count(0) {};
		~ariVertex() { 
			delete[] influences;
			delete[] weights;
		};

		double position[3];
		unsigned int count;
		unsigned int *influences;
		double *weights;
	};

	class ariInfluence
	{
	public:
		ariInfluence() {};
		~ariInfluence() {
			//delete name;
		};

		const char* name;
		double bindMat[4][4];
	};

	class ariSkinCluster
	{
	public:
		ariSkinCluster() {};
		ariSkinCluster(MObject &pSkinCluster, MDagPath& pGeo);

		~ariSkinCluster() 
		{ 
			delete[] influences;
			delete[] vertices;
			//delete name;
		};

		const char* name;
		unsigned int maxInfluences;
		double geoMatrix[4][4];

		unsigned int influencesCount;
		ariInfluence *influences;

		unsigned int vertexCount;
		ariVertex* vertices;

		bool writeToFile(const char* pFilePath) const;
		bool loadFromFile(const char* pFilePath);
	};
}


class ArikaraSkinDataCmd : public MPxCommand
{
public:
	ArikaraSkinDataCmd();
	virtual ~ArikaraSkinDataCmd();

	MStatus doIt(const MArgList&);
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const { return true; }

	static void Init();

	static		void* creator();
	static MSyntax newSyntax();

	static MString lastPathName;
	static MString lastFileName;
	static MString lastPrefix;
	static MString lastSuffix;

	/*---Option Variable---*/
	static MString defaultPath;
	static MString defaultPrefix;
	static MString defaultSuffix;
    static double defaultBias;

	static MString GetBasePath();
private:
	MString m_PathName;
	MString m_FileName;
	MString m_Prefix;
	MString m_Suffix;

	MStatus saveSkinData();
	MStatus loadSkinData();

	void loadCleanSkinData(ariSkinData::ariSkinCluster& ariSkin, MIntArray &influIndex, unsigned int& totalNewInfluences);
	void loadSkinData(ariSkinData::ariSkinCluster& ariSkin, MIntArray &influIndex, unsigned int& totalNewInfluences);
    void ParalelLoadByPos(ariSkinData::ariSkinCluster& ariSkin, MFnMesh& mfnMesh, MDoubleArray& newWeights, MIntArray&, unsigned int influenceCout);
	
    MDagPath m_GeoDagPath;
	MObject m_SkinCluster;

	bool m_SaveFlag;
	bool m_LoadFlag;
	bool m_LoadBindMatrixFlag;
	bool m_CleanFlag;
	bool m_LoadByPosFlag;
	bool m_WorldSpaceFlag;
    
    double m_bias;

	bool m_isSkinClusterCreated;

	MDGModifier m_MDGModifier;
	MDagModifier m_MDagModifier;
	MDGModifier m_MDGModMatrix;

	MDoubleArray m_oldWeights;
	MIntArray m_InfluenceArr;

	MObject m_components;

};