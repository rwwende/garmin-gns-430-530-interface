#include <windows.h>
#include "mainWindow.h"


MainWindow::MainWindow(QWidget* pParent, Qt::WFlags flags)
    : QWidget(pParent, flags)
{
    
    ui.setupUi(this);

    m_GNSx30Proxy.initialize();

    GNSIntf* pIntf = m_GNSx30Proxy.getInterface();

    //initialize the initial frequencies
    pIntf->com1_active = 12345;
    pIntf->com1_standby = 12345;
    pIntf->nav1_active = 12345;
    pIntf->nav1_standby = 12345;



    
}

MainWindow::~MainWindow()
{
    m_GNSx30Proxy.terminate();
}

void MainWindow::resizeEvent ( QResizeEvent * event )
{

    QWidget::resizeEvent(event);


    QTimer::singleShot(10, this, SLOT(onRepositionViewWidget()));


}


void MainWindow::onRepositionViewWidget()
{

    int view_width = ui.gnsViewWidget->getW();
    int view_height = ui.gnsViewWidget->getH();

    float view_ratio = (float)view_width/(float)view_height;

    int box_width = ui.view_box->width();
    int box_height = ui.view_box->height();

    float box_ratio = (float)(box_width)/(float)box_height;


    int x,y;
    int w, h;
    if(box_ratio >= view_ratio)
    {
        //this is a vertical fit, center the camera widget horizontaly
        h = box_height;
        w = view_ratio*h;

        x = (box_width-w)/2;
        y = 0;

    }else
    {
        //this a horizontal fit, center the camera widget vertically
        w = box_width;
        h = (float)w/view_ratio;

        x = 0;
        y = (box_height-h)/2;
    }


    ui.gnsViewWidget->setGeometry(x, y, w, h);


}

void MainWindow::on_btnStart_clicked ( bool checked  )
{
    Q_UNUSED(checked);

    if(0 == ui.cmbModel->currentIndex())
    {
        m_GNSx30Proxy.open(TYPE_GNS430);
    }else
    {
        m_GNSx30Proxy.open(TYPE_GNS530);
    }

    ui.gnsViewWidget->setGNSx30Proxy(&m_GNSx30Proxy);
    QTimer::singleShot(10, this, SLOT(onRepositionViewWidget()));
}

void MainWindow::on_btnStop_clicked ( bool checked  )
{
    Q_UNUSED(checked);
    m_GNSx30Proxy.close();

    ui.gnsViewWidget->setGNSx30Proxy(NULL);
    QTimer::singleShot(10, this, SLOT(onRepositionViewWidget()));

}
