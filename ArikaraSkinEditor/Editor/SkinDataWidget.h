#pragma once
#include <qtwidgets/qwidget>
#include <qtwidgets/qgroupbox>
#include <qtwidgets/qlineedit>
#include <qtwidgets/qcheckbox>
#include <qtwidgets/qdoublespinbox>

//class ArikaraOption;

class MString;

class SkinDataWidget :
    public QWidget
{
    Q_OBJECT

public:
    SkinDataWidget(QWidget *parent = 0);
    ~SkinDataWidget();

    //void saveOption(ArikaraOption*);

private slots:
    void browseFile();
    void saveSkin();
    void loadSkin();

private:
    QLineEdit *le_path;
    QLineEdit *le_prefix;
    QLineEdit *le_suffix;

    QCheckBox *cb_loadBindMat;
    QCheckBox *cb_clean;

    QGroupBox *cb_pos;
    QCheckBox *cb_worldSpace;
    QDoubleSpinBox *dsb_bias;

    bool isValidPath(MString& path);

};