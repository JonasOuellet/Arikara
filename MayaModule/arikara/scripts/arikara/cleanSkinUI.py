from PySide2.QtWidgets import QWidget, QFormLayout, QSpinBox, QDoubleSpinBox, QPushButton
from PySide2.QtGui import QIcon
from PySide2.QtCore import Qt

import uiUtils

from maya import cmds

reload(uiUtils)

class CleanSkinUI(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.setParent(uiUtils.getTopLevelWidget(), 1)

        self.setAttribute(Qt.WA_DeleteOnClose)
        self.setWindowTitle("Clean Skin")
        self.setWindowIcon(QIcon(uiUtils.getIconFolder()+"arikara_CleanSkin.png"))

        lLayout = QFormLayout()

        # Max Weight
        self.dsb_maxWeight = QDoubleSpinBox()
        self.dsb_maxWeight.setRange(0.0, 1.0)
        self.dsb_maxWeight.setDecimals(4)
        self.dsb_maxWeight.setSingleStep(0.001)
        weight = 0.001
        if cmds.optionVar(exists="ariCleanSkin_MW"):
            weight = cmds.optionVar(q="ariCleanSkin_MW")
        else:
            cmds.optionVar(fv=("ariCleanSkin_MW", weight))
        self.dsb_maxWeight.setValue(weight)
        self.dsb_maxWeight.editingFinished.connect(self.dsb_maxWeightEdited)

        # Max Influence
        self.sb_maxInfluence = QSpinBox()
        self.sb_maxInfluence.setRange(1, 12)
        infl = 4
        if cmds.optionVar(exists="ariCleanSkin_MI"):
            infl = cmds.optionVar(q="ariCleanSkin_MI")
        else:
            cmds.optionVar(iv=("ariCleanSkin_MI", infl))
        self.sb_maxInfluence.setValue(infl)
        self.sb_maxInfluence.editingFinished.connect(self.sb_maxInfluenceEdited)

        self.pb_cleanSkin = QPushButton("Clean Skin")
        self.pb_cleanSkin.clicked.connect(self.cleanSkinCallback)

        lLayout.addRow("Max Weight:", self.dsb_maxWeight)
        lLayout.addRow("Max Influence:", self.sb_maxInfluence)
        lLayout.addRow(self.pb_cleanSkin)

        self.setLayout(lLayout)

        if cmds.optionVar(exists="ariCleanSkin_Geo"):
            geo = cmds.optionVar(q="ariCleanSkin_Geo")
            self.setGeometry(*geo)

    def cleanSkinCallback(self):
        cmds.undoInfo(openChunk=True, chunkName="Arikara Clean Skin")
        selection = cmds.ls(selection=True)
        maxWeight = self.dsb_maxWeight.value()
        maxInfluence = self.sb_maxInfluence.value()
        for obj in selection:
            try:
                cmds.arikaraInfluence(obj, maxInfluence=maxInfluence)
            except Exception as e:
                print e
            try:
                cmds.arikaraInfluence(obj, ru=maxWeight)
            except Exception as e:
                print e
        cmds.undoInfo(closeChunk=True)

    def updateGeometry(self):
        geo = self.geometry()
        cmds.optionVar(iv=("ariCleanSkin_Geo", geo.x()))
        cmds.optionVar(iva=("ariCleanSkin_Geo", geo.y()))
        cmds.optionVar(iva=("ariCleanSkin_Geo", geo.width()))
        cmds.optionVar(iva=("ariCleanSkin_Geo", geo.height()))

    def moveEvent(self, event):
        self.updateGeometry()

    def resizeEvent(self, event):
        self.updateGeometry()
    
    def dsb_maxWeightEdited(self):
        val = self.dsb_maxWeight.value()
        cmds.optionVar(fv=("ariCleanSkin_MW", val))

    def sb_maxInfluenceEdited(self):
        val = self.sb_maxInfluence.value()
        cmds.optionVar(iv=("ariCleanSkin_MI", val))