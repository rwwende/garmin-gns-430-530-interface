#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QtGui>

#include "ui_mainWindow.h"
#include "gnsx30Proxy.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:

    MainWindow(QWidget* pParent=0, Qt::WFlags flag=0);
    ~MainWindow();

    void    updateGUI();

protected:
    virtual void 	resizeEvent ( QResizeEvent * event );
private slots:
    void 	onRepositionViewWidget();
    void    on_btnStart_clicked ( bool checked = false );
    void    on_btnStop_clicked ( bool checked = false );

    void    onUpdateTimer();
    //COM
    void    on_btnCOMActiveSet_clicked ( bool checked = false );
    void    on_btnCOMStandbySet_clicked ( bool checked = false );
    //NAV
    void    on_btnNAVActiveSet_clicked ( bool checked = false );
    void    on_btnNAVStandbySet_clicked ( bool checked = false );
    //GPS
    void    on_btnSetGPS_clicked ( bool checked = false );
    void    on_btnSimulateGPS_clicked ( bool checked = false );

private:
    GNSx30Proxy m_GNSx30Proxy;
    QTimer      m_updateTimer;

    Ui::MainWindow ui;
};



#endif // _MAINWINDOW_H_
