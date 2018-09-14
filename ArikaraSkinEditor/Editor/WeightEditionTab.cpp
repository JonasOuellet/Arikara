#include "../ArikaraMaya/ArikaraOptions.h"
#include "WeightEditionTab.h"
#include <qtwidgets/qlayout>
#include <string>
#include <qtwidgets/qsizepolicy>
#include <qtwidgets/qspaceritem>
#include <qtcore/qevent>
#include <qtgui/qmouseevent>

#include "../ArikaraMaya/include/MayaSkinFunction.h"

#include <maya/MFnSkinCluster.h>

#include "../skinMode/InteractiveEdit.h"

#pragma region WeightEditionTab

WeightEditionTab::WeightEditionTab(ArikaraSkin* ariSkin, QWidget *parent /*= 0*/) : QWidget(parent)
{
	arikaraSkin = ariSkin;

    s_WeightSlider = new QSlider(Qt::Orientation::Horizontal);
    s_WeightSlider->setRange(0, 100);
    connect(s_WeightSlider, SIGNAL(sliderPressed()), this, SLOT(weightSliderPressed()));
    connect(s_WeightSlider, SIGNAL(sliderReleased()), this, SLOT(weightSliderReleased()));
    connect(s_WeightSlider, SIGNAL(sliderMoved(int)), this, SLOT(weightSliderMoved(int)));

	skinMode = new SkinModeGroupBox(arikaraSkin, this);

	influencesBox = new InfluencesGroupBox(arikaraSkin, this);
	le_CurrentObject = new QLineEdit();
	le_CurrentObject->setReadOnly(true);

	QPushButton* pb_SetCurrentObject = new QPushButton("<<<");
	pb_SetCurrentObject->setToolTip("Set Current Object to edit");
	connect(pb_SetCurrentObject, SIGNAL(clicked()), this, SLOT(setCurrentObjectSlot()));

	QPushButton* pbweight0 = new QPushButton("0.00");
	connect(pbweight0, SIGNAL(clicked()), this, SLOT(weightButton0Slot()));
	QPushButton* pbweight1 = new QPushButton("0.25");
	connect(pbweight1, SIGNAL(clicked()), this, SLOT(weightButton1Slot()));
	QPushButton* pbweight2 = new QPushButton("0.50");
	connect(pbweight2, SIGNAL(clicked()), this, SLOT(weightButton2Slot()));
	QPushButton* pbweight3 = new QPushButton("0.75");
	connect(pbweight3, SIGNAL(clicked()), this, SLOT(weightButton3Slot()));
	QPushButton* pbweight4 = new QPushButton("1.00");
	connect(pbweight4, SIGNAL(clicked()), this, SLOT(weightButton4Slot()));

	QHBoxLayout *objectLayout = new QHBoxLayout;
	objectLayout->addWidget(le_CurrentObject, 5);
	objectLayout->addWidget(pb_SetCurrentObject);

	QHBoxLayout *weighbutlayout = new QHBoxLayout;
	weighbutlayout->addWidget(pbweight0);
	weighbutlayout->addWidget(pbweight1);
	weighbutlayout->addWidget(pbweight2);
	weighbutlayout->addWidget(pbweight3);
	weighbutlayout->addWidget(pbweight4);

	dsb_WeightValue = new QDoubleSpinBox();
	dsb_WeightValue->setRange(0.0, 1.0);
	dsb_WeightValue->setSingleStep(0.05);

	QPushButton* pb_SetWeight = new QPushButton("Set Weight");
	connect(pb_SetWeight, SIGNAL(clicked()), this, SLOT(setWeightSlot()));

	QPushButton* pb_AddWeight = new QPushButton(" + ");
	connect(pb_AddWeight, SIGNAL(clicked()), this, SLOT(addWeightSlot()));
	QPushButton* pb_RemWeight = new QPushButton(" - ");
	connect(pb_RemWeight, SIGNAL(clicked()), this, SLOT(remWeightSlot()));

	QHBoxLayout *setWeightLayout = new QHBoxLayout;
	setWeightLayout->addWidget(pb_SetWeight, 1);
	setWeightLayout->addWidget(dsb_WeightValue, 1);
	setWeightLayout->addWidget(pb_AddWeight);
	setWeightLayout->addWidget(pb_RemWeight);


	QHBoxLayout *sliderLayout = new QHBoxLayout;
	sliderLayout->addWidget(s_WeightSlider);

	QGridLayout *layout = new QGridLayout;
	//layout->addWidget(le_CurrentObject, 0, 0, 1, 4);
	//layout->addWidget(pb_SetCurrentObject, 0, 5, 1, 1);

	layout->addLayout(objectLayout, 0, 0);
	layout->addLayout(weighbutlayout, 1 ,0);
	layout->addLayout(setWeightLayout, 2, 0);
	layout->addWidget(skinMode, 3, 0);
	layout->addLayout(sliderLayout, 4, 0);
	layout->addWidget(influencesBox, 5, 0);


	skinMode->setMaximumHeight(skinMode->height());

	setLayout(layout);

   json j = ArikaraOption::TheOne()->data["ArikaraEditor"]["WeightEdition"];
    if (!j.is_null())
    {
        if (j.count("weightValue"))
        {
            double val = j["weightValue"].get<double>();
            dsb_WeightValue->setValue(val);
        }
    }
}

