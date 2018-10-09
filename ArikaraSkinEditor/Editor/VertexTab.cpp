#include "../ArikaraMaya/ArikaraOptions.h"
#include "VertexTab.h"
#include "../ArikaraMaya/include/MayaSkinFunction.h"

#include <qtwidgets/QVBoxLayout>
#include <qtwidgets/QHBoxLayout>
#include <qtwidgets/qpushbutton>
#include <qtcore/qsize>

VertexTab::VertexTab(ArikaraSkin *ariSkin, QWidget *parent /*= 0*/) : QWidget(parent)
{
	arikaraSkin = ariSkin;
	transferWeightGrp = new TransferWeightGroupBox(ariSkin);
	mirrorWeight = new MirrorWeightWidget;

	cb_UseSoftSelection = new QCheckBox("Use Soft Selection");
	connect(cb_UseSoftSelection, SIGNAL(stateChanged(int)), this, SLOT(cb_UseSoftSelectionToggled(int)));
	QPushButton* pb_SelVertForInf = new QPushButton("Select Vertex for Influences");
	connect(pb_SelVertForInf, SIGNAL(clicked()), this, SLOT(pb_SelectVertForInfSlot()));

	QGroupBox* gbVertLock = new QGroupBox("Vertex Lock");
	cb_UseLock = new QCheckBox("Use Lock");
	connect(cb_UseLock, SIGNAL(stateChanged(int)), this, SLOT(cb_UseLockToggled(int)));
	QPushButton* pbAddLock = new QPushButton("Lock");
	connect(pbAddLock, SIGNAL(clicked()), this, SLOT(pb_LockSelectedVerts()));
	QPushButton* pbRemLock = new QPushButton("Unlock");
	connect(pbRemLock, SIGNAL(clicked()), this, SLOT(pb_UnlockSelectedVerts()));
	QPushButton* pbClearLock = new QPushButton("Clear");
	connect(pbClearLock, SIGNAL(clicked()), this, SLOT(pb_ClearLockedVertex()));
	lbl_lockedVerts = new QLabel("0 Vertex Locked");
	QPushButton* pb_selLockvert = new QPushButton("Select Locked Vertices");
	connect(pb_selLockvert, SIGNAL(clicked()), this, SLOT(pb_SelectLockedVertex()));


	lbl_selectedVerts = new QLabel("0 Vertex Selected");
	QHBoxLayout* selCountLayout = new QHBoxLayout;
	selCountLayout->addWidget(lbl_selectedVerts);
	selCountLayout->addWidget(cb_UseSoftSelection);

	QVBoxLayout* lockMainLayout = new QVBoxLayout;
	QHBoxLayout* lockLayout1 = new QHBoxLayout;
	lockLayout1->addWidget(lbl_lockedVerts);
	lockLayout1->addWidget(cb_UseLock);
	QHBoxLayout* lockLayout2 = new QHBoxLayout;
	lockLayout2->addWidget(pbAddLock);
	lockLayout2->addWidget(pbRemLock);
	lockLayout2->addWidget(pbClearLock);
	lockMainLayout->addLayout(lockLayout1);
	lockMainLayout->addLayout(lockLayout2);
	lockMainLayout->addWidget(pb_selLockvert);
	gbVertLock->setLayout(lockMainLayout);

    QGroupBox* gb_attachSkin = new QGroupBox("Attach Skin");
    QHBoxLayout* hb_attachSkin = new QHBoxLayout;
    QPushButton* pb_AttachSetTarget = new QPushButton("Set Target");
    pb_AttachSetTarget->setToolTip("Select a geometry or faces for the attach target");
    connect(pb_AttachSetTarget, SIGNAL(clicked()), this, SLOT(pb_SetAttachTargetSlot()));
    QPushButton* pb_Attach = new QPushButton("Attach");
    pb_Attach->setToolTip("Attach selected geometry to specified target or/n attach selected faces to itself or target.");
    connect(pb_Attach, SIGNAL(clicked()), this, SLOT(pb_AttachSlot()));

    cb_attachUseTarget = new QCheckBox("Use Target");
    
    hb_attachSkin->addWidget(pb_AttachSetTarget, 1);
    hb_attachSkin->addWidget(pb_Attach, 1);
    hb_attachSkin->addWidget(cb_attachUseTarget);
    gb_attachSkin->setLayout(hb_attachSkin);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(selCountLayout);
	mainLayout->addWidget(pb_SelVertForInf);
	mainLayout->addWidget(gbVertLock);
	mainLayout->addWidget(transferWeightGrp);
	mainLayout->addWidget(mirrorWeight);
    mainLayout->addWidget(gb_attachSkin);
	QSpacerItem * spacer = new QSpacerItem(100, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
	mainLayout->addSpacerItem(spacer);

	setLayout(mainLayout);

    /*
        Load option data.
    */
    json j = ArikaraOption::TheOne()->data["ArikaraEditor"]["VertexTab"];
    if (!j.is_null())
    {
        if (j.count("useSoftSelection"))
        {
            bool check = j["useSoftSelection"].get<bool>();
            if (cb_UseSoftSelection->isChecked() != check)
                cb_UseSoftSelection->setChecked(check);
        }

        if (j.count("useLock"))
        {
            bool check = j["useLock"].get<bool>();
            if (cb_UseLock->isChecked() != check)
                cb_UseLock->setChecked(check);
        }
    }

}

VertexTab::~VertexTab()
{

}

void VertexTab::updateUI()
{
	lbl_selectedVerts->setText((std::to_string(arikaraSkin->GetVertexSelectedCount()) + " Vertex Selected").c_str());
	lbl_lockedVerts->setText((std::to_string(arikaraSkin->GetLockedVertexCount()) + " Vertex Locked").c_str());
}

void VertexTab::saveOption(ArikaraOption* pOption)
{
    json j;
    j["useSoftSelection"] = cb_UseSoftSelection->isChecked();
    j["useLock"] = cb_UseLock->isChecked();
    pOption->data["ArikaraEditor"]["VertexTab"] = j;
    
    transferWeightGrp->saveOption(pOption);
    mirrorWeight->saveOption(pOption);
}

void VertexTab::cb_UseSoftSelectionToggled(int state)
{
	arikaraSkin->useSoftSelection = static_cast<bool>(state);
    arikaraSkin->SetVertexFromSelection();
	updateUI();
}

void VertexTab::pb_SelectVertForInfSlot()
{
	arikaraSkin->selectVertexAffectedByCurrentInfluences();
	updateUI();
}

void VertexTab::cb_UseLockToggled(int state)
{
	arikaraSkin->useLockVertex = static_cast<bool>(state);
}

void VertexTab::pb_LockSelectedVerts()
{
	arikaraSkin->LockSelectedVerts();
	updateUI();
}

void VertexTab::pb_UnlockSelectedVerts()
{
	MGlobal::displayInfo("Unlock selected vertices");
	arikaraSkin->UnlockSelectedVerts();
	updateUI();
}

void VertexTab::pb_ClearLockedVertex()
{
	arikaraSkin->ClearLockedVertex();
	updateUI();
}

void VertexTab::pb_SelectLockedVertex()
{
	arikaraSkin->selectLockedVert();
}

void VertexTab::pb_SetAttachTargetSlot()
{
    MGlobal::executeCommand("arikaraAttachTarget", true);
}

void VertexTab::pb_AttachSlot()
{
    MString cmd("arikaraAttach");
    if (cb_attachUseTarget->isChecked())
    {
        cmd += " -useTarget";
    }
    MGlobal::executeCommand(cmd, true, true);
}

TransferWeightGroupBox::TransferWeightGroupBox(ArikaraSkin *ariSkin, QWidget *parent /*= 0*/) :
	QGroupBox(tr("Transfer Weight"), parent)
{
	arikaraSkin = ariSkin;
	QVBoxLayout* mainLayout = new QVBoxLayout;

	QHBoxLayout* sourceLayout = new QHBoxLayout;
	QLabel * sourcelbl = new QLabel("Source: ");
	le_sourceInfluence = new QLineEdit();
	QPushButton* pb_SetFromTool = new QPushButton("<-T");
	connect(pb_SetFromTool, SIGNAL(clicked()), this, SLOT(pb_setSourceFromTool()));
	pb_SetFromTool->setToolTip("Set Source from selected influence in tool");
	QPushButton* pb_SetFromView = new QPushButton("<-V");
	connect(pb_SetFromView, SIGNAL(clicked()), this, SLOT(pb_setSourceFromViewSlot()));
	pb_SetFromView->setToolTip("Set Source from selected influence in Viewport");
	sourceLayout->addWidget(sourcelbl, 0);
	sourceLayout->addWidget(le_sourceInfluence, 1);
	sourceLayout->addWidget(pb_SetFromTool, 0);
	sourceLayout->addWidget(pb_SetFromView, 0);

	QHBoxLayout* targetLayout = new QHBoxLayout;
	QLabel * targetlbl = new QLabel("Target: ");
	le_targetInfluence = new QLineEdit();
	QPushButton* pb_SetFromToolT = new QPushButton("<-T");
	connect(pb_SetFromToolT, SIGNAL(clicked()), this, SLOT(pb_setTargetFromTool()));
	pb_SetFromToolT->setToolTip("Set Target from selected influence in tool");
	QPushButton* pb_SetFromViewT = new QPushButton("<-V");
	connect(pb_SetFromViewT, SIGNAL(clicked()), this, SLOT(pb_setTargetFromViewSlot()));
	pb_SetFromViewT->setToolTip("Set Target from selected influence in Viewport");
	targetLayout->addWidget(targetlbl, 0);
	targetLayout->addWidget(le_targetInfluence, 1);
	targetLayout->addWidget(pb_SetFromToolT, 0);
	targetLayout->addWidget(pb_SetFromViewT, 0);

	QHBoxLayout *actionLayout = new QHBoxLayout;
	QPushButton *pb_transfer = new QPushButton("Transfer Weight");
	connect(pb_transfer, SIGNAL(clicked()), this, SLOT(pb_TransfertWeightSlot()));
	dsb_transfertValue = new QDoubleSpinBox;
	dsb_transfertValue->setRange(0.0, 1.0);
	dsb_transfertValue->setValue(1.0);
    dsb_transfertValue->setSingleStep(0.05);
	QSize size;
	size.setWidth(75);
	dsb_transfertValue->setMinimumSize(size);
	actionLayout->addWidget(pb_transfer, 1);
	actionLayout->addWidget(dsb_transfertValue, 0);

	mainLayout->addLayout(sourceLayout);
	mainLayout->addLayout(targetLayout);
	mainLayout->addLayout(actionLayout);
	setLayout(mainLayout);

    /************************************************************************/
    /* Load Data                                                            */
    /************************************************************************/

    json data = ArikaraOption::TheOne()->data["ArikaraEditor"]["TransferWeight"];
    if (!data.is_null())
    {
        if (data.count("source"))
        {
            std::string text = data["source"].get<std::string>();
            le_sourceInfluence->setText(text.c_str());
        }
        if (data.count("target"))
        {
            std::string text = data["target"].get<std::string>();
            le_targetInfluence->setText(text.c_str());
        }

        if (data.count("value"))
        {
            double val = data["value"].get<double>();
            dsb_transfertValue->setValue(val);
        }
    }
}

TransferWeightGroupBox::~TransferWeightGroupBox()
{

}

void TransferWeightGroupBox::saveOption(ArikaraOption* pOtion)
{
    json j;
    j["source"] = le_sourceInfluence->text().toStdString();
    j["target"] = le_targetInfluence->text().toStdString();
    j["value"] = dsb_transfertValue->value();

    pOtion->data["ArikaraEditor"]["TransferWeight"] = j;
}

void TransferWeightGroupBox::pb_TransfertWeightSlot()
{
	//MGlobal::displayInfo("Transfer weight Pressed");
	std::string test = le_sourceInfluence->text().toStdString();
	std::string test2 = le_targetInfluence->text().toStdString();
	MString source(test.c_str());
	MString target(test2.c_str());
	double val = dsb_transfertValue->value();
	MString valStr(std::to_string(val).c_str());
	/*ArikaraSkinUtils::transferWeightForSelectedObject(pSource, pTarget, val);*/
	MString command("arikaraTransferWeight -s " + source + " -t " + target + " -v " + valStr);

	MSelectionList sel;
	MGlobal::getActiveSelectionList(sel);
	if (sel.length() == 0)
	{
		if (arikaraSkin->isSkinValid())
		{
			command += " " + arikaraSkin->transformDagPath.partialPathName();
		}
		else
		{return;}
	}
	else
	{
		MDagPath tmp;
		sel.getDagPath(0, tmp);
		if (!tmp.hasFn(MFn::Type::kMesh))
		{
			if (arikaraSkin->isSkinValid())
			{
				command += " " + arikaraSkin->transformDagPath.partialPathName();
			}
			else
			{
				return;
			}
		}
	}

	MGlobal::executeCommand(command, true, true);
}

void TransferWeightGroupBox::pb_setSourceFromViewSlot()
{
	setInfluencesFromViewport(le_sourceInfluence);
}

void TransferWeightGroupBox::pb_setSourceFromTool()
{
	setInfluencesFromTool(le_sourceInfluence);
}

void TransferWeightGroupBox::pb_setTargetFromViewSlot()
{
	setInfluencesFromViewport(le_targetInfluence);
}

void TransferWeightGroupBox::pb_setTargetFromTool()
{
	setInfluencesFromTool(le_targetInfluence);
}

void TransferWeightGroupBox::setInfluencesFromViewport(QLineEdit* pLine)
{
	pLine->setText(ArikaraSelection::GetSelectedObjectName().c_str());
}

void TransferWeightGroupBox::setInfluencesFromTool(QLineEdit* pLine)
{
	pLine->setText(arikaraSkin->GetCurrentInfluenceName().c_str());
}
