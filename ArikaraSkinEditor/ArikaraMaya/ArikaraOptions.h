#pragma once
#include <maya/MString.h>
#include <maya/MEventMessage.h>

#include "../json.hpp"
#include "../Delegate/Delegate.h"

/*---
Info singleton:
	1. http://h-deb.clg.qc.ca/Sujets/Divers--cplusplus/CPP--Singletons.html
-*/

using json = nlohmann::json;

class ArikaraOption
{
private:
	ArikaraOption();

	static ArikaraOption* theOne;

	MCallbackId closeEventID;

public:
	~ArikaraOption();
	static ArikaraOption* TheOne();

	/*--Empecher la copie--*/
	ArikaraOption(const ArikaraOption&) = delete;
	ArikaraOption& operator=(const ArikaraOption&) = delete;

	static MString GetArikaraFolder();
	static MString GetArikaraOptionFile();

	void writeDataToFile();
	void loadDataFromFile();

    json data;

	MultiDelegate<void, ArikaraOption*> saveData;
	MultiDelegate<void, ArikaraOption*> loadData;

	static void cleanup();

    MString PluginFolder;
    MString IconFolder;
    MString dllFolder;

    static void SetPluginPath(MString& pPluginFolder);

};

void ArikaraOptionOnMayaClose(void* pData);