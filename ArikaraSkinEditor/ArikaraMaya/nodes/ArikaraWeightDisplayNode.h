#pragma once
#include <maya/MPxDeformerNode.h>


#define AWDN_TYPE_ID 0x9552


class ArikaraWeightDisplayNode : public MPxDeformerNode
{
public:
	static  void*   creator();
	static  MStatus initialize();

	// Deformation function
	//
	virtual MStatus deform(MDataBlock& block,
		MItGeometry& iter, 
		const MMatrix& mat,
		unsigned int multiIndex);

	static const MTypeId id;
};
