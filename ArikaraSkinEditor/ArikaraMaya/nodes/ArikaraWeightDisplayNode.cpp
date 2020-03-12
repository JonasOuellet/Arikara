#include "ArikaraWeightDisplayNode.h"

#include <maya/MTypeId.h> 
#include <maya/MItGeometry.h>


const MTypeId ArikaraWeightDisplayNode::id(AWDN_TYPE_ID);


void* ArikaraWeightDisplayNode::creator()
{
	return new ArikaraWeightDisplayNode();
}


MStatus ArikaraWeightDisplayNode::initialize()
{
	return MStatus::kSuccess;
}

MStatus ArikaraWeightDisplayNode::deform(MDataBlock& block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex)
{
	return MStatus::kSuccess;
}