WeightEditionTab::~WeightEditionTab()
{

}

void WeightEditionTab::updateUI()
{
    if (arikaraSkin->transformDagPath.isValid())
    {
        le_CurrentObject->setText(arikaraSkin->transformDagPath.partialPathName().asChar());
    }
    else
    {
        le_CurrentObject->clear();
    }
	updateWeightSlider();
	influencesBox->updateUI();
}

void WeightEditionTab::saveOption(ArikaraOption* pOption)
{
    json j;
    j["weightValue"] = dsb_WeightValue->value();
    j["mode"] = arikaraSkin->iEdit.currentSkinMode->getModeName();
    j["remUnusedMin"] = influencesBox->getRemUnusedMinVal();

    pOption->data["ArikaraEditor"]["WeightEdition"] = j;
}

void WeightEditionTab::setCurrentObjectSlot()
{
	if (arikaraSkin->SetObjectFromSelection())
	{
		arikaraSkin->SetVertexFromSelection();
		updateUI();
	}
	else
	{
		MGlobal::displayWarning("No Object with SkinCluster selected");
	}
}

void WeightEditionTab::setWeightSlot()
{
	setWeight(dsb_WeightValue->value());
	updateUI();
}

void WeightEditionTab::addWeightSlot()
{
	double val = dsb_WeightValue->value();
	arikaraSkin->AddVertexWeight(val);
	updateUI();
}

void WeightEditionTab::remWeightSlot()
{
	double val = -dsb_WeightValue->value();
	arikaraSkin->AddVertexWeight(val);
	updateUI();
}

void WeightEditionTab::weightSliderPressed()
{
	double lSliderBaseValue = static_cast<double>(s_WeightSlider->value()) / 100.0;
    arikaraSkin->iEdit.Init(lSliderBaseValue);
}

void WeightEditionTab::weightSliderReleased()
{
    if (arikaraSkin->iEdit.isValid())
    {
        arikaraSkin->iEdit.Close();
        influencesBox->UpdateInfluencesWeightUI();
        updateWeightSlider();
    }
}

void WeightEditionTab::weightSliderMoved(int value)
{
    if (arikaraSkin->iEdit.isValid())
    {
        double val = (static_cast<double>(value) / 100.0);
        arikaraSkin->iEdit.Edit(val);

        influencesBox->UpdateInfluencesWeightUI();
    }
}

void WeightEditionTab::setWeight(double value)
{
	arikaraSkin->SetVertexWeight(value);
	updateUI();
}

void WeightEditionTab::updateWeightSlider()
{
    if (arikaraSkin->iEdit.currentSkinMode != nullptr)
        arikaraSkin->iEdit.currentSkinMode->updateSlider(s_WeightSlider, arikaraSkin);
}

/*void WeightEditionTab::influencesChanged(int influ)
{
	updateWeightSlider();
}*/
#pragma endregion

#pragma region InfluencesGroupBox

