#include "ArikaraOptions.h"

#include <maya/MCommonSystemUtils.h>
#include <maya/MGlobal.h>

#include <fstream>

ArikaraOption* ArikaraOption::theOne = nullptr;

ArikaraOption::ArikaraOption()
{

}

MString ArikaraOption::GetArikaraFolder()
{
	MString folder = MCommonSystemUtils::getEnv("MAYA_APP_DIR");
	folder += "/ArikaraSkin";
	MStatus status = MCommonSystemUtils::makeDirectory(folder);

#ifdef WIN32
	std::string tmpFolder(folder.asChar());
	std::replace(tmpFolder.begin(), tmpFolder.end(), '/', '\\');
	folder = tmpFolder.c_str();
#endif // WIN32

	return folder;
}

MString ArikaraOption::GetArikaraOptionFile()
{
	MString folder = GetArikaraFolder();

#ifdef WIN32 
	folder += "\\";
#else
	folder += "/";
#endif

	folder += "ArikaraSkinSettings.json";
	return folder;
}

void ArikaraOption::writeDataToFile()
{
	saveData(this);

	MString path = GetArikaraOptionFile();
	std::ofstream f;
	f.open(path.asChar());
	if (f.good())
	{
		f << std::setw(4) << data << std::endl;
		f.close();
	}
}

void ArikaraOption::loadDataFromFile()
{
	MString path = GetArikaraOptionFile();
	std::ifstream f;
	f.open(path.asChar());
	if (f.good())
	{
		f >> data;
		f.close();
	}

	loadData(this);
}

void ArikaraOption::cleanup()
{
	if (theOne)
	{
		MEventMessage::removeCallback(theOne->closeEventID);

		delete theOne;
		theOne = nullptr;
	}
}

void ArikaraOption::SetPluginPath(MString& pPluginFolder)
{
    MStringArray splittedPath;
    pPluginFolder.split('/', splittedPath);

#ifdef WIN32
    std::string tmpPath = pPluginFolder.asChar();
    std::replace(tmpPath.begin(), tmpPath.end(), '/', '\\');
    ArikaraOption::TheOne()->dllFolder = tmpPath.c_str();
    ArikaraOption::TheOne()->dllFolder += "\\";

    for (unsigned int x = 0; x < splittedPath.length()-1; x++)
    {
        ArikaraOption::theOne->PluginFolder += splittedPath[x];
        ArikaraOption::theOne->PluginFolder += "\\";
    }
    ArikaraOption::theOne->IconFolder = ArikaraOption::theOne->PluginFolder;
    ArikaraOption::theOne->IconFolder += "icons\\";

#else
    //@TODO: Implement linux and mac version.
    ArikaraOption::TheOne()->dllFolder = pPluginFolder;
    ArikaraOption::TheOne()->dllFolder += "/";

    for (unsigned int x = 0; x < splittedPath.length() - 1; x++)
    {
        ArikaraOption::theOne->PluginFolder += splittedPath[x];
        ArikaraOption::theOne->PluginFolder += "/";
    }
    ArikaraOption::theOne->IconFolder = ArikaraOption::theOne->PluginFolder;
    ArikaraOption::theOne->IconFolder += "icons/";

#endif // WIN32

}

ArikaraOption::~ArikaraOption()
{

}

ArikaraOption* ArikaraOption::TheOne()
{
	if (theOne == nullptr)
	{
		theOne = new ArikaraOption;

#ifndef NLOADOPTION
        theOne->closeEventID = MEventMessage::addEventCallback("quitApplication", ArikaraOptionOnMayaClose);
#endif // !NLOADOPTION
	}
	return theOne;
}

void ArikaraOptionOnMayaClose(void* pData)
{
	ArikaraOption* option = ArikaraOption::TheOne();
	option->writeDataToFile();
}
