#pragma once
#include <qtwidgets/qwidget>
#include <qtwidgets/qlineedit>
#include <qtwidgets/qpushbutton>
#include <qtwidgets/qgroupbox>
#include <qtwidgets/qradiobutton>
#include <vector>
#include <maya/MGlobal.h>
#include <qtwidgets/qdoublespinbox>
#include <qtwidgets/qspinbox>
#include <qtwidgets/qslider>
#include <qtwidgets/qlistwidget>
#include <qtwidgets/qcheckbox>
#include "GlobalDefine.h"
#include "../ArikaraMaya/include/ArikaraSkin.h"


class ArikaraOption;
class WeightEditionTab;

class SkinModeGroupBox : 
	public QGroupBox
{
	Q_OBJECT

public:
	SkinModeGroupBox(ArikaraSkin*, WeightEditionTab*);

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    void modeChanged();

private:
    QObject* m_EventHandler;
    std::vector<QRadioButton*> modes;
    ArikaraSkin* m_arikaraSkin;
    WeightEditionTab* m_WeightTab;
};

class InfluencesGroupBox :
	public QGroupBox
{
	Q_OBJECT

public:
	InfluencesGroupBox(ArikaraSkin *ariSkin, WeightEditionTab*);
	~InfluencesGroupBox();

	void updateUI();
    void UpdateInfluencesWeightUI();

    double getRemUnusedMinVal() {
        return dsb_RemoveWeightVal->value();
    }

private slots:
	void addSelectedInfluencesSlot();
	void remInfluencesSlot();
	void remUnusedInfluencesSlot();
	void lockSelectedSlots();
	void unlockSelectedSlots();
	void filterToggle(int);
	void setMaxInfluenceSlot();

	void resetBindPoseSlot();

	//List widget slots
	void onItemClicked(QListWidgetItem* item);

	void onSearchEdited(const QString &filterString) 
    {
        std::string tmp(filterString.toStdString());
        mFilterString = tmp.c_str();
        updateUI(); 
    };

private:
	QLineEdit *le_Search;
	QListWidget *lw_influences;
	QDoubleSpinBox * dsb_RemoveWeightVal;
	QSpinBox *sb_MaxInfluences;
	QCheckBox *cb_filter;
	ArikaraSkin *arikaraSkin;
	void getSelectedInfluenceIndex(std::vector<unsigned int>&);

	//std::vector<unsigned int> listInfluencesIndex;
    MIntArray listInfluencesIndex;

	MString mFilterString;

	bool filterString(const MString& pName, const MStringArray &filter);

    WeightEditionTab* m_WeightTab;
};

class WeightEditionTab :
	public QWidget
{
	Q_OBJECT

public:
	WeightEditionTab(ArikaraSkin *ariSkin, QWidget *parent = 0);

	~WeightEditionTab();

	QLineEdit *le_CurrentObject;

	SkinModeGroupBox *skinMode;
	InfluencesGroupBox *influencesBox;

	void updateUI();

    void saveOption(ArikaraOption*);

    void updateWeightSlider();

private slots:
	void setCurrentObjectSlot();

	void weightButton0Slot() { setWeight(0.0); };
	void weightButton1Slot() { setWeight(0.25); };
	void weightButton2Slot() { setWeight(0.5); };
	void weightButton3Slot() { setWeight(0.75); };
	void weightButton4Slot() { setWeight(1.0); };
	void setWeightSlot();

	void addWeightSlot();
	void remWeightSlot();

	void weightSliderPressed();
	void weightSliderReleased();
	void weightSliderMoved(int);

private:
	void setWeight(double value);

	QDoubleSpinBox* dsb_WeightValue;
	ArikaraSkin *arikaraSkin;
    QSlider* s_WeightSlider;

	//void influencesChanged(int);
};