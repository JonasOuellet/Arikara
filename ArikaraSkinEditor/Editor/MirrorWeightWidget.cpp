#include "../ArikaraMaya/ArikaraOptions.h"
#include "MirrorWeightWidget.h"
#include "../GlobalDefine.h"

#include "qtwidgets/qhboxlayout"
#include "qtwidgets/qVboxlayout"
#include "qtwidgets/QLabel"
#include "qtwidgets/qpushbutton"

#include "Maya/MGlobal.h"


#include <sstream>

MirrorWeightWidget::MirrorWeightWidget(QWidget *parent /*= 0*/) : QGroupBox("Mirror Weight", parent)
{
	QVBoxLayout *mainLayout = new QVBoxLayout;

	QLabel *lbl_find = new QLabel("Find: ");
	le_Find = new QLineEdit("Left");
	QLabel *lbl_replace = new QLabel("Replace: ");
	le_Replace = new QLineEdit("Right");

	QHBoxLayout *layout1 = new QHBoxLayout;
	layout1->addWidget(lbl_find, 0);
	layout1->addWidget(le_Find, 1);
	layout1->addWidget(lbl_replace, 0);
	layout1->addWidget(le_Replace, 1);

	QLabel *lbl_MirrorAxis = new QLabel("Mirror Axis: ");
	cb_MirrorAxis = new QComboBox();
	for (unsigned int x = 0; x < EAxisCount; x++)
		cb_MirrorAxis->addItem(EAxisString[x]);
	QLabel *lbl_Bias = new QLabel("Bias: ");
	dsb_bias = new QDoubleSpinBox();
	dsb_bias->setValue(0.05);
	dsb_bias->setSingleStep(0.05);
	QPushButton *pb_MirrorWeight = new QPushButton("Mirror Weights");
	connect(pb_MirrorWeight, SIGNAL(clicked()), this, SLOT(mirrorWeightSlot()));

	QHBoxLayout *layout2 = new QHBoxLayout;
	layout2->addWidget(lbl_MirrorAxis, 0);
	layout2->addWidget(cb_MirrorAxis, 0);
	layout2->addWidget(lbl_Bias, 0);
	layout2->addWidget(dsb_bias, 0);
	layout2->addWidget(pb_MirrorWeight, 1);

	mainLayout->addLayout(layout1);
	mainLayout->addLayout(layout2);
	setLayout(mainLayout);

    /************************************************************************/
    /* Load Data                                                            */
    /************************************************************************/

    json data = ArikaraOption::TheOne()->data["ArikaraEditor"]["MirrorWeight"];
    if (!data.is_null())
    {
        if (data.count("find"))
        {
            std::string text = data["find"].get<std::string>();
            le_Find->setText(text.c_str());
        }

        if (data.count("replace"))
        {
            std::string text = data["replace"].get<std::string>();
            le_Replace->setText(text.c_str());
        }

        if (data.count("bias"))
        {
            double val = data["bias"].get<double>();
            dsb_bias->setValue(val);
        }

        if (data.count("axis"))
        {
            int axis = data["axis"].get<int>();
            cb_MirrorAxis->setCurrentIndex(axis);
        }
    }
}

MirrorWeightWidget::~MirrorWeightWidget()
{

}

void MirrorWeightWidget::saveOption(ArikaraOption* pOption)
{
    json j;
    j["find"] = le_Find->text().toStdString();
    j["replace"] = le_Replace->text().toStdString();
    j["bias"] = dsb_bias->value();
    j["axis"] = cb_MirrorAxis->currentIndex();

    pOption->data["ArikaraEditor"]["MirrorWeight"] = j;
}

void MirrorWeightWidget::mirrorWeightSlot()
{
	std::string find = le_Find->text().toStdString();
	std::string replace = le_Replace->text().toStdString();

	int axis = cb_MirrorAxis->currentIndex();
	double bias = dsb_bias->value();

	std::stringstream ss;
	ss << "arikaraMirrorWeight " << "-f \"" << find << "\" -r \"" << replace << "\" -a " << axis << " -b " << bias << ";";

	MGlobal::executeCommand(ss.str().c_str(), true, true);
}

