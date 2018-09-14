#pragma once
#include <qtwidgets/qwidget>
#include <qtwidgets/QTabWidget>
#include "../ArikaraMaya/include/ArikaraSkin.h"

#include <maya/MEventMessage.h>


class WeightEditionTab;
class VertexTab;
class ArikaraOption;
class SkinDataWidget;

class CustomEventFilter;

class ArikaraSkinEditor : public QWidget
{
	Q_OBJECT

public:
	ArikaraSkinEditor(QWidget *parent = nullptr);

	ArikaraSkin arikaraSkin;

	virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

	void refreshUI();

	QTabWidget* tabWidget;

    static ArikaraSkinEditor* curWindow;
    void saveOption(ArikaraOption*);

    //static void OnActiveViewChanged(void* pData);

    friend class CustomEventFilter;

private slots:
	void currentTabChangedSlot(int);

private:
    WeightEditionTab* weigthEditionTab;
    VertexTab*	vertexTab;
    SkinDataWidget* skinData;

    //void installEventFilter();
    //void removeEventFilter();

    //MCallbackId m_ActiveViewChangedID;

    //QObject* m_EventFilter;
    //QWidget* m_ActiveView;
};