InfluencesGroupBox::InfluencesGroupBox(ArikaraSkin *ariSkin, WeightEditionTab* pWeight) :
    QGroupBox("Influences"),
    m_WeightTab(pWeight),
    arikaraSkin(ariSkin)
{
	QVBoxLayout *layout = new QVBoxLayout;

	QHBoxLayout *searchBoxLayout = new QHBoxLayout;
	le_Search = new QLineEdit();
	connect(le_Search, SIGNAL(textChanged(const QString&)), this, SLOT(onSearchEdited(const QString&)));
	cb_filter = new QCheckBox("Filter");
	connect(cb_filter, SIGNAL(stateChanged(int)), this, SLOT(filterToggle(int)));
	searchBoxLayout->addWidget(le_Search, 4);
	searchBoxLayout->addWidget(cb_filter);

	lw_influences = new QListWidget;
	//lw_influences->setSelectionMode(QAbstractItemView::SelectionMode::ContiguousSelection);
	lw_influences->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	connect(lw_influences, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));

	QHBoxLayout *iblayout = new QHBoxLayout;
	QPushButton *remUnInf = new QPushButton("Remove Unused");
	remUnInf->setToolTip("Remove Unused influences, Can specify a minimum weight");
	connect(remUnInf, SIGNAL(clicked()), this, SLOT(remUnusedInfluencesSlot()));
	dsb_RemoveWeightVal = new QDoubleSpinBox;
	dsb_RemoveWeightVal->setRange(0.0, 1.0);
	dsb_RemoveWeightVal->setSingleStep(0.01);
	QPushButton * pb_addInf = new QPushButton(" + ");
	pb_addInf->setToolTip("Add influences selected from viewport");
	connect(pb_addInf, SIGNAL(clicked()), this, SLOT(addSelectedInfluencesSlot()));
	
	QPushButton * pb_rmInf = new QPushButton(" - ");
	pb_rmInf->setToolTip("Remove influences selected form list box");
	connect(pb_rmInf, SIGNAL(clicked()), this, SLOT(remInfluencesSlot()));

	QSpacerItem * spacerTmp = new QSpacerItem(100, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);

	iblayout->addWidget(remUnInf);
	iblayout->addWidget(dsb_RemoveWeightVal);
	iblayout->addWidget(pb_addInf);
	iblayout->addWidget(pb_rmInf);
	iblayout->insertSpacerItem(2, spacerTmp);
	iblayout->setStretch(2, 2);

	QHBoxLayout *tmpLayout = new QHBoxLayout;
	sb_MaxInfluences = new QSpinBox;
	sb_MaxInfluences->setRange(1, 20);
    sb_MaxInfluences->setValue(4);
	QPushButton *pb_setMaxInf = new QPushButton("Set Max Influences");
	connect(pb_setMaxInf, SIGNAL(clicked()), this, SLOT(setMaxInfluenceSlot()));
	QPushButton *pb_lock = new QPushButton("Lock");
	connect(pb_lock, SIGNAL(clicked()), this, SLOT(lockSelectedSlots()));
	QPushButton *pb_unlock = new QPushButton("Unlock");
	connect(pb_unlock, SIGNAL(clicked()), this, SLOT(unlockSelectedSlots()));

	QSpacerItem * spacer = new QSpacerItem(100, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);

	tmpLayout->addWidget(sb_MaxInfluences);
	tmpLayout->addWidget(pb_setMaxInf);
	tmpLayout->addWidget(pb_lock);
	tmpLayout->addWidget(pb_unlock);
	tmpLayout->insertSpacerItem(2, spacer);
	tmpLayout->setStretch(2, 2);

	QHBoxLayout *tmpLayout2 = new QHBoxLayout;
	QPushButton *pb_BindPose = new QPushButton("Reset Bind Pose");
	connect(pb_BindPose, SIGNAL(clicked()), this, SLOT(resetBindPoseSlot()));
	tmpLayout2->addWidget(pb_BindPose);

	layout->addLayout(searchBoxLayout);
	layout->addWidget(lw_influences, 15);
	layout->addLayout(iblayout);
	layout->addLayout(tmpLayout);
	layout->addLayout(tmpLayout2);
	setLayout(layout);


    json j = ArikaraOption::TheOne()->data["ArikaraEditor"]["WeightEdition"];
    if (!j.is_null())
    {
        if (j.count("remUnusedMin"))
        {
            double val = j["remUnusedMin"].get<double>();
            dsb_RemoveWeightVal->setValue(val);
        }
    }
}

InfluencesGroupBox::~InfluencesGroupBox()
{

}

