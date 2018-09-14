#include "../ArikaraMaya/ArikaraOptions.h"
#include "arikaraSkinEditor.h"
#include <qtwidgets/qlayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/qlineedit>
#include <qtcore/qevent>
#include <qtgui/qmouseevent>
#include <qtgui/qkeyevent>
#include <qtwidgets/qapplication>

#include <maya/MQtUtil.h>
#include <maya/M3dView.h>

#include "WeightEditionTab.h"
#include "VertexTab.h"

#include "SkinDataWidget.h"
#include "../GlobalDefine.h"


ArikaraSkinEditor* ArikaraSkinEditor::curWindow = nullptr;

class CustomEventFilter : public QObject 
{
public:
    CustomEventFilter(ArikaraSkinEditor* pArikara, QWidget* parent=nullptr):
        m_ArikaraSkinEditor(pArikara),
        m_IsEditing(false),
        QObject(parent)
    {};
    ~CustomEventFilter() {};

    virtual bool eventFilter(QObject *watched, QEvent *event) override
    {   
        if (m_IsEditing)
        {
            if (event->type() == QEvent::MouseButtonPress)
            {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::MouseButton::LeftButton)
                {
                    MGlobal::displayInfo("End Editing");
                    m_IsEditing = false;
                    QWidget* curView = M3dView::active3dView().widget();
                    curView->removeEventFilter(this);
                    return true;
                }
            }
            MGlobal::displayInfo("Editing");
            return true;
        }

        if (event->type() == QEvent::KeyPress)
        {
            if (QApplication::keyboardModifiers() == Qt::ControlModifier)
            {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_E)
                {
                    QWidget* curView = M3dView::active3dView().widget();
                    QRect curViewRect = curView->geometry();
                    QPoint relCursorPos = curView->mapFromGlobal(QCursor::pos());
                    /* Test if the cursor is in the current view.
                    */
                    if (curViewRect.contains(relCursorPos))
                    {
                        m_ActiveView = curView;
                        m_basePos = relCursorPos;
                        MGlobal::displayInfo("Editing Init");
                        m_IsEditing = true;
                        curView->installEventFilter(this);
                        return true;
                    }
                }
            }
        }

        return QObject::eventFilter(watched, event);
    }

private:
    bool m_IsEditing;
    ArikaraSkinEditor* m_ArikaraSkinEditor;

    QPoint m_basePos;
    QObject* m_ActiveView;
};


void saveDataEditor(ArikaraOption* pData)
{
    if (ArikaraSkinEditor::curWindow != nullptr)
    {
        ArikaraSkinEditor::curWindow->saveOption(pData);
    }
}

void ArikaraSkinEditor::saveOption(ArikaraOption* pOption)
{
    json data;
    QRect geo = this->geometry();
    json jgeo;
    jgeo["x"] = geo.x();
    jgeo["y"] = geo.y();
    jgeo["w"] = geo.width();
    jgeo["h"] = geo.height();
    data["windowGeo"] = jgeo;

    data["currentTab"] = tabWidget->currentIndex();

    pOption->data["ArikaraEditor"] = data;

    weigthEditionTab->saveOption(pOption);
    vertexTab->saveOption(pOption);
}

