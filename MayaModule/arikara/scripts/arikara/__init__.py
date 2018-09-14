import cleanSkinUI
from maya import cmds

def cleanSkin(maxInfluence=4, maxWeight=0.001):
   inst = cleanSkinUI.CleanSkinUI()
   inst.show()


def resetBindPose():
    cmds.undoInfo(openChunk=True, chunkName="Arikara Reset Bind Pose")
    sel = cmds.ls(selection=True)
    for obj in sel:
        try:
            cmds.arikaraInfluence(obj, resetBindPose=True)
        except Exception as e:
            print e
    cmds.undoInfo(closeChunk=True)