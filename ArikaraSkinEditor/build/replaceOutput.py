import sys
import os


def ReplaceOutput(pluginDir, filePath, version=2017):
    f = open(filePath, mode='r')
    data = f.read()
    f.close()
    
    destSTR = "DESTDIR_TARGET = "
    index = data.find(destSTR)
    if index != -1:
        start = index + len(destSTR)

        # find the next line break
        end = data.find("\n", start)

        location = os.path.join(pluginDir, str(version), "arikaraSkinEditor.mll")

        newData = data[:start]
        newData += location
        newData += data[end:]

        f = open(filePath, mode='w')
        f.write(newData)
        f.close()
    

if __name__ == "__main__":
    filePath = sys.argv[1]
    version = sys.argv[2]

    pluginDir = os.path.join(os.path.dirname(sys.argv[0]), os.pardir, os.pardir, 'MayaModule', 'arikara', 'plug-ins')
    pluginDir = os.path.abspath(pluginDir)

    ReplaceOutput(pluginDir, filePath, version)