ArikaraSkinEditor::ArikaraSkinEditor(QWidget *parent) : QWidget(parent, Qt::Window)
{
	//m_ActiveViewChangedID = 0;
	//@TODO: Need to destroy it correctly after because it is causing a crash when unloading plugin.
	//m_EventFilter = new CustomEventFilter(this);
	//m_ActiveView = nullptr;

	//setAttribute(Qt::WA_DeleteOnClose, true);

	//this->setAttribute(Qt::WA_Hover, true);
	//setMouseTracking(true);
    
    setWindowTitle(tr("Arikara Skin Editor"));

    //Set window icons

    MString iconPath = ArikaraOption::TheOne()->IconFolder;
    iconPath += "arikaraIconSmall.png";

    QIcon* icon = new QIcon(iconPath.asChar());
    setWindowIcon(*icon);

	/*if (parent != nullptr)
	{
		//window flag hep
		//https://stackoverflow.com/questions/9291413/how-to-set-the-multiple-flags-in-qmainwindow
		setParent(parent, Qt::WindowStaysOnTopHint);
	}*/
	weigthEditionTab = new WeightEditionTab(&arikaraSkin);
	vertexTab = new VertexTab(&arikaraSkin);

    skinData = new SkinDataWidget;

    QGridLayout *layout = new QGridLayout;

    //http://doc.qt.io/qt-5/qtwidgets-dialogs-tabdialog-example.html
    tabWidget = new QTabWidget;
    tabWidget->addTab(weigthEditionTab, tr("Weight Edition"));
    tabWidget->addTab(vertexTab, tr("Vertex"));
    tabWidget->addTab(skinData, tr("Skin Data"));

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChangedSlot(int)));

    layout->addWidget(tabWidget);

    setLayout(layout);

    curWindow = this;
    ArikaraOption::TheOne()->saveData.addCallback(&saveDataEditor);

    json data = ArikaraOption::TheOne()->data["ArikaraEditor"];
    if (!data.is_null())
    {
        
        if (data.count("windowGeo"))
        {
            json geo = data["windowGeo"];
            int x = geo["x"].get<int>();
            int y = geo["y"].get<int>();
            int w = geo["w"].get<int>();
            int h = geo["h"].get<int>();
            this->setGeometry(x, y, w, h);
        }

        if (data.count("currentTab"))
        {
            int curTab = data["currentTab"].get<int>();
            tabWidget->setCurrentIndex(curTab);
        }
    }
}

void ArikaraSkinEditor::showEvent(QShowEvent *event)
{
    /*if (m_ActiveViewChangedID == 0)
    {
        MStatus status;
        m_ActiveViewChangedID = MEventMessage::addEventCallback("ActiveViewChanged", &ArikaraSkinEditor::OnActiveViewChanged,
            nullptr, &status);
        if (status != MS::kSuccess)
            m_ActiveViewChangedID = 0;
    }*/

    //installEventFilter();


    if (arikaraSkin.SetObjectFromSelection())
    {
        arikaraSkin.SetVertexFromSelection();
    }
    arikaraSkin.OnToolShow();
    refreshUI();
}

void ArikaraSkinEditor::closeEvent(QCloseEvent *event)
{
    arikaraSkin.OnToolClose();

    /*if (m_ActiveViewChangedID != 0)
    {
        MEventMessage::removeCallback(m_ActiveViewChangedID);
        m_ActiveViewChangedID = 0;
    }*/

    //removeEventFilter();
}

void ArikaraSkinEditor::currentTabChangedSlot(int currentTab)
{
	switch (currentTab)
	{
	case 0:
		weigthEditionTab->updateUI();
		break;
	case 1:
		vertexTab->updateUI();
		break;
	default:
		break;
	}
}

//void ArikaraSkinEditor::installEventFilter()
//{
//    removeEventFilter();
//    MQtUtil::mainWindow()->installEventFilter(m_EventFilter);
//}
//
//void ArikaraSkinEditor::removeEventFilter()
//{
//    MQtUtil::mainWindow()->removeEventFilter(m_EventFilter);
//}
//
//void ArikaraSkinEditor::OnActiveViewChanged(void* pData)
//{
//    MGlobal::displayInfo("ActiveViewChanged");
//    curWindow->installEventFilter();
//}

void ArikaraSkinEditor::refreshUI()
{
    if (!arikaraSkin.isSkinValid())
    {
        arikaraSkin.clear();
        weigthEditionTab->updateUI();
        vertexTab->updateUI();
        return;
    }

	switch (tabWidget->currentIndex())
	{
	case 0:
		weigthEditionTab->updateUI();
		break;
	case 1:
		vertexTab->updateUI();
	default:
		break;
	}
}
