import sys

def ReplaceLibs(filePath):
    f = open(filePath, mode='r')
    data = f.read()
    f.close()

    data = data.replace("Qt5Widgetsd.lib", "Qt5Widgets.lib")
    data = data.replace('Qt5Guid.lib', 'Qt5Gui.lib')
    data = data.replace('Qt5Cored.lib', 'Qt5Core.lib')

    f = open(filePath, mode='w')
    f.write(data)
    f.close()
    

if __name__ == "__main__":
    filePath = sys.argv[1]
    ReplaceLibs(filePath)