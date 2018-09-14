#pragma once

static unsigned int ESkinModeCount = 3;
enum ESkinMode {
	Absolute,
	Relative,
	Scale
};

static const char * ESkinModeString[] = { "Absolute", "Relative", "Scale" };

ESkinMode getSkinModeFromStr(const char* pName);

static unsigned int EAxisCount = 3;
enum EAxis {
	X,
	Y,
	Z,
};
static const char * EAxisString[] = { "X", "Y", "Z" };

enum EDirection {
	PosToNeg,
	NegToPos,
};

static const char * EDirection[] = { "+ -> -", "- -> +" };

#define ReturnOnError(status)			   \
		if (MS::kSuccess != status) {	   \
			return status;				  \
		}

#define ReturnOnErrorVoid(status)			   \
		if (MS::kSuccess != status) {	   \
			return;				  \
		}

#define ReturnOnErrorBool(status)			   \
		if (MS::kSuccess != status) {	   \
			return false;				  \
		}