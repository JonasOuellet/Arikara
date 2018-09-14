from maya import OpenMayaUI
from PySide2 import QtWidgets
from shiboken2 import wrapInstance
from maya import cmds


def getTopLevelWidget():
    OpenMayaUI.MQtUtil.mainWindow()    
    ptr = OpenMayaUI.MQtUtil.mainWindow()    
    widget = wrapInstance(long(ptr), QtWidgets.QWidget)
    return widget


def getIconFolder():
    modulePath = cmds.moduleInfo(mn="arikara", path=True)
    iconPath = modulePath + "/icons/"
    return iconPath