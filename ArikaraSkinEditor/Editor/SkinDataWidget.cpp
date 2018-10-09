#include "../ArikaraMaya/command/ArikaraSkinDataCmd.h"
#include "SkinDataWidget.h"

#include "qtwidgets/qhboxlayout"
#include "qtwidgets/qVboxlayout"
#include "qtwidgets/QLabel"
#include "qtwidgets/qpushbutton"
#include <qtcore/qfileinfo>

#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/MStringArray.h>
#include <maya/MSelectionList.h>


SkinDataWidget::SkinDataWidget(QWidget *parent /*= 0*/) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    QLabel *lbl_file = new QLabel("File:");
    le_path = new QLineEdit(ArikaraSkinDataCmd::defaultPath.asChar());
    QPushButton* pb_browse = new QPushButton("...");
    connect(pb_browse, SIGNAL(clicked()), this, SLOT(browseFile()));

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(lbl_file, 0);
    layout1->addWidget(le_path, 1);
    layout1->addWidget(pb_browse, 0);

    cb_loadBindMat = new QCheckBox("Load Bind Matrix");
    cb_clean = new QCheckBox("Clean Skin");
    QVBoxLayout *layoutcb1 = new QVBoxLayout;
    layoutcb1->addWidget(cb_loadBindMat);
    layoutcb1->addWidget(cb_clean);

    cb_pos = new QGroupBox("Matching Position");
    cb_pos->setCheckable(true);
    cb_pos->setChecked(false);

    cb_worldSpace = new QCheckBox("World Space");

    QHBoxLayout *layouttmp = new QHBoxLayout;
    QLabel *lbl_bias = new QLabel("bias: ");
    dsb_bias = new QDoubleSpinBox();
    dsb_bias->setSingleStep(0.005);
    dsb_bias->setDecimals(5);
    dsb_bias->setValue(ArikaraSkinDataCmd::defaultBias);
    layouttmp->addWidget(lbl_bias, 0);
    layouttmp->addWidget(dsb_bias, 0);

    QVBoxLayout *layoutcb2 = new QVBoxLayout;
    layoutcb2->addWidget(cb_worldSpace);
    layoutcb2->addLayout(layouttmp);
    
    cb_pos->setLayout(layoutcb2);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addLayout(layoutcb1);
    layout2->addWidget(cb_pos);

    QPushButton* pb_save = new QPushButton("Save");
    connect(pb_save, SIGNAL(clicked()), this, SLOT(saveSkin()));
    QPushButton* pb_load = new QPushButton("Load");
    connect(pb_load, SIGNAL(clicked()), this, SLOT(loadSkin()));

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addWidget(pb_save);
    layout3->addWidget(pb_load);

    QSpacerItem * spacer = new QSpacerItem(100, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addLayout(layout1);
    mainLayout->addLayout(layout2);
    mainLayout->addLayout(layout3);
    mainLayout->addSpacerItem(spacer);

    setLayout(mainLayout);
}

SkinDataWidget::~SkinDataWidget()
{

}

void SkinDataWidget::browseFile()
{
    //MString cmd = "fileDialog2 -fileFilter \"*.ars\" -dialogStyle 2 -fm 0 -okc \"Ok\" -cap \"Chose a skin data file.\" -dir \"";
    MString cmd = "fileDialog2 -dialogStyle 2 -fm 3 -okc \"Ok\" -cap \"Pick arikara skin data folder.\" -dir \"";

#ifdef WIN32
    MStringArray splittedPath;
    ArikaraSkinDataCmd::defaultPath.split('\\', splittedPath);
    MString newPath;
    for (unsigned int x = 0; x < splittedPath.length(); x++)
    {
        newPath += splittedPath[x];
        if (x < splittedPath.length() - 1)
            newPath += "\\\\";
    }
    cmd += newPath;
#else
    cmd += ArikaraSkinDataCmd::defaultPath.asChar();
#endif // WIN32
    cmd += "\"";

    
    MStringArray result;
    MStatus status = MGlobal::executeCommand(cmd, result);
    if (status == MS::kSuccess)
    {
        if (result.length() > 0)
        {
            std::string tmp = result[0].asChar();
#ifdef WIN32
            std::replace(tmp.begin(), tmp.end(), '/', '\\');
#endif
            le_path->setText(tmp.c_str());
        }
    }
}