void InfluencesGroupBox::updateUI()
{
	if (arikaraSkin->isSkinValid())
	{
		bool doStringFilter = false;
        MStringArray filterStringArr;
		if (mFilterString.length() > 0)
		{
			doStringFilter = true;
            mFilterString.split(' ', filterStringArr);
		}

		lw_influences->clear();

		MDagPathArray& infl = arikaraSkin->InfluencesList;
        unsigned int count = arikaraSkin->InfluencesCount;

		std::vector<bool> isInfluenceLocked;
        arikaraSkin->isInfluencesLocked(isInfluenceLocked);

		listInfluencesIndex.clear();

		std::string display = "0.000 | U | ";
		std::vector<double> weights;
		if (arikaraSkin->GetAverageWeightForAllInfluences(weights))
		{
			if (cb_filter->isChecked())
			{
				for (unsigned int x = 0; x < count; x++)
				{
					if (weights[x] > 0.000001)
					{
                        MString name = infl[x].partialPathName();
						if (doStringFilter && !filterString(name, filterStringArr))
							continue;
						std::string weight = std::to_string(weights[x]);
						std::string tmp = display + name.asChar();
						tmp[0] = weight[0];
						tmp[2] = weight[2];
						tmp[3] = weight[3];
						tmp[4] = weight[4];
						if (isInfluenceLocked[x])
							tmp[8] = 'L';

						lw_influences->addItem(tmp.c_str());
						listInfluencesIndex.append(x);
					}
				}
			}
			else
			{
				for (unsigned int x = 0; x < count; x++)
				{
					MString name = infl[x].partialPathName();
					if (doStringFilter && !filterString(name, filterStringArr))
						continue;
					std::string weight = std::to_string(weights[x]);
					std::string tmp = display + name.asChar();
					tmp[0] = weight[0];
					tmp[2] = weight[2];
					tmp[3] = weight[3];
					tmp[4] = weight[4];
					if (isInfluenceLocked[x])
						tmp[8] = 'L';
					lw_influences->addItem(tmp.c_str());
					listInfluencesIndex.append(x);
				}
			}
		}
		else
		{
			// No Vertex selected:
			
			for (unsigned int x = 0; x < count; x++)
			{
                MString name = infl[x].partialPathName();
				if (doStringFilter && !filterString(name, filterStringArr))
					continue;
				std::string tmp = display + name.asChar();
				if (isInfluenceLocked[x])
					tmp[8] = 'L';
				lw_influences->addItem(tmp.c_str());
				listInfluencesIndex.append(x);
			}
		}
		if (arikaraSkin->currentInfluence != -1)
		{
			for (unsigned int x = 0; x < listInfluencesIndex.length(); x++)
			{
				if (arikaraSkin->currentInfluence == listInfluencesIndex[x])
				{
					lw_influences->setCurrentRow(x);
					break;
				}
			}
		}

		if (count > 0)
		{
			std::string name = "Influences ";
			name += std::to_string(count);
			setTitle(name.c_str());
		}
		else
		{
			setTitle("Influences");
		}

		sb_MaxInfluences->setValue(ArikaraSkinUtils::getMaxInfluence(arikaraSkin->skinClusterMObject));
	}
    else
    {
        lw_influences->clear();
        //set project default.
        sb_MaxInfluences->setValue(4);
        listInfluencesIndex.clear();
        setTitle("Influences");
    }
}

void InfluencesGroupBox::UpdateInfluencesWeightUI()
{
    if (listInfluencesIndex.length() > 0)
    {
        std::vector<double> weights;
        arikaraSkin->GetAverageWeightForInfluences(listInfluencesIndex, weights);

        for (unsigned int x = 0; x < listInfluencesIndex.length(); x++)
        {
            std::string weight = std::to_string(weights[x]);
            QListWidgetItem* curItem = lw_influences->item(x);
            QString tmp = curItem->text();
            tmp[0] = weight[0];
            tmp[2] = weight[2];
            tmp[3] = weight[3];
            tmp[4] = weight[4];;
            curItem->setText(tmp);
        }
    }
}

void InfluencesGroupBox::addSelectedInfluencesSlot()
{
	arikaraSkin->addSelectedInfluences();
}

void InfluencesGroupBox::remInfluencesSlot()
{
	
	QList<QListWidgetItem*> selectedItem = lw_influences->selectedItems();
	if (selectedItem.length() > 0)
	{
		std::vector<unsigned int> index(selectedItem.size());
		for (int x = 0; x < selectedItem.size(); x++)
		{
			index[x] = listInfluencesIndex[lw_influences->row(selectedItem[x])];
		}
		arikaraSkin->removeInfluences(index);
	}
}

void InfluencesGroupBox::remUnusedInfluencesSlot()
{
	arikaraSkin->removeUnusedInfluence(dsb_RemoveWeightVal->value());
}

