#pragma once

#include <qtwidgets/qgroupbox>
#include <qtwidgets/qlineedit>
#include <qtwidgets/qcombobox>
#include <qtwidgets/qdoublespinbox>

class ArikaraOption;

class MirrorWeightWidget :
	public QGroupBox
{
	Q_OBJECT

public:
	MirrorWeightWidget(QWidget *parent = 0);
	~MirrorWeightWidget();

    void saveOption(ArikaraOption*);

private slots:
	void mirrorWeightSlot();

private:
	QLineEdit *le_Find;
	QLineEdit *le_Replace;
	QComboBox *cb_MirrorAxis;
	QDoubleSpinBox *dsb_bias;
};