void SkinDataWidget::saveSkin()
{
    MSelectionList sel;
    MGlobal::getActiveSelectionList(sel);

    unsigned int selLen = sel.length();
    if (selLen > 0)
    {
        MString cmd = "arikaraSkinData -save ";
        MString path;
        if (isValidPath(path))
        {
#ifdef WIN32
            MStringArray splittedPath;
            path.split('\\', splittedPath);
            path = "";
            for (unsigned int x = 0; x < splittedPath.length(); x++)
            {
                path += splittedPath[x];
                if (x < splittedPath.length() - 1)
                    path += "\\\\";
            }
#endif // WIN32

            cmd += "-path ";
            cmd += path;
            cmd += " ";
        }

        for (unsigned int x = 0; x < selLen; x++)
        {
            MDagPath dag;
            sel.getDagPath(x, dag);

            MString curCmd(cmd);
            curCmd += dag.partialPathName();
            MGlobal::executeCommand(curCmd, true);
        }
    }
    else
    {
        MGlobal::displayError("You must select at least one skinned object");
    }

}

void SkinDataWidget::loadSkin()
{
    MSelectionList sel;
    MGlobal::getActiveSelectionList(sel);

    unsigned int selLen = sel.length();
    if (selLen > 0)
    {
        MString cmd = "arikaraSkinData -load ";

        if (cb_clean->isChecked())
        {
            cmd += "-clean ";
        }

        if (cb_loadBindMat->isChecked())
        {
            cmd += "-loadBindMatrix ";
        }
        
        if (cb_pos->isChecked())
        {
            cmd += "-position ";
            double bias = dsb_bias->value();
            if (bias != ArikaraSkinDataCmd::defaultBias)
            {
                cmd += "-bias ";
                cmd += bias;
                cmd += " ";
            }

            if (cb_worldSpace->isChecked())
            {
                cmd += "-worldSpace ";
            }
        }

        MString path;
        MString commandPath;
        bool useCmdPath = false;
        if (isValidPath(path))
        {
            bool useCmdPath = true;
        }
        else
        {
            path = ArikaraSkinDataCmd::defaultPath;
        }

#ifdef WIN32
        MStringArray splittedPath;
        path.split('\\', splittedPath);
        for (unsigned int x = 0; x < splittedPath.length(); x++)
        {
            commandPath += splittedPath[x];
            if (x < splittedPath.length() - 1)
                commandPath += "\\\\";
        }
#else
        commandPath = path;

#endif // WIN32

        MString bigCommand;
        for (unsigned int x = 0; x < selLen; x++)
        {
            MDagPath dag;
            sel.getDagPath(x, dag);

            MString curCmd(cmd);

            //Check if we can find a file for the selected object.
            MString filePath = path.asChar();
#ifdef WIN32
            filePath += "\\";
#else
            filePath += "/";
#endif // WIN32

            filePath += dag.partialPathName();
            filePath += ".ars";
            
            QFileInfo file(filePath.asChar());
            if (file.exists())
            {
                if (useCmdPath)
                {
                    curCmd += "-path \"";
                    curCmd += commandPath;
                    curCmd += "\" ";
                }
            }
            else
            {
                MString info = "Couldn't find file: ";
                info += filePath;
                info += "\nPlease specify a file.";
                MGlobal::displayInfo(info);

                MString browseCmd = "fileDialog2 -fileFilter \"*.ars\" -dialogStyle 2 -fm 1 -cap \"Chose a skin data file for object: ";
                browseCmd += dag.partialPathName();
                browseCmd += "\" -dir \"";
                browseCmd += commandPath;
                browseCmd += "\"";

                MStringArray result;
                MStatus status = MGlobal::executeCommand(browseCmd, result);
                if (status == MS::kSuccess)
                {
                    if (result.length() > 0)
                    {
                        MStringArray path;
                        result[0].split('/', path);

                        MString file = path[path.length() - 1];

                        MString folder;

                        for (unsigned int x = 0; x < path.length() - 1; x++)
                        {
                            folder += path[x];
                            if (x < path.length() - 2)
                            {
#ifdef WIN32
                                folder += "\\\\";
#else
                                folder += "/";
#endif //WIN32
                            }
                        }

                        curCmd += "-path \"";
                        curCmd += folder;
                        curCmd += "\" -file \"";
                        curCmd += file;
                        curCmd += "\" ";
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            bigCommand += curCmd;
            bigCommand += dag.partialPathName();
            bigCommand += ";\n";
        }
        MGlobal::executeCommand(bigCommand, true, true);
    }
    else
    {
        MGlobal::displayError("You must select at least one skinned object");
    }
}

bool SkinDataWidget::isValidPath(MString& path)
{
    std::string tmp = le_path->text().toStdString();
    if (tmp.length() > 0)
    {
        int comp = tmp.compare(ArikaraSkinDataCmd::defaultPath.asChar());
        if (comp != 0)
        {
            path = tmp.c_str();
            return true;
        }
    }
    return false;
}