void InfluencesGroupBox::lockSelectedSlots()
{
	std::vector<unsigned int> index;
	getSelectedInfluenceIndex(index);
	ArikaraSkinUtils::lockInfluence(arikaraSkin->skinClusterMObject, index);
	updateUI();
}

void InfluencesGroupBox::unlockSelectedSlots()
{
	std::vector<unsigned int> index;
	getSelectedInfluenceIndex(index);
	ArikaraSkinUtils::lockInfluence(arikaraSkin->skinClusterMObject, index, false);
	updateUI();
}

void InfluencesGroupBox::filterToggle(int state)
{
	updateUI();
}

void InfluencesGroupBox::setMaxInfluenceSlot()
{
    int val = sb_MaxInfluences->value();
    MString command = "arikaraInfluence -maxInfluence ";
    command += val;
	if (arikaraSkin->isSkinValid())
	{
        MDagPath geo = arikaraSkin->geoDagPath;
        geo.pop();
        command += " " + geo.partialPathName();
	}
    MGlobal::executeCommand(command, true, true);
}

void InfluencesGroupBox::resetBindPoseSlot()
{
	//ArikaraSkinUtils::resetBindPose(arikaraSkin->skinClusterMObject);
	MString command = "arikaraInfluence -resetBindPose " + arikaraSkin->transformDagPath.partialPathName();
	MGlobal::executeCommand(command, true, true);
}

void InfluencesGroupBox::onItemClicked(QListWidgetItem* item)
{
	int index = listInfluencesIndex[lw_influences->currentRow()];
	arikaraSkin->currentInfluence = index;

    arikaraSkin->OnInfluenceChanged(index);

    m_WeightTab->updateWeightSlider();
}

void InfluencesGroupBox::getSelectedInfluenceIndex(std::vector<unsigned int>&index )
{
	index.clear();
	for (int x = 0; x < lw_influences->count(); x++)
	{
		if (lw_influences->item(x)->isSelected())
		{
			index.push_back(listInfluencesIndex[x]);
		}
	}
}

bool InfluencesGroupBox::filterString(const MString &Name, const MStringArray &filter)
{
    for (unsigned int x = 0; x < filter.length(); x++)
    {
        if (Name.indexW(filter[x]) < 0)
        {
            return false;
        }
    }
    return true;
}
#pragma endregion

#pragma region SkinModeGroupBox
/**
* Skin Mode Group Box.
*
*/


SkinModeGroupBox::SkinModeGroupBox(ArikaraSkin* p_arikaraSkin, WeightEditionTab* pWeightTab) :
    m_arikaraSkin(p_arikaraSkin),
    m_WeightTab(pWeightTab),
    QGroupBox("Mode")
{
    QGridLayout *layout = new QGridLayout;

    std::vector<const char*> modeNames;
    InteractiveEdit::GetModesNames(modeNames);

    for (int x = 0; x < static_cast<int>(modeNames.size()); x++)
    {
        QRadioButton *tmp = new QRadioButton(modeNames[x]);

        int row = x / 3;
        int col = x % 3;

        layout->addWidget(tmp, row, col);

        if (x == 0)
        {
            tmp->setChecked(true);
            //tmp->setDown(true);
            m_arikaraSkin->iEdit.SetCurrentMode(0);
        }

        tmp->installEventFilter(this);
        modes.push_back(tmp);
    }
    setLayout(layout);

    json j = ArikaraOption::TheOne()->data["ArikaraEditor"]["WeightEdition"];
    if (!j.is_null())
    {
        if (j.count("mode"))
        {
            std::string mode = j["mode"].get<std::string>();
            m_arikaraSkin->iEdit.SetCurrentMode(mode.c_str());
            modes[m_arikaraSkin->iEdit.GetCurrentModeIndex()]->setChecked(true);
        }
    }
}

bool SkinModeGroupBox::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;
        if (mouseEvent->button() == Qt::LeftButton)
        {
            bool found = false;
            int id = 0;
            QRadioButton* selectedMode;
            for (QRadioButton* curMode : modes)
            {
                if (watched == (QObject*)curMode)
                {
                    curMode->setChecked(true);
                    m_arikaraSkin->iEdit.SetCurrentMode(id);
                    modeChanged();
                    return true;
                }
                id++;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void SkinModeGroupBox::modeChanged()
{
    m_WeightTab->updateWeightSlider();
}

#pragma endregion