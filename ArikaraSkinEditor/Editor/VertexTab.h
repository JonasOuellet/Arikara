#pragma once
#include "../ArikaraMaya/include/ArikaraSkin.h"
#include "MirrorWeightWidget.h"

//#include <qtwidgets/qwidget>
#include <qtwidgets/qgroupbox>
#include <qtwidgets/qcheckbox>
#include <qtwidgets/qlabel>
#include <qtwidgets/qlineedit>
#include <qtwidgets/qdoublespinbox>


class ArikaraOption;

class TransferWeightGroupBox :
	public QGroupBox
{
	Q_OBJECT

public:
	TransferWeightGroupBox(ArikaraSkin *ariSkin, QWidget *parent = 0);
	~TransferWeightGroupBox();

    void saveOption(ArikaraOption*);

private slots:
	void pb_TransfertWeightSlot();

	void pb_setSourceFromViewSlot();
	void pb_setSourceFromTool();

	void pb_setTargetFromViewSlot();
	void pb_setTargetFromTool();
	
private:
	ArikaraSkin* arikaraSkin;
	QLineEdit* le_sourceInfluence;
	QLineEdit* le_targetInfluence;
	QDoubleSpinBox* dsb_transfertValue;

	void setInfluencesFromViewport(QLineEdit* pLine);
	void setInfluencesFromTool(QLineEdit* pLine);
};

class VertexTab :
	public QWidget
{
	Q_OBJECT

public:
	VertexTab(ArikaraSkin *ariSkin, QWidget *parent = 0);

	~VertexTab();

	QCheckBox* cb_UseSoftSelection;
	QCheckBox* cb_UseLock;

	TransferWeightGroupBox* transferWeightGrp;
	MirrorWeightWidget* mirrorWeight;

	void updateUI();

    void saveOption(ArikaraOption*);

private slots:
	void cb_UseSoftSelectionToggled(int state);
	void pb_SelectVertForInfSlot();

	void cb_UseLockToggled(int state);
	void pb_LockSelectedVerts();
	void pb_UnlockSelectedVerts();
	void pb_ClearLockedVertex();
	void pb_SelectLockedVertex();

    void pb_SetAttachTargetSlot();
    void pb_AttachSlot();

private:
	ArikaraSkin* arikaraSkin;

	QLabel* lbl_lockedVerts;
	QLabel* lbl_selectedVerts;

    QCheckBox* cb_attachUseTarget;

};