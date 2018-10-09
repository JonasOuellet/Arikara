include(qtconfig)

QT += widgets
TARGET = arikaraSkinEditor
HEADERS += ArikaraMaya/include/ArikaraSkinEditorCmd.h \
		   Editor/VertexTab.h \
           ArikaraMaya/include/MayaSkinFunction.h \
		   ArikaraMaya/include/ArikaraSkin.h \
		   Editor/arikaraSkinEditor.h \
		   Editor/WeightEditionTab.h \
		   Editor/MirrorWeightWidget.h \
		   ArikaraMaya/command/ArikaraMirrorWeightCmd.h \
		   ArikaraMaya/command/ArikaraSkinEditorSetWeightCmd.h \
		   ArikaraMaya/command/ArikaraInfluenceCmd.h \
		   ArikaraMaya/command/ArikaraSkinDataCmd.h \
		   ArikaraMaya/command/ArikaraTransferWeightCmd.h \
		   Delegate/Delegate.h \
		   json.hpp \
		   ArikaraMaya/ArikaraOptions.h \
		   GlobalDefine.h \
           Editor/SkinDataWidget.h \
           ArikaraMaya/command/ArikaraAttachSkinCmd.h \
           ArikaraMaya/include/ArikaraMath.h \
           skinMode/InteractiveEdit.h \
           skinMode/skinModeBase.h \
           skinMode/VertexInfluenceData.h
SOURCES += pluginMain.cpp \
           ArikaraMaya/source/ArikaraSkinEditorCmd.cpp \
		   Editor/VertexTab.cpp \
           ArikaraMaya/source/ArikaraSkin.cpp \
		   ArikaraMaya/source/MayaSkinFunction.cpp \
		   Editor/arikaraSkinEditor.cpp \
		   Editor/WeightEditionTab.cpp \
		   Editor/MirrorWeightWidget.cpp \
		   ArikaraMaya/command/ArikaraSkinEditorSetWeightCmd.cpp \
		   ArikaraMaya/command/ArikaraInfluenceCmd.cpp \
		   ArikaraMaya/command/ArikaraMirrorWeightCmd.cpp \
		   ArikaraMaya/command/ArikaraTransferWeightCmd.cpp \
		   ArikaraMaya/command/ArikaraSkinDataCmd.cpp \
		   ArikaraMaya/ArikaraOptions.cpp \
           GlobalDefine.cpp \
           Editor/SkinDataWidget.cpp \
           ArikaraMaya/command/ArikaraAttachSkinCmd.cpp \
           ArikaraMaya/source/ArikaraMath.cpp \
           skinMode/InteractiveEdit.cpp \
           skinMode/skinModeBase.cpp \
           skinMode/VertexInfluenceData.cpp